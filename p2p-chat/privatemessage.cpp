#include "privatemessage.h"


PrivateMessage::PrivateMessage(const QString &sender, const QString &receiver, const QString &message,
                               const QDateTime &timestamp)
        : Message(sender, timestamp),
          _receiver(QString(receiver).replace(Message::SeparatorHTMLCode, QString(Message::Separator))),
          _message(message) {
}

QByteArray PrivateMessage::data() const {
    QByteArray data = QByteArray::number(Message::PrivateMessage);
    data += Message::Separator;

    data += QString(_receiver).replace(Message::Separator, Message::SeparatorHTMLCode);
    data += Message::Separator;
    data += _message.toUtf8();

    return data;
}

QString PrivateMessage::receiver() const {
    return _receiver;
}

QString PrivateMessage::message() const {
    return _message;
}
