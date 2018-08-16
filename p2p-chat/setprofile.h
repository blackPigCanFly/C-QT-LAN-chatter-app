#ifndef SETPROFILE_H
#define SETPROFILE_H

#include <QMainWindow>

#include "identitymessage.h"

namespace Ui {
    class SetProfile;
}

/**
 * @brief The SetProfile class is the SetProfile window. This window also can be used to view a profile.
 */
class SetProfile : public QMainWindow {
Q_OBJECT

public:
    /**
     * @brief SetProfile constructor
     * @param im the IdentityMessage if has (nullptr means it's first time boot up)
     * @param myself bool value, true if this is my profile
     * @param parent the parent window
     */
    explicit SetProfile(IdentityMessage *im, bool myself, QWidget *parent = nullptr);

    ~SetProfile() override;

protected:
    /**
     * @brief closeEvent override the close event to emit windowClosed() signal
     * @param event the close event
     */
    void closeEvent(QCloseEvent *event) override;

private slots:

    /**
     * @brief on_setAvatar_clicked set avatar button clicked. Prompt the user to select a image.
     */
    void on_setAvatar_clicked();

    /**
     * @brief on_buttonBox_accepted when OK button is clicked. emit profileUpdate() signal and close the window.
     */
    void on_buttonBox_accepted();

    /**
     * @brief on_buttonBox_rejected when Cancel button is clicked. Close the window.
     */
    void on_buttonBox_rejected();

    /**
     * @brief on_nickname_textChanged when the nickname text changed. If the text is empty, OK button is disabled
     * @param arg1 the text in the nickname text box
     */
    void on_nickname_textChanged(const QString &arg1);

signals:

    /**
     * @brief profileUpdated emit when user clicked OK button.
     */
    void profileUpdated(IdentityMessage *);

    /**
     * @brief windowClosed emit when the window is closed.
     */
    void windowClosed();

private:
    /**
     * @brief ui the UI
     */
    Ui::SetProfile *ui;
};

#endif // SETPROFILE_H
