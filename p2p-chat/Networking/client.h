/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CLIENT_H
#define CLIENT_H

#include "message.h"
#include "server.h"
#include <QAbstractSocket>
#include <QHash>
#include <QHostAddress>
#include <QSharedData>

namespace p2pnetworking {

class PeerManager;

/**
 * @brief The Client class a complete peer to peer chat client. IPv4 only.
 */
class Client : public QObject {
  Q_OBJECT

public:
  Client();

  /**
   * @brief nickName retrieve the local nickname
   * @return the local user's nickname
   */
  QString nickName() const;
  /**
   * @brief hasConnection check if a peer is already connected.
   * @param peer the IPv4 address of the peer.
   * @param nickName the nickname of the user on the peer.
   * @return true if a connection from the user on the given peer exists.
   */
  bool hasConnection(quint32 peer, const QString &nickName) const;
  /**
   * @brief setUserName set the user's alias/name/handle - does not include the host IP.
   * @param name any name.
   */
  void setUserName(const QString &name);
  quint16 serverPort() const;

signals:
  /**
   * @brief newMessage emitted when a message is received from the network
   * @param message the message.
   */
  void newMessage(QSharedPointer<Message> message);
  /**
   * @brief newParticipant emitted when a user connects
   * @param nick the unique identifier for the user.
   */
  void newParticipant(const QString &nick);
  /**
   * @brief participantLeft emitted when a user disconnects.
   * @param nick the unique identifier for the user.
   */
  void participantLeft(const QString &nick);

public slots:
  /**
   * @brief start begin the peer to peer client network processing.
   */
  void start();
  /**
   * @brief stop stops all network processing and disconnects.
   */
  void stop();
  /**
   * @brief sendMessage send a message to all peers.
   * @param message the message to send.
   */
  void sendMessage(QSharedPointer<Message> message);
  /**
   * @brief sendMessage send a message to only one recipient
   * @param message the message to send
   * @param nick the unique identifer of the user to send to.
   */
  void sendMessage(QSharedPointer<Message> message, QString nick);

  /* ALL IMPLEMENTATION BELOW THIS POINT IS PRIVATE TO THE NETWORK AND MAY CHANGE AT ANY TIME! */

private slots:
  void newConnection(PeerConnection *connection);
  void connectionError(QAbstractSocket::SocketError socketError);
  void disconnected();
  void readyForUse();

private:
  void removeConnection(PeerConnection *connection);

  PeerManager *peerManager;
  Server server;
  QMultiHash<quint32, PeerConnection *> peers;
};

} // namespace p2pnetworking
#endif
