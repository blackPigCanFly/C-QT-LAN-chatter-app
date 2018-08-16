#include "chatwindow.h"
#include "htmldelegate.h"
#include "imagemessage.h"
#include "textmessage.h"
#include "ui_chatwindow.h"
#include "privatemessage.h"

#include <QBuffer>
#include <QFileDialog>
#include <QHostInfo>
#include <QMessageBox>
#include <QMenu>
#include <QInputDialog>

ChatWindow::ChatWindow(QWidget *parent)
        : QMainWindow(parent),
          ui(new Ui::ChatWindow),
          client(QSharedPointer<p2pnetworking::Client>::create()),
          _chatStarted(false),
          _isPrivate(false) {
    ui->setupUi(this);

    connect(client.data(), &p2pnetworking::Client::newMessage, this, &ChatWindow::handleMessage);
    connect(client.data(), SIGNAL(newParticipant(QString)), this, SLOT(newParticipant(QString)));
    connect(client.data(), SIGNAL(participantLeft(QString)), this, SLOT(participantLeft(QString)));
    ui->listView->setModel(&historyModel);
    // The HTMLDelegate allows display of HTML formatted text, a subset of HTML is supported.
    // See http://doc.qt.io/qt-5/richtext-html-subset.html - also can be found in built-in help.
    auto *delegate = new HTMLDelegate(this);
    ui->listView->setItemDelegate(delegate);

    ui->noPeople_privateChatroom->hide();

    // show set profile window
    _setProfileWindow = new SetProfile(nullptr, true, this);
    _setProfileWindow->setWindowModality(Qt::ApplicationModal);
    _setProfileWindow->show();
}

ChatWindow::ChatWindow(ChatWindow *parent, QString chatroomName,
                       QSharedPointer<QList<QListWidgetItem *>> participants,
                       QSharedPointer<ActionMessage> actionMessageIfBeingInvited)
        : ui(new Ui::ChatWindow),
          _parent(parent),
          client(parent->client),
          _chatStarted(true),
          _isPrivate(true) {
    ui->setupUi(this);

    connect(client.data(), &p2pnetworking::Client::participantLeft, this, &ChatWindow::participantLeft);
    connect(this, SIGNAL(windowClosed(QString)), parent, SLOT(privateChatWindowClosed(QString)));

    // setup UI
    ui->listView->setModel(&historyModel);
    auto delegate = new HTMLDelegate(this);
    ui->listView->setItemDelegate(delegate);
    setWindowTitle(chatroomName);
    ui->createRoom->setToolTip("Invite people...");
    setWindowIcon(QIcon(":/resource/private.png"));

    // start invite people in
    inviteParticipants(std::move(participants), std::move(actionMessageIfBeingInvited));
}

ChatWindow::~ChatWindow() {
    delete ui;
}

