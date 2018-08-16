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
#include "peermanager.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace p2pnetworking {

static const qint32 BroadcastInterval = 2000;
static const unsigned broadcastPort = 45001;
static const QString centralServerUrl = QStringLiteral("http://cppchat.dpwlabs.com/chat");
static const qint32 CentralInterval = 600;

PeerManager::PeerManager(Client *client)
    : QObject(client), client(client), sendCount{}, serverPort{} {
  updateAddresses();
  centralTimer.setSingleShot(true);
  centralTimer.setInterval(CentralInterval);
  connect(&centralTimer, &QTimer::timeout, this, &PeerManager::centralRequest);
  connect(&manager, &QNetworkAccessManager::finished, this, &PeerManager::managerReplyFinished);
}

void PeerManager::setServerPort(int port) {
  serverPort = port;
}

QByteArray PeerManager::userName() const {
  return username;
}

void PeerManager::start() {
  broadcastSocket.bind(QHostAddress::Any, broadcastPort,
                       QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
  connect(&broadcastSocket, SIGNAL(readyRead()), this, SLOT(readBroadcastDatagram()));

  broadcastTimer.setInterval(BroadcastInterval);
  connect(&broadcastTimer, SIGNAL(timeout()), this, SLOT(sendBroadcastDatagram()));
  broadcastTimer.start();
  centralTimer.start();
}

void PeerManager::stop() {
  disconnect(&broadcastTimer, &QTimer::timeout, this, 0);
  disconnect(&broadcastSocket, &QTcpSocket::readyRead, this, 0);
  broadcastTimer.stop();
  centralTimer.stop();
  broadcastSocket.close();
}

bool PeerManager::isLocalHostAddress(const QHostAddress &address) {
  foreach (QHostAddress localAddress, ipAddresses) {
    if (address == localAddress)
      return true;
  }
  return false;
}

void PeerManager::setUserName(const QByteArray &name) {
  username = name;
}

void PeerManager::sendBroadcastDatagram() {
  QByteArray datagram(username);
  datagram.append('|');
  datagram.append(QByteArray::number(serverPort));
  datagram.append('|');
  int len = datagram.length();
  bool validBroadcastAddresses = true;
  auto ipIter = ipAddresses.begin();
  foreach (QHostAddress address, broadcastAddresses) {
    QByteArray ip = ipIter->toString().toUtf8();
    datagram.append(ip);
    if (broadcastSocket.writeDatagram(datagram, address, broadcastPort) == -1)
      validBroadcastAddresses = false;
    if (sendCount % 10 == 0) {
      sendCount = 0;
      QUrl url(centralServerUrl + QStringLiteral("/post"));
      QString query = QStringLiteral("sender=");
      query += QString::fromUtf8(username);
      query += QStringLiteral("&message=");
      query += QString::fromUtf8(datagram);
      query += QStringLiteral("&id=0");
      url.setQuery(query);
      manager.get(QNetworkRequest(url));
    }
    datagram.truncate(len);
    ++ipIter;
  }
  ++sendCount;
  if (!validBroadcastAddresses) {
    updateAddresses();
  }
}

void PeerManager::readBroadcastDatagram() {
  while (broadcastSocket.hasPendingDatagrams()) {
    QHostAddress senderIp;
    quint16 senderPort;
    QByteArray datagram;
    datagram.resize(broadcastSocket.pendingDatagramSize());
    if (broadcastSocket.readDatagram(datagram.data(), datagram.size(), &senderIp, &senderPort)
        == -1)
      continue;
    parseMessage(datagram);
  }
}

void PeerManager::newPeer(QHostAddress address, QString nickName, quint16 port) {
  if (!client->hasConnection(address.toIPv4Address(), nickName)) {
    PeerConnection *connection = new PeerConnection(this);
    emit newConnection(connection);
    connection->connectToHost(address, port);
  }
}

void PeerManager::centralRequest() {
  QUrl url(centralServerUrl + QStringLiteral("/list"));
  manager.get(QNetworkRequest(url));
  centralTimer.start(5000); // crude timeout
}

void PeerManager::parseUserAnnouncements(QNetworkReply *reply) {
  QByteArray httpResponse = reply->readAll();
  if (!httpResponse.isEmpty()) {
    QJsonDocument doc(QJsonDocument::fromJson(httpResponse));
    if (doc.isObject()) {
      QJsonObject obj = doc.object();
    }
    if (doc.isArray()) {
      QJsonArray array = doc.array();
      foreach (QJsonValue val, array) {
        if (val.isObject()) {
          QJsonObject obj = val.toObject();
          parseMessage(obj.value("message").toString().toUtf8());
        }
      }
    }
  }
  centralTimer.start(CentralInterval);
}

void PeerManager::managerReplyFinished(QNetworkReply *reply)
{
  if (reply->error() == QNetworkReply::NoError && reply->url().path().contains("/list")) {
    parseUserAnnouncements(reply);
  }
  reply->deleteLater();
}

void PeerManager::updateAddresses() {
  sendCount = 0;
  broadcastAddresses.clear();
  ipAddresses.clear();
  foreach (QNetworkInterface interface, QNetworkInterface::allInterfaces()) {
    if (interface.flags() & (QNetworkInterface::IsUp | QNetworkInterface::CanBroadcast)
        && !(interface.flags() & QNetworkInterface::IsLoopBack)) {
      foreach (QNetworkAddressEntry entry, interface.addressEntries()) {
        QHostAddress broadcastAddress = entry.broadcast();
        if (broadcastAddress != QHostAddress::Null && entry.ip() != QHostAddress::LocalHost
            && !broadcastAddresses.contains(broadcastAddress)) {
          broadcastAddresses << broadcastAddress;
          ipAddresses << entry.ip();
        }
      }
    }
  }
}

void PeerManager::parseMessage(const QByteArray &message) {
  QList<QByteArray> list = message.split('|');
  if (list.size() != 3)
    return;
  QHostAddress address(QString::fromUtf8(list.at(2)));

  int senderServerPort = list.at(1).toInt();
  if (isLocalHostAddress(address) && senderServerPort == serverPort)
    return;
  QString nickName = QString::fromUtf8(list.at(0)) + '@' + QString::fromUtf8(list.at(2));
  newPeer(address, nickName, senderServerPort);
}

} // namespace p2pnetworking
