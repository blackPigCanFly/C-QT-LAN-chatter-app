#include "filemessage.h"

FileMessage::FileMessage(const QString &sender,
                         const QString &filename,
                         const QByteArray &file,
                         const QDateTime &timestamp)
        : Message(sender, timestamp),
          _filename(QString(filename).replace(Message::SeparatorHTMLCode, QString(Message::Separator))),
          _file(file) {

}

FileMessage::FileMessage(const QString &sender,
                         const QString &chatroomName,
                         const QString &filename,
                         const QByteArray &file,
                         const QDateTime &timestamp)
        : Message(sender, timestamp),
          _chatroomName(QString(chatroomName).replace(Message::SeparatorHTMLCode, QString(Message::Separator))),
          _filename(QString(filename).replace(Message::SeparatorHTMLCode, QString(Message::Separator))),
          _file(file) {

}

QByteArray FileMessage::data() const {
    QByteArray data = QByteArray::number(Message::FileMessage);
    data += Message::Separator;

    if (isPrivate()) {
        data += QString(_chatroomName).replace(Message::Separator, Message::SeparatorHTMLCode) + "/";
    }

    data += QString(_filename).replace(Message::Separator, Message::SeparatorHTMLCode);
    data += Message::Separator;
    data += _file;

    return data;
}

QString FileMessage::filename() const {
    return _filename;
}

QByteArray FileMessage::file() const {
    return _file;
}

QString FileMessage::chatroomName() const {
    return _chatroomName;
}

bool FileMessage::isPrivate() const {
    return !_chatroomName.isNull();
}
