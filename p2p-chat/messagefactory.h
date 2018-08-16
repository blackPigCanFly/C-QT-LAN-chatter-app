#ifndef MESSAGEFACTORY_H
#define MESSAGEFACTORY_H

#include "message.h"
#include <QByteArray>
#include <QSharedPointer>

/**
 * @brief The MessageFactory class handles the creation of specific message type objects.
 */
class MessageFactory {
public:
    MessageFactory();

    /**
     * @brief create
     * @param sender the sender's identity
     * @param data the data for this message - excluding the type id and the first separator.
     * @param type the type id
     * @return A QSharedPointer to the required Message subclass.
     */
    QSharedPointer<Message> create(const QString &sender, const QByteArray &data, int type) const;

    /**
     * @brief runTest this code allows you to test your create implementation. It includes the samples
     * of each message type.
     */
    static void runTest();

private:
    static QByteArray stripTypeId(const QByteArray &array);
};

#endif // MESSAGEFACTORY_H