void ChatWindow::handleMessage(QSharedPointer<Message> message) {
    if (QSharedPointer<TextMessage> txt = qSharedPointerDynamicCast<TextMessage>(message)) {
        // PLAIN TEXT MESSAGE
        appendMessage(txt->sender(), txt->message());
    } else if (QSharedPointer<ImageMessage> img = qSharedPointerDynamicCast<ImageMessage>(message)) {
        // IMAGE MESSAGE
        if (_isPrivate || !img->isPrivate()) {  // IN A PRIVATE WINDOW
            appendMessage(img->sender(), img->imageRaw(), true);
        } else { // IN A PUBLIC WINDOW
            auto privateWindow = _privateChatWindows.find(img->chatroomName());
            if (privateWindow == _privateChatWindows.end()) {
                // NO SUCH PRIVATE WINDOW, create one
                inviteParticipants(QSharedPointer<QList<QListWidgetItem *>>::create(
                        ui->participantsListWidget->findItems(img->sender(), Qt::MatchExactly)), img->chatroomName());
                privateWindow = _privateChatWindows.find(img->chatroomName());
            }
            privateWindow.value()->handleMessage(img);
        }
    } else if (QSharedPointer<FileMessage> file = qSharedPointerDynamicCast<FileMessage>(message)) {
        // FILE MESSAGE
        if (_isPrivate || !file->isPrivate()) {
            appendMessage(file->sender(), file->file(), false);
            _files.insert(chatHistory.length() - 1, file);
        } else {
            auto privateWindow = _privateChatWindows.find(file->chatroomName());
            if (privateWindow == _privateChatWindows.end()) {
                // NO SUCH PRIVATE WINDOW, create one
                inviteParticipants(QSharedPointer<QList<QListWidgetItem *>>::create(
                        ui->participantsListWidget->findItems(file->sender(), Qt::MatchExactly)), file->chatroomName());
                privateWindow = _privateChatWindows.find(file->chatroomName());
            }
            privateWindow.value()->handleMessage(file);
        }
    } else if (QSharedPointer<IdentityMessage> profile = qSharedPointerDynamicCast<IdentityMessage>(message)) {
        // IDENTITY MESSAGE
        _profiles.insert(profile->sender(), profile);
        if (!profile->image().isNull()) {
            auto toUpdate = ui->participantsListWidget->findItems(profile->sender(), Qt::MatchExactly);
                    foreach (QListWidgetItem *item, toUpdate) {
                    item->setIcon(QIcon(QPixmap::fromImage(profile->image())));
                }
        }
    } else if (QSharedPointer<PrivateMessage> pMessage = qSharedPointerDynamicCast<PrivateMessage>(message)) {
        // PRIVATE MESSAGE
        if (_isPrivate) { // IN A PRIVATE WINDOW
            appendMessage(pMessage->sender(), pMessage->message());
        } else { // IN A PUBLIC WINDOW
            auto privateWindow = _privateChatWindows.find(pMessage->receiver());
            if (privateWindow == _privateChatWindows.end()) {
                // NO SUCH PRIVATE WINDOW, create one
                inviteParticipants(QSharedPointer<QList<QListWidgetItem *>>::create(
                        ui->participantsListWidget->findItems(pMessage->sender(), Qt::MatchExactly)),
                                   pMessage->receiver());
                privateWindow = _privateChatWindows.find(pMessage->receiver());
            }
            privateWindow.value()->handleMessage(pMessage);
        }
    } else if (QSharedPointer<ActionMessage> action = qSharedPointerDynamicCast<ActionMessage>(message)) {
        // ACTION MESSAGE
        handleActionMessage(action);
    }
}

void ChatWindow::appendMessage(const QString &from, const QString &message) {
    QString newLine = "<strong>";
    newLine += from;
    newLine += ":</strong><br \\>  ";
    newLine += message;
    chatHistory << newLine;
    cleanupHistory();

    ui->listView->scrollToBottom();
}

void ChatWindow::appendMessage(const QString &from, const QByteArray &raw, bool isImage) {
    QString newLine = "<strong>";
    newLine += from;
    newLine += ":</strong><br \\>  ";
    if (isImage) {
        // image
        newLine += "<img src=\"data:image/png;base64,";
        newLine += raw.toBase64();
        newLine += "\">";
    } else {
        // file
        newLine += "[FILE] Double click to save. ";
    }
    chatHistory << newLine;
    cleanupHistory();

    ui->listView->scrollToBottom();
}

void ChatWindow::updateProfile(IdentityMessage *im) {
    // update the _myProfile variable
    _myProfile = QSharedPointer<IdentityMessage>(
            new IdentityMessage(client->nickName(),
                                _chatStarted ? im->username() : im->sender(),
                                im->location(),
                                im->timezone(),
                                im->image()));

    if (!_chatStarted) { // the chat have not started
        // set the username client
        client->setUserName(im->username());
        // start
        client->start();
        _chatStarted = true;
        setWindowTitle(windowTitle() + " - " + im->sender());
        setWindowIcon(QIcon(":/resource/public.png"));
    }

    delete im;

    client->sendMessage(_myProfile); // send out the updated profile

    ui->lineEdit->setFocus();
}

