#ifndef TEXTMESSAGE_H
#define TEXTMESSAGE_H

#include "message.h"

/**
 * @brief The TextMessage class a message sent to the public chat.
 */
class TextMessage : public Message {
public:
    /**
     * @brief TextMessage constructor
     * @param sender the sender of this message
     * @param message the message text
     * @param timestamp the creation or received time of this message
     */
    TextMessage(const QString &sender, const QString &message,
                const QDateTime &timestamp = QDateTime::currentDateTime());

    /**
     * @brief data convert data to the format required by the network.
     * @return a QByteArray containing a network compatible representation of this TextMessage.
     */
    QByteArray data() const override;

    /**
     * @brief message retrieve the message text
     * @return the message text
     */
    QString message() const;

private:
    QString _message; /**< the message text. */
};

#endif // TEXTMESSAGE_H
