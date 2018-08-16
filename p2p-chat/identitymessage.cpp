#include "identitymessage.h"
#include <QBuffer>

IdentityMessage::IdentityMessage(const QString &sender, const QString &username,
                                 const QString &location, const QString &timezone, const QImage &image,
                                 const QDateTime &timestamp)
        : Message(sender, timestamp),
          _username(QString(username).replace(Message::SeparatorHTMLCode, QString(Message::Separator))),
          _location(QString(location).replace(Message::SeparatorHTMLCode, QString(Message::Separator))),
          _timezone(timezone),
          _image(image) {

}

QByteArray IdentityMessage::data() const {
    QByteArray data = QByteArray::number(Message::IdentityMessage);
    data += Message::Separator;

    data += QString(_username).replace(Message::Separator, Message::SeparatorHTMLCode);
    data += Message::Separator;
    data += QString(_location).replace(Message::Separator, Message::SeparatorHTMLCode);
    data += Message::Separator;
    data += _timezone;
    data += Message::Separator;
    data += imageRaw();

    return data;
}

QString IdentityMessage::username() const {
    return _username;
}

QString IdentityMessage::location() const {
    return _location;
}

QString IdentityMessage::timezone() const {
    return _timezone;
}

QImage IdentityMessage::image() const {
    return _image;
}

QByteArray IdentityMessage::imageRaw() const {
    QByteArray image;
    QBuffer imageBuffer(&image);
    imageBuffer.open(QIODevice::WriteOnly);
    _image.save(&imageBuffer, "PNG");

    return image;
}