void ChatWindow::setProfileClosedWithoutProfile() {
    this->close();
}

void ChatWindow::setProfileWindowClosed() {
    delete _setProfileWindow;
}

void ChatWindow::newParticipant(const QString &nick) {
    newParticipant(new QListWidgetItem(QIcon(":/resource/no-avatar.jpg"), nick));
}

void ChatWindow::newParticipant(QListWidgetItem *newParticipant) {
    ui->participantsListWidget->addItem(newParticipant);
    client->sendMessage(_myProfile, newParticipant->text());

    ui->noPeople_privateChatroom->hide();
    ui->noPeople->hide();
}

void ChatWindow::participantLeft(const QString &nick) {
    QList<QListWidgetItem *> toRemove = ui->participantsListWidget->findItems(nick, Qt::MatchExactly);
    for (QListWidgetItem *item : toRemove) {
        ui->participantsListWidget->removeItemWidget(item);
        delete item;
    }
    _profiles.remove(nick);

    if (ui->participantsListWidget->count() == 0) {
        ui->noPeople->show();
        if (_isPrivate) {
            ui->noPeople_privateChatroom->show();
        }
    }
}

void ChatWindow::on_textSend_clicked() {
    // Create the shared pointer containing a TextMessage/PrivateMessage.
    QSharedPointer<Message> message;
    if (_isPrivate) {
        message = QSharedPointer<Message>(new PrivateMessage(client->nickName(), windowTitle(), ui->lineEdit->text()));
    } else {
        message = QSharedPointer<Message>(new TextMessage(client->nickName(), ui->lineEdit->text()));
    }

    // Send to users
    _isPrivate ? sendMessageToParticipantList(message) : client->sendMessage(message);
    // Display for this computer (messages from this user are not passed to this user from the
    // network) - no point sending data to yourself!
    handleMessage(message);
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();
}

void ChatWindow::on_imageSend_clicked() {
    QString filename =
            QFileDialog::getOpenFileName(this,
                                         tr("Select an image... "), "",
                                         tr("Images (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm)"));
    if (!filename.isEmpty()) {
        // user selected a file
        QImage image(filename);
        if (image.isNull()) {
            // read failed
            QMessageBox::critical(this, tr("Error"), tr("Unable to open the image, please try again. "));
            return;
        }

        // create the message obj
        QSharedPointer<ImageMessage> message(
                _isPrivate ? new ImageMessage(client->nickName(), windowTitle(),
                                              filename.right(filename.size() - filename.lastIndexOf("/") - 1), image)
                           : new ImageMessage(client->nickName(),
                                              filename.right(filename.size() - filename.lastIndexOf("/") - 1), image));
        // send the message
        _isPrivate ? sendMessageToParticipantList(message) : client->sendMessage(message);
        // display on local
        handleMessage(message);
    }
}

void ChatWindow::on_lineEdit_textChanged(const QString &arg1) {
    ui->textSend->setEnabled(!arg1.isEmpty());
}

void ChatWindow::cleanupHistory() {
    while (chatHistory.size() > 100) {
        chatHistory.removeFirst();
    }
    historyModel.setStringList(chatHistory);

    // update _files
    QMap<int, QSharedPointer<FileMessage>> copy(_files);
    _files.clear();
    for (auto iter = copy.begin(); iter != copy.end(); ++iter) {
        if (!iter.key() - 1 < 0) {
            _files.insert(iter.key() - 1, iter.value());
        }
    }
}

