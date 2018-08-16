#ifndef PRIVATEMESSAGE_H
#define PRIVATEMESSAGE_H

#include "message.h"


class PrivateMessage : public Message {
public:
    /**
    * @brief PrivateMessage constructor
    * @param sender the sender of this message
    * @param receiver the receiver of this message, it can either be a room name or a username
    * @param message the message text
    * @param timestamp the creation or received time of this message
    */
    PrivateMessage(const QString &sender, const QString &receiver, const QString &message,
                   const QDateTime &timestamp = QDateTime::currentDateTime());

    /**
    * @brief data convert data to the format required by the network.
    * @return a QByteArray containing a network compatible representation of this PrivateMessage.
    */
    QByteArray data() const override;

    /**
     * @brief receiver retrieves the receiver of this message, it can either be a room name or a username
     * @return the receiver of this message
     */
    QString receiver() const;

    /**
     * @brief message retrieves the message text
     * @return the message text
     */
    QString message() const;

private:
    const QString _receiver; /**< the receiver of this message, it can either be a room name or a username*/
    const QString _message; /**< the message text. */
};

#endif // PRIVATEMESSAGE_H
