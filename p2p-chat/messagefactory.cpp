#include "messagefactory.h"
#include "textmessage.h"
#include "identitymessage.h"
#include "actionmessage.h"
#include "filemessage.h"
#include "imagemessage.h"
#include "privatemessage.h"

#include <QDebug>
#include <QFile>

MessageFactory::MessageFactory() = default;

QSharedPointer<Message> MessageFactory::create(const QString &sender, const QByteArray &data,
                                               const int type) const {
    switch (type) {
        case Message::TextMessage: {
            // As with std::shared_ptr, a QSharedPointer to a base class can hold a derived class pointer.
            return QSharedPointer<Message>(new TextMessage(sender, QString::fromUtf8(data)));
        }
        case Message::IdentityMessage: {
            QByteArray dataList[4];

            int index1 = data.indexOf(Message::Separator);
            int index2 = data.indexOf(Message::Separator, index1 + 1);
            int index3 = data.indexOf(Message::Separator, index2 + 1);

            dataList[0] = data.left(index1);
            dataList[1] = data.mid(index1 + 1, index2 - index1 - 1);
            dataList[2] = data.mid(index2 + 1, index3 - index2 - 1);
            dataList[3] = data.right(data.length() - index3 - 1);

            return QSharedPointer<Message>(
                    new IdentityMessage(sender, dataList[0], dataList[1], dataList[2], QImage::fromData(dataList[3])));
        }
        case Message::ActionMessage: {
            auto dataList = data.split(Message::Separator);
            if (dataList.length() == 2) {
                // does not have a second parameter
                return QSharedPointer<Message>(new ActionMessage(sender, dataList[0], dataList[1]));
            } else if (dataList.length() == 3) {
                // have a second parameter
                return QSharedPointer<Message>(new ActionMessage(sender, dataList[0], dataList[1], dataList[2]));
            } else {
                // have a third parameter
                return QSharedPointer<Message>(
                        new ActionMessage(sender, dataList[0], dataList[1], dataList[2], dataList[3]));
            }
        }
        case Message::FileMessage: {
            int separatorPosition = data.indexOf(Message::Separator);
            QString filename = data.left(separatorPosition);
            QByteArray fileData = data.right(data.length() - separatorPosition - 1);

            // process private message
            const int index = filename.indexOf('/');
            if (index != -1) {
                // private message
                return QSharedPointer<Message>(
                        new FileMessage(sender, filename.left(index), filename.right(filename.length() - index - 1),
                                        fileData));
            } else {
                return QSharedPointer<Message>(new FileMessage(sender, filename, fileData));

            }
        }
        case Message::ImageMessage: {
            int separatorPosition = data.indexOf(Message::Separator);
            QString name = data.left(separatorPosition);
            QByteArray imageData = data.right(data.length() - separatorPosition - 1);

            // process private message
            const int index = name.indexOf('/');
            if (index != -1) {
                // private message
                return QSharedPointer<Message>(
                        new ImageMessage(sender, name.left(index), name.right(name.length() - index - 1),
                                         QImage::fromData(imageData)));
            } else {
                return QSharedPointer<Message>(new ImageMessage(sender, name, QImage::fromData(imageData)));
            }
        }
        case Message::PrivateMessage: {
            int separatorPosition = data.indexOf(Message::Separator);
            QString receiver = data.left(separatorPosition);
            QByteArray message = data.right(data.length() - separatorPosition - 1);

            return QSharedPointer<Message>(new PrivateMessage(sender, receiver, QString::fromUtf8(message)));
        }
        default:
            return nullptr;
    }
}

void MessageFactory::runTest() {
    QByteArray imageData;
    {
        // This reads a file from the resources.
        // See Qt documentation on The Qt Resource System.
        QFile imageFile(":/test/unisa-48px.png");
        if (imageFile.open(QIODevice::ReadOnly)) {
            imageData = imageFile.readAll();
            imageFile.close();
        }
    }
    QByteArray fileData;
    {
        // This reads a file from the resources.
        // See Qt documentation on The Qt Resource System.
        QFile file(":/test/Helpful Links.docx");
        if (file.open(QIODevice::ReadOnly)) {
            fileData = file.readAll();
            file.close();
        }
    }

    // Example data for each message type note you should always use the defined constants, not hard
    // coded strings for generating these (see TextMessage).
    QByteArray textMessageData = QByteArrayLiteral("1|Hello World!");
    QByteArray identityMessageData = QByteArrayLiteral("0|David|Adelaide|+930|") + imageData;
    QByteArray imageMessageData = QByteArrayLiteral("4|unisa.png|") + imageData;
    QByteArray fileMessageData = QByteArrayLiteral("3|Helpful Links.docx|") + fileData;
    QByteArray actionMessageData = QByteArrayLiteral("2|JOIN|David@10.1.1.10");
    QByteArray privateMessageData = QByteArrayLiteral("5|David@10.1.1.10|Hi, David!");

    MessageFactory factory;

    QByteArray data = stripTypeId(textMessageData);
    QSharedPointer<Message> textMessage = factory.create("none", data, Message::TextMessage);
    data = stripTypeId(identityMessageData);
    QSharedPointer<Message> identityMessage = factory.create("none", data, Message::IdentityMessage);
    data = stripTypeId(imageMessageData);
    QSharedPointer<Message> imageMessage = factory.create("none", data, Message::ImageMessage);
    data = stripTypeId(fileMessageData);
    QSharedPointer<Message> fileMessage = factory.create("none", data, Message::FileMessage);
    data = stripTypeId(actionMessageData);
    QSharedPointer<Message> actionMessage = factory.create("none", data, Message::ActionMessage);
    data = stripTypeId(privateMessageData);
    QSharedPointer<Message> privateMessage = factory.create("none", data, Message::PrivateMessage);

    // Without a complete implementation of message types, only test can be to see if a valid pointer
    // was created by MessageFactory::create();
    if (textMessage == nullptr) {
        qDebug() << "TextMessage is not implemented in MessageFactory::create().";
    }
    if (identityMessage == nullptr) {
        qDebug() << "IdentityMessage is not implemented in MessageFactory::create().";
    }
    if (imageMessage == nullptr) {
        qDebug() << "ImageMessage is not implemented in MessageFactory::create().";
    }
    if (fileMessage == nullptr) {
        qDebug() << "FileMessage is not implemented in MessageFactory::create().";
    }
    if (actionMessage == nullptr) {
        qDebug() << "ActionMessage is not implemented in MessageFactory::create().";
    }
    if (privateMessage == nullptr) {
        qDebug() << "PrivateMessage is not implemented in MessageFactory::create().";
    }
}

QByteArray MessageFactory::stripTypeId(const QByteArray &array) {
    int pos = array.indexOf('|');
    return array.mid(pos + 1);
}
