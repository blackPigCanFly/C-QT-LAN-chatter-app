#ifndef FILEMESSAGE_H
#define FILEMESSAGE_H

#include "message.h"

/**
 * @brief FileMessage class represents sending a file through the network
 */
class FileMessage : public Message {
public:
    /**
     * @brief Constructor for FileMessage
     * @param sender the sender of this message
     * @param filename the filename of this file of this message
     * @param file the data of this file of this message
     * @param timestamp the creation or received time of this message
     */
    FileMessage(const QString &sender,
                const QString &filename,
                const QByteArray &file,
                const QDateTime &timestamp = QDateTime::currentDateTime());

    /**
     * @brief Constructor for FileMessage
     * @param chatroomName the chatroom name or recipient identifier (private message)
     * @param sender the sender of this message
     * @param filename the filename of this file of this message
     * @param file the data of this file of this message
     * @param timestamp the creation or received time of this message
     */
    FileMessage(const QString &sender,
                const QString &chatroomName,
                const QString &filename,
                const QByteArray &file,
                const QDateTime &timestamp = QDateTime::currentDateTime());

    /**
    * @brief data convert data to the format required by the network.
    * @return a QByteArray containing a network compatible representation of this FileMessage.
    */
    QByteArray data() const override;

    /**
     * @brief retrieve the filename of the file
     * @return the filename
     */
    QString filename() const;

    /**
     * @brief retrieve the file data
     * @return the file data
     */
    QByteArray file() const;

    /**
     * @brief chatroomName retrieves the chatroom name (private message)
     * @return the chatroom name or recipient identifier (private message)
     */
    QString chatroomName() const;

    /**
     * @brief isPrivate check if this message is a private message
     * @return true is private
     */
    bool isPrivate() const;

private:
    const QString _chatroomName; /**< the chatroom name or recipient identifier (private message) */
    const QString _filename; /**< the filename of this file of this message */
    const QByteArray _file; /**< the data of this file of this message */
};

#endif // FILEMESSAGE_H
