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

#include "connection.h"

#include <QtNetwork>

namespace p2pnetworking {

static const int TransferTimeout = 30 * 1000;
static const int PongTimeout = 15 * 1000;
static const int PingInterval = 5 * 1000;
static const int ConnectTimeout = 5 * 1000;
static const char SeparatorToken = Message::Separator;

PeerConnection::PeerConnection(QObject *parent) : QTcpSocket(parent) {
  greetingMessage = tr("undefined");
  username = tr("unknown");
  state = WaitingForGreeting;
  currentDataType = Undefined;
  numBytesForCurrentDataType = -1;
  transferTimerId = 0;
  isGreetingMessageSent = false;
  pingTimer.setInterval(PingInterval);
  transferTimerId = startTimer(ConnectTimeout);

  QObject::connect(this, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
  QObject::connect(this, SIGNAL(disconnected()), &pingTimer, SLOT(stop()));
  QObject::connect(&pingTimer, SIGNAL(timeout()), this, SLOT(sendPing()));
  QObject::connect(this, SIGNAL(connected()), this, SLOT(sendGreetingMessage()));
}

QString PeerConnection::name() const {
  return username;
}

void PeerConnection::setGreetingMessage(const QString &message) {
  greetingMessage = message;
}

bool PeerConnection::sendMessage(QSharedPointer<Message> message) {
  if (message->isEmpty())
    return false;
  QByteArray messageContent = message->data();
  QByteArray data = "MESG|" + QByteArray::number(messageContent.size()) + SeparatorToken + messageContent;
  return write(data) == data.size();
}

void PeerConnection::timerEvent(QTimerEvent *timerEvent) {
  if (timerEvent->timerId() == transferTimerId) {
    abort();
    killTimer(transferTimerId);
    transferTimerId = 0;
  }
}

void PeerConnection::processReadyRead() {
  if (state == WaitingForGreeting) {
    if (!readProtocolHeader())
      return;
    if (currentDataType != Greeting) {
      abort();
      return;
    }
    state = ReadingGreeting;
  }

  if (state == ReadingGreeting) {
    if (!hasEnoughData())
      return;

    buffer = read(numBytesForCurrentDataType);
    if (buffer.size() != numBytesForCurrentDataType) {
      abort();
      return;
    }

    username = QString::fromUtf8(buffer) + '@' + peerAddress().toString();
    currentDataType = Undefined;
    numBytesForCurrentDataType = 0;
    buffer.clear();

    if (!isValid()) {
      abort();
      return;
    }

    if (!isGreetingMessageSent)
      sendGreetingMessage();

    pingTimer.start();
    pongTime.start();
    state = ReadyForUse;
    emit readyForUse();
  }

  do {
    if (currentDataType == Undefined) {
      if (!readProtocolHeader())
        return;
    }
    if (!hasEnoughData())
      return;
    processData();
  } while (bytesAvailable() > 0);
}

void PeerConnection::sendPing() {
  if (pongTime.elapsed() > PongTimeout) {
    abort();
    return;
  }

  write("PING|1|p");
}

void PeerConnection::sendGreetingMessage() {
  QByteArray greeting = greetingMessage.toUtf8();
  QByteArray data = "GREE|" + QByteArray::number(greeting.size()) + '|' + greeting;
  if (write(data) == data.size())
    isGreetingMessageSent = true;
}

int PeerConnection::readDataIntoBuffer(int maxSize) {
  if (maxSize > MaxBufferSize)
    return 0;

  int numBytesBeforeRead = buffer.size();
  if (numBytesBeforeRead == MaxBufferSize) {
    abort();
    return 0;
  }

  while (bytesAvailable() > 0 && buffer.size() < maxSize) {
    buffer.append(read(1));
    if (buffer.endsWith(SeparatorToken))
      break;
  }
  return buffer.size() - numBytesBeforeRead;
}

int PeerConnection::dataLengthForCurrentDataType() {
  if (bytesAvailable() <= 0 || readDataIntoBuffer() <= 0 || !buffer.endsWith(SeparatorToken))
    return 0;

  buffer.chop(1);
  int number = buffer.toInt();
  buffer.clear();
  return number;
}

bool PeerConnection::readProtocolHeader() {
  if (transferTimerId) {
    killTimer(transferTimerId);
    transferTimerId = 0;
  }

  if (readDataIntoBuffer() <= 0) {
    transferTimerId = startTimer(TransferTimeout);
    return false;
  }

  if (buffer == "PING|") {
    currentDataType = Ping;
  } else if (buffer == "PONG|") {
    currentDataType = Pong;
  } else if (buffer == "MESG|") {
    currentDataType = MessageType;
  } else if (buffer == "GREE|") {
    currentDataType = Greeting;
  } else {
    currentDataType = Undefined;
    abort();
    return false;
  }

  buffer.clear();
  numBytesForCurrentDataType = dataLengthForCurrentDataType();
  return true;
}

bool PeerConnection::hasEnoughData() {
  if (transferTimerId) {
    QObject::killTimer(transferTimerId);
    transferTimerId = 0;
  }

  if (numBytesForCurrentDataType <= 0)
    numBytesForCurrentDataType = dataLengthForCurrentDataType();

  if (bytesAvailable() < numBytesForCurrentDataType || numBytesForCurrentDataType <= 0) {
    transferTimerId = startTimer(TransferTimeout);
    return false;
  }

  return true;
}

void PeerConnection::processData() {
  buffer = read(numBytesForCurrentDataType);
  if (buffer.size() != numBytesForCurrentDataType) {
    abort();
    return;
  }

  switch (currentDataType) {
  case MessageType:
    // Split message.
    {
      int pos = buffer.indexOf('|');
      if (pos >= 0) {
        int type = buffer.left(pos).toInt();
        QSharedPointer<Message> message =
            messageFactory.create(username, buffer.mid(pos + 1), type);

        emit newMessage(message);
      }
    }
    break;
  case Ping:
    write("PONG|1|p");
    break;
  case Pong:
    pongTime.restart();
    break;
  default:
    break;
  }

  currentDataType = Undefined;
  numBytesForCurrentDataType = 0;
  buffer.clear();
}

} // namespace p2pnetworking
