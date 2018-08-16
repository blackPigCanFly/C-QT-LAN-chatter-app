#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include "./Networking/client.h"
#include "filemessage.h"
#include "message.h"
#include "setprofile.h"
#include "selectparticipants.h"
#include "actionmessage.h"
#include <QMainWindow>
#include <QStringList>
#include <QStringListModel>
#include <QMap>
#include <QListWidgetItem>
#include <QCloseEvent>

namespace Ui {
    class ChatWindow;
}

/**
 * @brief The ChatWindow class represents the ChatWindow, it is used for both public chat
 * and private chat
 */
class ChatWindow : public QMainWindow {
Q_OBJECT

public:
    /**
     * @brief ChatWindow the default constructor
     * @param parent parent QWidget
     */
    explicit ChatWindow(QWidget *parent = nullptr);

    /**
     * @brief ChatWindow for creating a private chatroom only
     * @param parent the parent window (MUST BE the public chat window)
     * @param chatroomName the name of the chatroom
     * @param participants the list of initial participants in the room (names should be nick[@]ip)
     */
    explicit ChatWindow(ChatWindow *parent, QString chatroomName,
                        QSharedPointer<QList<QListWidgetItem *>> participants,
                        QSharedPointer<ActionMessage> actionMessageIfBeingInvited = nullptr);

    ~ChatWindow() override;

public slots:

    /**
     * @brief handleMessage is responsible for handling multiple types of messages from
     * the internet, this method SHOULD ONLY BE used in the public chat window, otherwise
     * will crash
     * @param message the message to be handled
     */
    void handleMessage(QSharedPointer<Message> message);

    /**
     * @brief appendMessage append a text message into the UI
     * @param from message sender
     * @param message message content
     */
    void appendMessage(const QString &from, const QString &message);

    /**
     * @brief appendMessage append an Image or File message into the UI
     * @param from message sender
     * @param raw the raw data of the image or file
     * @param isImage true if image, false if file
     */
    void appendMessage(const QString &from, const QByteArray &raw, bool isImage);

    /**
     * @brief updateProfile slot is used to process when user's profile changed
     * @param im the IdentityMessage of the user (myself only)
     */
    void updateProfile(IdentityMessage *im);

    /**
     * @brief setProfileClosedWithoutProfile when the first time run SetProfile
     * window closed but didn't return a IdentityMessage (i.e. invoke updateProfile
     * method), this slot should be called to close the whole program as no info
     * was received and the program cannot be continued.
     */
    void setProfileClosedWithoutProfile();

    /**
     * @brief setProfileWindowClosed should be called when the SetProfile window closed,
     * to destroy the window to avoid memory leak.
     */
    void setProfileWindowClosed();

    /**
     * @brief selectParticipantsWindowClosed should be called when the selectParticipant window
     * closed, to destroy the window to avoid memory leak.
     */
    void selectParticipantsWindowClosed();

    /**
     * @brief on_setProfile_clicked when the setProfile button on the UI is clicked. If it's in
     * the private chatroom, this method in the parent window (i.e. public room) will be invoked.
     * A setProfile window will pop up with user info populated
     */
    void on_setProfile_clicked();

    /**
     * @brief inviteParticipants should be invoked in public chat only. When invoked, this method will
     * pass parameters to the private member inviteParticipants(), see inviteParticipants() for more
     * details.
     * @param participants a list of all participants should be sent to the room.
     * @param customChatroomName used for creating a chatroom with custom name
     */
    void inviteParticipants(QSharedPointer<QList<QListWidgetItem *> > participants,
                            QString customChatroomName = QString());

protected:
    /**
     * @brief closeEvent when the window closed, if the window is a private chatroom, send out LEAVE
     * messages; if public, check if there is any private chatrooms opening and prompt the user.
     * @param event the close event
     */
    void closeEvent(QCloseEvent *event) override;

private slots:

    /**
     * @brief newParticipant when a new participant comes in, this method populates the user
     * into the participant list on the RHS of the window
     * @param nick the participant's nick name (identifier)
     */
    void newParticipant(const QString &nick);

    /**
     * @brief participantLeft when a participant leave, this method removes the user from the
     * participant list on the RHS of the window
     * @param nick the participant's nick name (identifier)
     */
    void participantLeft(const QString &nick);

    /**
     * @brief on_imageSend_clicked when the sending image button clicked, this method pops up a
     * file selection window and then sends out the ImageMessage. This method fits for private
     * chatroom.
     */
    void on_imageSend_clicked();

    /**
     * @brief on_textSend_clicked when the send text button clicked, just simply send the message
     * out. This method fits for private chatroom.
     */
    void on_textSend_clicked();

    /**
     * @brief on_lineEdit_textChanged when the text changed, it checks the text length. If the text
     * is empty, then textSend button is disabled.
     * @param arg1 the text of lineEdit
     */
    void on_lineEdit_textChanged(const QString &arg1);

    /**
     * @brief on_lineEdit_returnPressed when return (enter) key pressed on lineEdit, it simulates the
     * textSend button click action.
     */
    void on_lineEdit_returnPressed();

    /**
     * @brief on_listView_doubleClicked when the chat history double clicked, it checks if the message
     * is a file, if it is, then prompts the user to save the file.
     * @param index the index of the listView clicked
     */
    void on_listView_doubleClicked(const QModelIndex &index);

    /**
     * @brief on_fileSend_clicked similar to imageSend button, this method invoked when fileSend button
     * clicked, then it prompts the file, and send out the file in the form of FileMessage
     */
    void on_fileSend_clicked();

