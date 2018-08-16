#include "selectparticipants.h"
#include "ui_selectparticipants.h"

#include <QPushButton>
#include <QCloseEvent>

SelectParticipants::SelectParticipants(QSharedPointer<QList<QListWidgetItem *> > allUsers,
                                       QWidget *parent,
                                       bool isPrivate)
        : QDialog(parent),
          ui(new Ui::SelectParticipants),
          _chatroomNameChanged(false) {
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint); // remove question mark icon
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
            false); // set the ok button grey at the beginning (no selection)

    // populate users in
    for (auto *user : *allUsers) {
        ui->allUserList->addItem(user);
    }

    connect(this, SIGNAL(windowClosed()), parent, SLOT(selectParticipantsWindowClosed()));
    if (!isPrivate) {
        connect(this, SIGNAL(selectedParticipants(QSharedPointer<QList<QListWidgetItem *>>, QString)),
                parent, SLOT(inviteParticipants(QSharedPointer<QList<QListWidgetItem *>>, QString)));
    } else {
        connect(this, SIGNAL(selectedParticipants(QSharedPointer<QList<QListWidgetItem *>>)),
                parent, SLOT(inviteParticipants(QSharedPointer<QList<QListWidgetItem *>>)));
        // Inviting new participants into an existing chatroom, chatroom doesn't allowed to change
        ui->chatroomNameLabel->setEnabled(false);
        ui->chatroomName->setEnabled(false);
        ui->chatroomName->setText(parent->windowTitle()); // set the correct chatroom name
    }
}

SelectParticipants::~SelectParticipants() {
    delete ui;
}

void SelectParticipants::on_buttonBox_rejected() {
    this->close();
}

void SelectParticipants::on_buttonBox_accepted() {
    if (!_chatroomNameChanged) {
        ui->chatroomName->setText(generatePrivateChatroomName(
                QSharedPointer < QList < QListWidgetItem * >> ::create(ui->allUserList->selectedItems())));
    }

    auto participants = QSharedPointer < QList < QListWidgetItem * >> ::create(ui->allUserList->selectedItems());
    emit selectedParticipants(participants);
    emit selectedParticipants(participants, ui->chatroomName->text());

    this->close();
}

void SelectParticipants::closeEvent(QCloseEvent *event) {
    emit windowClosed();
    event->accept();
}

void SelectParticipants::on_allUserList_itemSelectionChanged() {
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->allUserList->selectedItems().length() != 0);

    if (!_chatroomNameChanged) {
        ui->chatroomName->setText(generatePrivateChatroomName(
                QSharedPointer < QList < QListWidgetItem * >> ::create(ui->allUserList->selectedItems())));
    }
}

QString SelectParticipants::generatePrivateChatroomName(QSharedPointer<QList<QListWidgetItem *>> participants) {
    if (participants->length() == 0) {
        return QString();
    } else if (participants->length() == 1) {
        return participants->first()->text();
    } else if (participants->length() == 2) {
        // two participants
        return "Chatting with " + participants->first()->text() + " and " + participants->last()->text();
    } else {
        // more than 1 participants
        return "Chatting with " + participants->first()->text() + ", " + participants->last()->text() + " and more";
    }
}


void SelectParticipants::on_chatroomName_textEdited(const QString &arg1) {
    _chatroomNameChanged = arg1.size() != 0;
}
