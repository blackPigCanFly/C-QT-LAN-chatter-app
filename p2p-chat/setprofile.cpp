#include "setprofile.h"
#include "ui_setprofile.h"
#include <QFileDialog>
#include <QHostInfo>
#include <QMessageBox>
#include <QCloseEvent>

SetProfile::SetProfile(IdentityMessage *im, bool myself, QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::SetProfile) {
    ui->setupUi(this);

    ui->timezone->setCurrentText("000"); // apply the default timezone

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false); // OK button disable by default

    if (!myself) {
        // not my profile, set the ui as READ ONLY
        ui->nickname->setEnabled(false);
        ui->location->setEnabled(false);
        ui->timezone->setEnabled(false);
        // set the avatar button to invisible
        auto sp = ui->setAvatar->sizePolicy();
        sp.setRetainSizeWhenHidden(true);
        ui->setAvatar->setSizePolicy(sp);
        ui->setAvatar->hide();
        // update the buttons on the bottom
        QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
        ui->buttonBox->removeButton(okButton);
    } else {
        // IS MY PROFILE
        connect(this, SIGNAL(profileUpdated(IdentityMessage * )), parent, SLOT(updateProfile(IdentityMessage * )));
    }

    if (im != nullptr) {
        // has the information (ie. not null), then populate info into UI
        ui->nickname->setText(im->username());
        ui->nickname->setEnabled(false);
        ui->location->setText(im->location());
        ui->timezone->setCurrentText(im->timezone());
        ui->avatarDisplay->setPixmap(QPixmap::fromImage(im->image().isNull()
                                                        ? QImage(":/resource/no-avatar.jpg") : im->image()));
    } else {
        // FIRST TIME BOOT UP
        // set the name and display
        ui->avatarDisplay->setPixmap(QPixmap::fromImage(QImage(":/resource/no-avatar.jpg")));

        connect(this, SIGNAL(windowClosed()), parent, SLOT(setProfileClosedWithoutProfile()));

        // auto select system's current timezone to optimise the UX
        QTimeZone sysTZ(QTimeZone::systemTimeZone());
        QString tzName = sysTZ.displayName(QTimeZone::TimeType::StandardTime, QTimeZone::NameType::OffsetName);
        tzName.remove(':');
        tzName.remove("UTC");
        tzName.replace("+0", "+");
        tzName.replace("-0", "-");
        ui->timezone->setCurrentText(tzName);
    }

    if (myself && im != nullptr) {
        // SECOND TIME MODIFY PROFILE
        ui->nickname->setEnabled(false);
    }

    connect(this, SIGNAL(windowClosed()), parent, SLOT(setProfileWindowClosed()));
}

SetProfile::~SetProfile() {
    delete ui;
}

void SetProfile::closeEvent(QCloseEvent *event) {
    emit windowClosed();
    event->accept();
}

void SetProfile::on_setAvatar_clicked() {
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

        ui->avatarDisplay->setPixmap(QPixmap::fromImage(image).scaled(48, 48));
    }
}

void SetProfile::on_buttonBox_accepted() {
    disconnect(this, SIGNAL(windowClosed()), parent(), SLOT(setProfileClosedWithoutProfile()));
    emit profileUpdated(new IdentityMessage(ui->nickname->text() + '@' + QHostInfo::localHostName(),
                                            ui->nickname->text(),
                                            ui->location->text(),
                                            ui->timezone->currentText(),
                                            ui->avatarDisplay->pixmap()->toImage() != QImage(":/resource/no-avatar.jpg")
                                            ? ui->avatarDisplay->pixmap()->toImage() : QImage()));
    this->close();
}

void SetProfile::on_buttonBox_rejected() {
    this->close();
}

void SetProfile::on_nickname_textChanged(const QString &arg1) {
    auto *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (okButton) {
        okButton->setEnabled(!arg1.isEmpty());
    }
}
