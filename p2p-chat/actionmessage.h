#ifndef ACTIONMESSAGE_H
#define ACTIONMESSAGE_H

#include "message.h"

/**
 * @brief ActionMessage represents an action - invite, kick, join and leave
 */

class ActionMessage : public Message {
public:
    enum Action {
        JOIN, LEAVE, INVITE, KICK
    };

    /**
     * @brief ActionMessage Constructor
     * @param sender the sender of this message
     * @param action the action of this message, from ActionMessage::Action
     * @param roomName the first operand, room name
     * @param secondOperand the second operand, may refer to nothing, invited name, invitee or nickname
     * @param thirdOperand the third operand, may refer to nothing or identifier 0
     * @param timestamp the creation or received time of this message
     */
    ActionMessage(const QString &sender,
                  Action action,
                  const QString &roomName,
                  const QString &secondOperand = "",
                  const QString &thirdOperand = "",
                  const QDateTime &timestamp = QDateTime::currentDateTime());

    /**
     * @brief ActionMessage Constructor
     * @param sender the sender of this message
     * @param action the action of this message, in QString format, passing in a invalid action will become KICK
     * @param roomName the first operand, room name
     * @param secondOperand the second operand, may refer to nothing, invited name, invitee or nickname
     * @param timestamp the creation or received time of this message
     */
    ActionMessage(const QString &sender,
                  const QString &action,
                  const QString &roomName,
                  const QString &secondOperand = "",
                  const QString &thirdOperand = "",
                  const QDateTime &timestamp = QDateTime::currentDateTime());

    /**
    * @brief data convert data to the format required by the network.
    * @return a QByteArray containing a network compatible representation of this ActionMessage.
    */
    QByteArray data() const override;

    /**
    * @brief retrieve the Action type
    * @return the action type
    */
    Action action() const;

    /**
    * @brief retrieve the room name
    * @return the room name
    */
    QString roomName() const;

    /**
    * @brief retrieve the second operand, may refer to nothing, invited name, invitee or nickname
    * depends on the action type
    * @return the second operand
    */
    QString secondOperand() const;

    /**
     * @brief retrieve the third operand, may refer to nothing, or identifier 0, refer to
     * https://lo.unisa.edu.au/mod/forum/discuss.php?d=600564 for more details
     * @return the third operand
     */
    QString thirdOperand() const;

private:
    const Action _action; /**< the action name, from Action enum */
    const QString _roomName; /**< the operation on room, the room's name */
    const QString _secondOperand;  /**< the second operand, may refer to nothing, invited name, invitee or nickname */
    const QString _thirdOperand; /**< used to put identifier 0, see https://lo.unisa.edu.au/mod/forum/discuss.php?d=600564 */
};

#endif // ACTIONMESSAGE_H