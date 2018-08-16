#ifndef IMAGEMESSAGE_H
#define IMAGEMESSAGE_H

#include "message.h"

#include <QImage>


class ImageMessage : public Message {
public:
    /**
     * @brief ImageMessage Constructor for ImageMessage
     * @param sender the sender of this message
     * @param name the name of image file of this message
     * @param image the image data of this image of this message
     * @param timestamp the creation or received time of this message
     */
    ImageMessage(const QString &sender, const QString &name, const QImage &image,
                 const QDateTime &timestamp = QDateTime::currentDateTime());

    /**
     * @brief ImageMessage Constructor for ImageMessage (private room used only)
     * @param sender the sender of this message
     * @param chatroomName the chatroom name or recipient identifier (private message)
     * @param name the name of image file of this message
     * @param image the image data of this image of this message
     * @param timestamp the creation or received time of this message
     */
    ImageMessage(const QString &sender, const QString &chatroomName, const QString &name, const QImage &image,
                 const QDateTime &timestamp = QDateTime::currentDateTime());

    /**
     * @brief  data convert data to the format required by the network.
     * @return a QByteArray containing a network compatible representation of this ImageMessage.
     */
    QByteArray data() const override;

    /**
     * @brief name retrieves the filename of the image
     * @return the filename of this image
     */
    QString name() const;

    /**
     * @brief image retrieves the image
     * @return the image in QImage format
     */
    QImage image() const;

    /**
     * @brief imageRaw retrieves the image raw data inside a QByteArray
     * @return image raw data inside a QByteArray
     */
    QByteArray imageRaw() const;

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
    const QString _name; /**< the name of this image of this message */
    const QImage _image; /**< the data of this image of this message */
};

#endif // IMAGEMESSAGE_H
