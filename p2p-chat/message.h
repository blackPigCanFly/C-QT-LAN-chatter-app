#ifndef MESSAGE_H
#define MESSAGE_H

#include <QByteArray>
#include <QDateTime>

/**
 * @brief The Message class represents a message passed between peers.
 */
class Message {
public:
    /**
     * @brief Message simple constructor
     * @param sender the sender's identity
     * @param timestamp the time the message was created or received
     */
    Message(QString sender, QDateTime timestamp);

    /**
     * @brief ~Message virtual destructor - best practice for classes with virtual members.
     */
    virtual ~Message();

    /**
     * @brief sender retrieve the sender's identity.
     * @return the sender's identity.
     */
    QString sender() const;

    /**
     * @brief data convert the message to the format expected for transmission over the network.
     * @return a QByteArray containing the data to send across the network.
     */
    virtual QByteArray data() const = 0;

    /**
     * @brief isEmpty test if this message has any content
     * @return true if this message has no content
     */
    virtual bool isEmpty() const;

    /**
     * @brief timestamp retrieve the creation or received time of this message.
     * @return the creation or receive time of the message.
     */
    QDateTime timestamp() const;

    // Message Type ids, these are used on the network.
    static const int IdentityMessage = 0;
    static const int TextMessage = 1;
    static const int ActionMessage = 2;
    static const int FileMessage = 3;
    static const int ImageMessage = 4;
    static const int PrivateMessage = 5;
    // The separator character is used to delimit data. It is reserved, make sure you do not allow
    // your users to send it (unless you HTML encode it).
    static const char Separator = '|';
    static const QString SeparatorHTMLCode;

private:
    QString _sender;      /**< the sender's identity. */
    QDateTime _timestamp; /**< the creation or received time of this message. */
};

#endif // MESSAGE_H