QSharedPointer<QList<QListWidgetItem *>> ChatWindow::getAllOnlineUsers() const {
    // get the correct participantsListWidget pointer
    QListWidget *participantsListWidget;
    if (_isPrivate) {
        participantsListWidget = _parent->ui->participantsListWidget;
    } else {
        participantsListWidget = ui->participantsListWidget;
    }

    // putting in content (i.e. Items)
    QSharedPointer<QList<QListWidgetItem *>> allUsers = QSharedPointer<QList<QListWidgetItem *>>::create();
    for (int i = 0; i < participantsListWidget->count(); ++i) {
        // have a copy to prevent the delete of the QLWItem
        allUsers->append(new QListWidgetItem(*(participantsListWidget->item(i))));
    }

    // for private chat, remove participants that already joined
    if (_isPrivate) {
        for (int i = 0; i < ui->participantsListWidget->count(); ++i) {
            // use for loop to check each element if they equal
            // cannot use QList::removeAll() cus the QList stores pointers, and also QListWidgetItem didn't overload ==
            for (auto iter = allUsers->begin(); iter != allUsers->end(); /*increment in body*/) {
                if ((*iter)->text() == ui->participantsListWidget->item(i)->text()) {
                    iter = allUsers->erase(iter);
                } else {
                    ++iter;
                }
            }
        }
    }

    return allUsers;
}

void ChatWindow::on_lineEdit_returnPressed() {
    if (!ui->lineEdit->text().isEmpty()) {
        on_textSend_clicked();
    }
}

void ChatWindow::on_listView_doubleClicked(const QModelIndex &index) {
    auto file = _files.find(index.row());
    if (file != _files.end()) {
        QString filename = QFileDialog::getSaveFileName(this, tr("Save File"), file.value()->filename());

        if (filename != "") {
            // file name is not empty
            QFile qfile(filename);
            if (qfile.open(QIODevice::WriteOnly)) {
                // the file can be open
                qfile.write(file.value()->file());
                qfile.close();
            } else {
                QMessageBox::critical(this, tr("Error"), tr("Unable to save the file. "));
            }
        }
    }
}

void ChatWindow::on_fileSend_clicked() {
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Select a file... "), "",
                                                    tr("Files (*)"));
    if (!filename.isEmpty()) {
        // user selected a file
        QFile qfile(filename);
        QByteArray file;
        if (qfile.open(QIODevice::ReadOnly)) {
            // the file can be open
            file = qfile.readAll();
            qfile.close();
        } else {
            // read failed
            QMessageBox::critical(this, tr("Error"), tr("Unable to open the file, please try again. "));
            return;
        }

        // create the message obj
        QSharedPointer<FileMessage> message(
                _isPrivate ? new FileMessage(client->nickName(), windowTitle(),
                                             filename.right(filename.size() - filename.lastIndexOf("/") - 1), file)
                           : new FileMessage(client->nickName(),
                                             filename.right(filename.size() - filename.lastIndexOf("/") - 1), file));
        // send the message
        _isPrivate ? sendMessageToParticipantList(message) : client->sendMessage(message);
        // display on local
        handleMessage(message);
    }
}

void ChatWindow::on_setProfile_clicked() {
    if (!_isPrivate) {
        _setProfileWindow = new SetProfile(_myProfile.data(), true, this);
        _setProfileWindow->setWindowModality(Qt::ApplicationModal);
        _setProfileWindow->show();
    } else {
        _parent->on_setProfile_clicked();
    }
}

void ChatWindow::closeEvent(QCloseEvent *event) {
    if (_isPrivate) {
        // send out LEAVE message when closing the private window
        sendMessageToParticipantList(QSharedPointer<Message>(
                new ActionMessage(client->nickName(), ActionMessage::Action::LEAVE, windowTitle())));

        emit windowClosed(windowTitle());
    } else {
        if (!_privateChatWindows.isEmpty()) {
            // public chat window && still having private chat window(s) opening
            if (QMessageBox::question(this, "Exit",
                                      "Close public chat window will also close private chat window(s), continue?",
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                // close with deleting the private chatting windows
                for (auto &pair : _privateChatWindows) {
                    delete pair;
                }
                event->accept();
            } else {
                event->ignore();
            }
        }
    }
}

void ChatWindow::on_participantsListWidget_itemDoubleClicked(QListWidgetItem *item) {
    QMap<QString, QSharedPointer<IdentityMessage>>::Iterator profile;
    if (_isPrivate) {
        profile = _parent->_profiles.find(item->text());
    } else {
        profile = _profiles.find(item->text());
    }

    if (profile != _profiles.end() && profile.value() != nullptr) {
        _setProfileWindow = new SetProfile(profile.value().data(), false, this);
        _setProfileWindow->setWindowModality(Qt::ApplicationModal);
        _setProfileWindow->show();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("This user did not send any profile messages to us. "));
    }
}

