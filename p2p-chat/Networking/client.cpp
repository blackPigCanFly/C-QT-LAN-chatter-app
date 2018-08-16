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

#include <QtNetwork>

#include "client.h"
#include "connection.h"
#include "message.h"
#include "peermanager.h"

namespace p2pnetworking {

Client::Client() {
  peerManager = new PeerManager(this);
  peerManager->setServerPort(server.serverPort());

  QObject::connect(peerManager, SIGNAL(newConnection(PeerConnection *)), this,
                   SLOT(newConnection(PeerConnection *)));
  QObject::connect(&server, SIGNAL(newConnection(PeerConnection *)), this,
                   SLOT(newConnection(PeerConnection *)));
}

QString Client::nickName() const {
  return QString::fromUtf8(peerManager->userName()) + '@' + QHostInfo::localHostName();
}

bool Client::hasConnection(quint32 peer, const QString &nickName) const {
  QList<PeerConnection *> conns = peers.values(peer);
  if (conns.empty()) {
    return false;
  }
  foreach (PeerConnection *con, conns) {
    if (con->name() == nickName) {
      return true;
    }
  }
  return false;
}

void Client::setUserName(const QString &name) {
  peerManager->setUserName(name.toUtf8());
}

void Client::start() {
  server.start();
  peerManager->setServerPort(server.serverPort());
  peerManager->start();
}

void Client::stop() {
  server.stop();
  peerManager->stop();
}

void Client::sendMessage(QSharedPointer<Message> message) {
  if (message.isNull() || message->isEmpty())
    return;

  QList<PeerConnection *> connections = peers.values();
  foreach (PeerConnection *connection, connections) {
    connection->sendMessage(message);
  }
}

void Client::sendMessage(QSharedPointer<Message> message, QString nick) {
  if (message.isNull() || message->isEmpty())
    return;

  QList<PeerConnection *> connections = peers.values();
  foreach (PeerConnection *connection, connections) {
    if (connection->name() == nick) {
      connection->sendMessage(message);
    }
  }
}

void Client::newConnection(PeerConnection *connection) {
  connection->setGreetingMessage(peerManager->userName());

  connect(connection, SIGNAL(error(QAbstractSocket::SocketError)), this,
          SLOT(connectionError(QAbstractSocket::SocketError)));
  connect(connection, SIGNAL(disconnected()), this, SLOT(disconnected()));
  connect(connection, SIGNAL(readyForUse()), this, SLOT(readyForUse()));
}

void Client::readyForUse() {
  PeerConnection *connection = qobject_cast<PeerConnection *>(sender());
  if (!connection || hasConnection(connection->peerAddress().toIPv4Address(), connection->name())) {
    connection->deleteLater();
    return;
  }

  connect(connection, &PeerConnection::newMessage, this, &Client::newMessage);

  peers.insert(connection->peerAddress().toIPv4Address(), connection);
  QString nick = connection->name();
  if (!nick.isEmpty())
    emit newParticipant(nick);
}

void Client::disconnected() {
  if (PeerConnection *connection = qobject_cast<PeerConnection *>(sender()))
    removeConnection(connection);
}

void Client::connectionError(QAbstractSocket::SocketError /* socketError */) {
  if (PeerConnection *connection = qobject_cast<PeerConnection *>(sender()))
    removeConnection(connection);
}

void Client::removeConnection(PeerConnection *connection) {
  QList<PeerConnection *> conns = peers.values(connection->peerAddress().toIPv4Address());
  foreach (PeerConnection *con, conns) {
    if (con->name() == connection->name()) {
      peers.remove(connection->peerAddress().toIPv4Address(), con);
      emit participantLeft(connection->name());
    }
  }
  connection->deleteLater();
}

quint16 Client::serverPort() const {
  return server.serverPort();
}

} // namespace p2pnetworking
