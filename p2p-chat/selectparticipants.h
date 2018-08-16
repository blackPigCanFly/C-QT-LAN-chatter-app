#ifndef SELECTPARTICIPANTS_H
#define SELECTPARTICIPANTS_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
    class SelectParticipants;
}
/**
 * @brief The SelectParticipants class is the UI for selecting participants and setting chatroom name
 */
class SelectParticipants : public QDialog {
Q_OBJECT

public:
    /**
     * @brief SelectParticipants constructor
     * @param allUsers the users should be put into the selection list
     * @param parent the parent window
     * @param isPrivate is requested from a private window
     */
    explicit SelectParticipants(QSharedPointer<QList<QListWidgetItem *> > allUsers, QWidget *parent, bool isPrivate);

    ~SelectParticipants() override;

    /**
     * @brief generatePrivateChatroomName can generate a chatroom name based on the participants selected
     * @param participants the participants selected
     * @return the name of the room
     */
    static QString generatePrivateChatroomName(QSharedPointer<QList<QListWidgetItem *>> participants);

protected:
    /**
     * @brief closeEvent override to emit a windowClosed() signal
     * @param event the close event
     */
    void closeEvent(QCloseEvent *event) override;

signals:

    /**
     * @brief selectedParticipants emitted when OK button is clicked with the participants selected
     */
    void selectedParticipants(QSharedPointer<QList<QListWidgetItem *>>);

    /**
     * @brief selectedParticipants emitted when OK button is clicked with the participants selected and the name of the
     * chatroom name
     */
    void selectedParticipants(QSharedPointer<QList<QListWidgetItem *>>, QString);

    /**
     * @brief windowClosed emitted when window is closed
     */
    void windowClosed();

private slots:

    /**
     * @brief on_buttonBox_rejected when Cancel button is clicked, close the window
     */
    void on_buttonBox_rejected();

    /**
     * @brief on_buttonBox_accepted when OK button is clicked, emit the signals and close the window
     */
    void on_buttonBox_accepted();

    /**
     * @brief on_allUserList_itemSelectionChanged when item selection changed, used to update chatroom name and OK button
     * enable state
     */
    void on_allUserList_itemSelectionChanged();

    /**
     * @brief on_chatroomName_textEdited  used to update _chatroomNameChanged
     * @param arg1 the text inside the input box
     */
    void on_chatroomName_textEdited(const QString &arg1);

private:
    /**
     * @brief ui the UI.
     */
    Ui::SelectParticipants *ui;

    /**
     * @brief _chatroomNameChanged record if the chatroomName is manually changed or not.
     */
    bool _chatroomNameChanged;
};

#endif // SELECTPARTICIPANTS_H
