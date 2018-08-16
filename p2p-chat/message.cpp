#include "message.h"

Message::Message(QString sender, QDateTime timestamp) : _sender(std::move(sender)), _timestamp(std::move(timestamp)) {
}

Message::~Message() = default;

QString Message::sender() const {
    return _sender;
}

bool Message::isEmpty() const {
    return _sender.isEmpty();
}

QDateTime Message::timestamp() const {
    return _timestamp;
}

const QString Message::SeparatorHTMLCode = "&#124;";
