#include "imagemessage.h"
#include <QBuffer>
#include <QDebug>


ImageMessage::ImageMessage(const QString &sender,
                           const QString &name,
                           const QImage &image,
                           const QDateTime &timestamp)
        : Message(sender, timestamp),
          _name(QString(name).replace(Message::SeparatorHTMLCode, QString(Message::Separator))),
          _image(image) {

}

ImageMessage::ImageMessage(const QString &sender,
                           const QString &chatroomName,
                           const QString &name,
                           const QImage &image,
                           const QDateTime &timestamp)
        : Message(sender, timestamp),
          _chatroomName(QString(chatroomName).replace(Message::SeparatorHTMLCode, QString(Message::Separator))),
          _name(QString(name).replace(Message::SeparatorHTMLCode, QString(Message::Separator))),
          _image(image) {

}

QByteArray ImageMessage::data() const {
    QByteArray data = QByteArray::number(Message::ImageMessage);
    data += Message::Separator;

    if (isPrivate()) {
        data += QString(_chatroomName).replace(Message::Separator, Message::SeparatorHTMLCode) + "/";
    }

    data += QString(_name).replace(Message::Separator, Message::SeparatorHTMLCode);
    data += Message::Separator;

    data += imageRaw();

    return data;
}

QString ImageMessage::name() const {
    return _name;
}

QImage ImageMessage::image() const {
    return _image;
}

QByteArray ImageMessage::imageRaw() const {
    QByteArray image;
    QBuffer imageBuffer(&image);
    imageBuffer.open(QIODevice::WriteOnly);
    _image.save(&imageBuffer, "PNG");

    return image;
}

QString ImageMessage::chatroomName() const {
    return _chatroomName;
}

bool ImageMessage::isPrivate() const {
    return !_chatroomName.isNull();
}
