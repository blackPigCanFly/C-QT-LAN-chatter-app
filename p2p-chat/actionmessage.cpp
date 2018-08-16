#include "actionmessage.h"

ActionMessage::ActionMessage(const QString &sender, ActionMessage::Action action, const QString &roomName,
                             const QString &secondOperand, const QString &thirdOperand, const QDateTime &timestamp)
        : Message(sender, timestamp),
          _action(action),
          _roomName(QString(roomName).replace(Message::SeparatorHTMLCode, QString(Message::Separator))),
          _secondOperand(QString(secondOperand).replace(Message::SeparatorHTMLCode, QString(Message::Separator))),
          _thirdOperand(QString(thirdOperand).replace(Message::SeparatorHTMLCode, QString(Message::Separator))) {

}

ActionMessage::ActionMessage(const QString &sender, const QString &action, const QString &roomName,
                             const QString &secondOperand, const QString &thirdOperand, const QDateTime &timestamp)
        : Message(sender, timestamp),
          _action(action == "JOIN" ? JOIN
                                   : (action == "LEAVE" ? LEAVE
                                                        : (action == "INVITE" ? INVITE
                                                                              : KICK))),
          _roomName(QString(roomName).replace(Message::SeparatorHTMLCode, QString(Message::Separator))),
          _secondOperand(QString(secondOperand).replace(Message::SeparatorHTMLCode, QString(Message::Separator))),
          _thirdOperand(QString(thirdOperand).replace(Message::SeparatorHTMLCode, QString(Message::Separator))) {
}

QByteArray ActionMessage::data() const {
    QByteArray data = QByteArray::number(Message::ActionMessage);
    data += Message::Separator;

    switch (_action) {
        case JOIN:
            data += "JOIN";
            break;
        case LEAVE:
            data += "LEAVE";
            break;
        case INVITE:
            data += "INVITE";
            break;
        case KICK:
            data += "KICK";
            break;
    }
    data += Message::Separator;

    data += QString(_roomName).replace(Message::Separator, Message::SeparatorHTMLCode);

    if (!_secondOperand.isEmpty()) {
        data += Message::Separator;
        data += QString(_secondOperand).replace(Message::Separator, Message::SeparatorHTMLCode);
    }

    if (!_thirdOperand.isEmpty()) {
        if (_secondOperand.isEmpty()) {
            // if no secondOperand but thirdOperand (should not happen, just in case), add an extra separate
            data += Message::Separator;
        }
        data += Message::Separator;
        data += QString(_thirdOperand).replace(Message::Separator, Message::SeparatorHTMLCode);
    }

    return data;
}

ActionMessage::Action ActionMessage::action() const {
    return _action;
}

QString ActionMessage::roomName() const {
    return _roomName;
}

QString ActionMessage::secondOperand() const {
    return _secondOperand;
}

QString ActionMessage::thirdOperand() const {
    return _thirdOperand;
}