void ChatWindow::on_exportChat_clicked() {
    QString filename = QFileDialog::getSaveFileName(this, tr("Export chat history to..."),
                                                    "", tr("HTML (*.html)"));
    if (!filename.isEmpty()) {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error"), tr("Unable to save the chat history, please try again. "));
            return;
        }
        QTextStream out(&file);

        // export the chat history into a HTML table
        out << "<table>";
        for (const auto &chatLine : chatHistory) {
            out << "<tr><td>";
            out << chatLine;
            out << "</tr></td>";
        }
        out << "</table>";

        file.close();
    }
}

void ChatWindow::privateChatWindowClosed(QString chatroomName) {
    auto privateWindow = _privateChatWindows.find(chatroomName);
    if (privateWindow != _privateChatWindows.end()) {
        delete privateWindow.value();
        _privateChatWindows.erase(privateWindow);
    }
}

void ChatWindow::selectParticipantsWindowClosed() {
    delete _selectParticipantsWindow;
}

void ChatWindow::on_createRoom_clicked() {
    _selectParticipantsWindow = new SelectParticipants(getAllOnlineUsers(), this, _isPrivate);
    _selectParticipantsWindow->setWindowModality(Qt::ApplicationModal);
    _selectParticipantsWindow->show();
}

void ChatWindow::inviteParticipants(QSharedPointer<QList<QListWidgetItem *> > participants,
                                    QString customChatroomName) {
    inviteParticipants(std::move(participants), nullptr, std::move(customChatroomName));
}

void ChatWindow::inviteParticipants(QSharedPointer<QList<QListWidgetItem *> > participants,
                                    QSharedPointer<ActionMessage> actionMessageIfBeingInvited,
                                    QString customChatroomName) {
    if (_isPrivate) {
        // PRIVATE ROOM
        if (actionMessageIfBeingInvited.isNull()) {
            // invite others from user selections
            for (auto *QLWIUser : *participants) {
                // send the invite message to all other receivers
                QSharedPointer<ActionMessage> message(
                        new ActionMessage(client->nickName(),
                                          ActionMessage::Action::INVITE,
                                          windowTitle(),
                                          QLWIUser->text()));
                client->sendMessage(message, QLWIUser->text());
            }
        } else {
            // accept the invite and send back join message
            // at this time, the participants list should have only one element
            client->sendMessage(QSharedPointer<ActionMessage>(
                    new ActionMessage(client->nickName(),
                                      ActionMessage::Action::JOIN,
                                      windowTitle(),
                                      actionMessageIfBeingInvited->secondOperand(),
                                      actionMessageIfBeingInvited->sender())),
                                actionMessageIfBeingInvited->sender());
            // add the people to the participant list
            newParticipant(new QListWidgetItem(*participants->first()));
            // update my identifier
            _myIdentifier = actionMessageIfBeingInvited->secondOperand();
        }
    } else {
        // PUBLIC ROOM, then create a private room
        auto *privateChatroomWindow =
                new ChatWindow(this,
                               actionMessageIfBeingInvited ? actionMessageIfBeingInvited->roomName()
                                                           : (
                                       customChatroomName.isNull()
                                       ? SelectParticipants::generatePrivateChatroomName(participants)
                                       : customChatroomName
                               ),
                               participants,
                               actionMessageIfBeingInvited);
        privateChatroomWindow->show();
        _privateChatWindows.insert(actionMessageIfBeingInvited ? actionMessageIfBeingInvited->roomName()
                                                               : (
                                           customChatroomName.isNull()
                                           ? SelectParticipants::generatePrivateChatroomName(participants)
                                           : customChatroomName
                                   ),
                                   privateChatroomWindow);
    }
}

