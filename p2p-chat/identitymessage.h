#ifndef IDENTITYMESSAGE_H
#define IDENTITYMESSAGE_H

#include <QImage>
#include <QTimeZone>
#include "message.h"

/**
 * @brief IdentityMessage represents a user's profile
 */

class IdentityMessage : public Message {
public:
    /**
     * @brief IdentityMessage constructor
     * @param sender the sender of this message
     * @param username the username of the account
     * @param location the location of the user
     * @param timezone time zone format sample: +930
     * @param image the avatar of the user
     * @param timestamp the creation or received time of this message
     */
    IdentityMessage(const QString &sender,
                    const QString &username,
                    const QString &location,
                    const QString &timezone,
                    const QImage &image,
                    const QDateTime &timestamp = QDateTime::currentDateTime());

    /**
     * @brief data convert data to the format required by the network.
     * @return a QByteArray containing a network compatible representation of this IdentityMessage.
     */
    QByteArray data() const override;

    /**
    * @brief retrieve the username text
    * @return the username text
    */
    QString username() const;

    /**
    * @brief retrieve the location text
    * @return the location text
    */
    QString location() const;

    /**
    * @brief retrieve the timezone of the user
    * @return the timezone
    */
    QString timezone() const;

    /**
    * @brief retrieve the user's avatar
    * @return the avatar image
    */
    QImage image() const;

    /**
     * @brief imageRaw retrieves the image raw data inside a QByteArray
     * @return image raw data inside a QByteArray
     */
    QByteArray imageRaw() const;

private:
    const QString _username; /**< the user's name */
    const QString _location; /**< the user's location */
    const QString _timezone; /**< the user's timezone */
    const QImage _image; /**< the user's avatar */
};

#endif // IDENTITYMESSAGE_H