    /**
     * @brief on_participantsListWidget_itemDoubleClicked when the item double clicked in the participant
     * list, pops up a setProfile window to view this people's profile.
     * @param item the Item being double clicked
     */
    void on_participantsListWidget_itemDoubleClicked(QListWidgetItem *item);

    /**
     * @brief on_exportChat_clicked exportChat button, prompt the user where to save, and save the chat
     * history as a html file.
     */
    void on_exportChat_clicked();

    /**
     * @brief privateChatWindowClosed when a private chatroom window closed, this method should be called
     * to recycle the window
     * @param chatroomName the name of the chatroom
     */
    void privateChatWindowClosed(QString chatroomName);

    /**
     * @brief on_createRoom_clicked when createRoom button is clicked, a selectParticipants window is pop
     * up.
     */
    void on_createRoom_clicked();

    /**
     * @brief on_participantsListWidget_customContextMenuRequested when participant list right clicked, show
     * the contextMenu, including view profile, send PM, kick (private chatroom only), etc.
     * @param pos the position of the mouse right clicked
     */
    void on_participantsListWidget_customContextMenuRequested(const QPoint &pos);

signals:

    /**
     * @brief windowClosed emit when the window is closed, with parameter of the window title,
     * for private chatroom, this is the chatroom name.
     */
    void windowClosed(QString);

private:
    /**
     * @brief cleanupHistory cleanups the chat history when the it reaches 100 entries
     */
    void cleanupHistory();

    /**
     * @brief newParticipant an overloaded method to add a new participant into the list
     * @param newParticipant the new participant's QListWidgetItem, a Icon should be associated as well
     */
    void newParticipant(QListWidgetItem *newParticipant);

    /**
     * @brief handleActionMessage used to handle an ActionMessage. Fit for both public & private purposes.
     * @param message the ActionMessage to be handled
     */
    void handleActionMessage(QSharedPointer<ActionMessage> message);

    /**
     * @brief getAllOnlineUsers gets all online users in the public room. However, when you are in a
     * private room, this method will remove the participants in the private room already
     * @return a list of all participants in the public room, without the participants already in the
     * private room (if is in a private chatroom)
     */
    QSharedPointer<QList<QListWidgetItem *>> getAllOnlineUsers() const;

    /**
     * @brief inviteParticipants invites the participant into a private room, and is also able to handle
     * a invite ActionMessage from another client. Fit for both public and private room.
     * @param participants the participants to add them in to the room
     * @param actionMessageIfBeingInvited pass in the invite ActionMessage (if has)
     * @param customChatroomName custom a chatroom name, it will be ignored when actionMessageIfBeingInvited
     * is not null as the chatroom name can be retrieved from actionMessageIfBeingInvited.
     */
    void inviteParticipants(QSharedPointer<QList<QListWidgetItem *> > participants,
                            QSharedPointer<ActionMessage> actionMessageIfBeingInvited,
                            QString customChatroomName = QString());

    /**
     * @brief sendMessageToParticipantList sends the message to all participants on the participantList.
     * You can use it for both public and private chatroom, but in public room, you should use client
     * (p2pnetworking::Client) instead.
     * @param message the message to be sent
     */
    void sendMessageToParticipantList(QSharedPointer<Message> message);

    /**
     * @brief sendMessageToParticipantList send the message to all participants of a specific room,
     * ONLY can be used for public chatroom, otherwise will crash. However, the message won't be
     * displayed in the private chatroom.
     * @param roomName the roomName to send to
     * @param message the message to be sent
     */
    void sendMessageToParticipantList(QString roomName, QSharedPointer<Message> message);

    /**
     * @brief ui the ChatWindow UI
     */
    Ui::ChatWindow *ui;

    /**
     * @brief _parent is used for private chatroom ONLY. Record the parent ChatWindow (i.e. the public
     * chatroom)
     */
    ChatWindow *_parent;

    /**
     * @brief _myIdentifier is used for private chatroom ONLY. Records my identifier (myName[@]ip)
     */
    QString _myIdentifier;

    /**
     * @brief client the network client
     */
    QSharedPointer<p2pnetworking::Client> client;

    /**
     * @brief chatHistory the chat history of the chatroom
     */
    QStringList chatHistory;

    /**
     * @brief historyModel the chat history of the chatroom
     */
    QStringListModel historyModel;

    /**
     * @brief _files stores all received (incl sent) files indexes in listView and the FileMessage
     */
    QMap<int, QSharedPointer<FileMessage>> _files;

    /**
     * @brief _profiles stores all received user profiles. Used for public room ONLY
     */
    QMap<QString, QSharedPointer<IdentityMessage>> _profiles;

    /**
     * @brief _privateChatWindows stores all private chatting windows. Used for public room ONLY
     */
    QMap<QString, ChatWindow *> _privateChatWindows;

    /**
     * @brief _setProfileWindow the setProfile window. Should be used for public room ONLY
     */
    SetProfile *_setProfileWindow;

    /**
     * @brief _selectParticipantsWindow
     */
    SelectParticipants *_selectParticipantsWindow;

    /**
     * @brief _myProfile stores my profile. Used for public room ONLY
     */
    QSharedPointer<IdentityMessage> _myProfile;

    /**
     * @brief _chatStarted uses to record if the chat is started or not, used for public room ONLY
     */
    bool _chatStarted;

    /**
     * @brief _isPrivate uses to record if the room is a private chatroom or not (public chatroom)
     */
    bool _isPrivate;
};

#endif // CHATWINDOW_H