void ChatWindow::handleActionMessage(QSharedPointer<ActionMessage> message) {
    auto privateWindow = _privateChatWindows.find(message->roomName());
    switch (message->action()) {
        case ActionMessage::Action::JOIN:
            if (privateWindow != _privateChatWindows.end()) {
                // has the room, otherwise discard the message
                if (!message->thirdOperand().isEmpty()) {
                    // this participant was invited by me
                    privateWindow.value()->_myIdentifier = message->thirdOperand(); // update my identifier

                    sendMessageToParticipantList(message->roomName(), QSharedPointer<ActionMessage>(
                            new ActionMessage(client->nickName(), ActionMessage::Action::JOIN, message->roomName(),
                                              message->sender())));
                } else {
                    // this participant was not invited by me
                    // Detailed explanation of this code block, see:
                    // https://lo.unisa.edu.au/pluginfile.php/1395052/mod_forum/post/1660745/protocol_chart.png
                    if (message->sender() != message->secondOperand()) {
                        // the message is from inviter to otherParticipants (the 3rd message in the img)
                        client->sendMessage(QSharedPointer<ActionMessage>(
                                new ActionMessage(client->nickName(), ActionMessage::Action::JOIN, message->roomName(),
                                                  privateWindow.value()->_myIdentifier)),
                                            message->secondOperand());
                    }   // else:
                    // the message is from otherParticipants to invitee (the 4th message in the img)
                    // later will add it to the participantListWidget
                }

                // add the new participant into the list
                privateWindow.value()->newParticipant(new QListWidgetItem(
                        *ui->participantsListWidget->findItems(message->secondOperand(), Qt::MatchExactly).first()));
            }
            break;
        case ActionMessage::Action::LEAVE:
            if (privateWindow != _privateChatWindows.end()) {
                // ↑ make sure the window exist, if not just discard
                // ↓ remove this participant from the list widget
                privateWindow.value()->participantLeft(message->sender());
            }
            break;
        case ActionMessage::Action::KICK:
            if (privateWindow != _privateChatWindows.end()) {
                // make sure the window exist, if not just discard
                if (message->secondOperand() == privateWindow.value()->_myIdentifier) {
                    // I'm the one to be kicked :(
                    // then close the window directly, when closing, it will also send out a LEAVE message
                    privateWindow.value()->close();
                    QMessageBox::information(this, "Kicked",
                                             message->sender() + " kicked you out of room \"" + message->roomName() +
                                             "\". ");
                } else {
                    // kick this guy from the list
                    participantLeft(message->secondOperand());
                }
            }
            break;
        case ActionMessage::INVITE:
            if (QMessageBox::question(this, "Invite",
                                      message->sender() + " invites you to join the room \"" + message->roomName()
                                      + "\", do you accept? ",
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                inviteParticipants(
                        QSharedPointer<QList<QListWidgetItem *>>::create(
                                ui->participantsListWidget->findItems(message->sender(), Qt::MatchExactly)),
                        message);
            }
            break;
    }
}

void ChatWindow::sendMessageToParticipantList(QSharedPointer<Message> message) {
    for (int i = 0; i < ui->participantsListWidget->count(); ++i) {
        // for each already participated participants, send out the message
        client->sendMessage(message, ui->participantsListWidget->item(i)->text());
    }
}

void ChatWindow::sendMessageToParticipantList(QString roomName, QSharedPointer<Message> message) {
    _privateChatWindows.find(roomName).value()->sendMessageToParticipantList(std::move(message));
}

void ChatWindow::on_participantsListWidget_customContextMenuRequested(const QPoint &pos) {
    if (!ui->participantsListWidget->selectedItems().isEmpty()) { // make sure a item is selected
        // Create menu and insert actions
        QMenu participantRightClickMenu;
        participantRightClickMenu.setDefaultAction(
                participantRightClickMenu.addAction(QIcon(":/resource/profile.png"), "View profile",
                                                    this,
                                                    [=]() {
                                                        on_participantsListWidget_itemDoubleClicked(
                                                                ui->participantsListWidget->selectedItems().first());
                                                    }));
        // ↓ if the ChatBot can send a rude text message (a PM without sending an invitation), I also can do this
        participantRightClickMenu.addAction(QIcon(":/resource/PM.png"), "Quick PM",
                                            this,
                                            [=]() {
                                                QString receiver =
                                                        ui->participantsListWidget->selectedItems().first()->text();
                                                bool ok;
                                                QString text =
                                                        QInputDialog::getText(this,
                                                                              QString("PM to " + receiver),
                                                                              QString("<span style=\" "
                                                                                              "font-weight:600;\">"
                                                                                              "Quick PM to " + receiver
                                                                                      + ":</span><br/><span style=\" "
                                                                                              "color:#ff0000;\">Note: "
                                                                                              "</span>Depends on the "
                                                                                              "receiver's client, a "
                                                                                              "new chatroom may or may "
                                                                                              "not be created."),
                                                                              QLineEdit::Normal, QString(), &ok);
                                                if (ok) {
                                                    auto message = QSharedPointer<PrivateMessage>(
                                                            new PrivateMessage(client->nickName(), receiver, text));
                                                    client->sendMessage(message, receiver);

                                                    // if has such room, append the message in
                                                    auto &privateChatWindows = _isPrivate ? _parent->_privateChatWindows
                                                                                          : _privateChatWindows;
                                                    auto privateWindow = privateChatWindows.find(receiver);
                                                    if (privateWindow != privateChatWindows.end()) {
                                                        privateWindow.value()->handleMessage(message);
                                                    }
                                                }
                                            });
        // ↓ and... also allow user sending a rude file message
        participantRightClickMenu.addAction(QIcon(":/resource/file.png"), "Quick file",
                                            this,
                                            [=]() {
                                                QString receiver =
                                                        ui->participantsListWidget->selectedItems().first()->text();
                                                QString filename =
                                                        QFileDialog::getOpenFileName(this,
                                                                                     "Send a file to " + receiver, "",
                                                                                     tr("Files (*)"));
                                                if (!filename.isEmpty()) {
                                                    // user selected a file
                                                    QFile qfile(filename);
                                                    QByteArray file;
                                                    if (qfile.open(QIODevice::ReadOnly)) {
                                                        // the file can be open
                                                        file = qfile.readAll();
                                                        qfile.close();
                                                    } else {
                                                        // read failed
                                                        QMessageBox::critical(this, tr("Error"),
                                                                              tr("Unable to open the file, please try "
                                                                                         "again. "));
                                                        return;
                                                    }
                                                    auto message = QSharedPointer<FileMessage>(
                                                            new FileMessage(client->nickName(), receiver,
                                                                            filename.right(filename.size() -
                                                                                           filename.lastIndexOf("/") -
                                                                                           1),
                                                                            file));
                                                    client->sendMessage(message, receiver);
                                                    // if has such room, append the message in
                                                    auto &privateChatWindows = _isPrivate ? _parent->_privateChatWindows
                                                                                          : _privateChatWindows;
                                                    auto privateWindow = privateChatWindows.find(receiver);
                                                    if (privateWindow != privateChatWindows.end()) {
                                                        privateWindow.value()->handleMessage(message);
                                                    }
                                                }
                                            });
        if (_isPrivate) {
            participantRightClickMenu.addAction(QIcon(":/resource/kick.png"), "Kick from chatroom",
                                                this,
                                                [=]() {
                                                    // send out the KICK message
                                                    sendMessageToParticipantList(
                                                            QSharedPointer<Message>(
                                                                    new ActionMessage(client->nickName(),
                                                                                      ActionMessage::Action::KICK,
                                                                                      windowTitle(),
                                                                                      ui->participantsListWidget->
                                                                                              selectedItems().first()->
                                                                                              text())));
                                                    // remove from the list
                                                    participantLeft(ui->participantsListWidget->
                                                            selectedItems().first()->text());
                                                });
        }

        // Show context menu
        participantRightClickMenu.exec(ui->participantsListWidget->mapToGlobal(pos));
    }
}

