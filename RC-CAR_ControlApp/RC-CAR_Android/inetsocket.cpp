#include "inetsocket.h"

InetSocket::InetSocket(QObject *parent) : QObject(parent)
{
    udpSocket = new QUdpSocket(this);
    connect(udpSocket, &QUdpSocket::readyRead,
            this, &InetSocket::readPacket);
    establishConnectionTimerDelay = new QTimer();
    searchBroadcastMessagePacketTimeout = new QTimer();
    connectingMessagePacketTimeout = new QTimer();
    sendNewPacketTimerDelay = new QTimer();
    connectionTimedOut = new QTimer();
    connect(searchBroadcastMessagePacketTimeout, &QTimer::timeout,
            this, &InetSocket::searchBroadcastMessagePacketTimeoutSlot);
    connect(connectingMessagePacketTimeout, &QTimer::timeout,
            this, &InetSocket::connectingMessagePacketTimeoutSlot);
    connect(connectionTimedOut, &QTimer::timeout,
            this, &InetSocket::resetConnectionSlot);
    if (!connectedToCar()) {
        establishConnectionTimerDelay->start(2000);
    }
    qDebug() << "hehmda";
    //establishConnection();
}

void InetSocket::establishConnection()
{
    if (!searchBroadcastMessageSent) {
        udpSocket->writeDatagram(QByteArray(searchBroadcastMessage), QHostAddress::Broadcast, m_RC_CarPort);
        searchBroadcastMessageSent = true;
    }
    searchBroadcastMessagePacketTimeout->start(50);
}

void InetSocket::keepAlive()
{
    udpSocket->writeDatagram(QByteArray::fromHex("FF"), m_RC_CarAddress, m_RC_CarPort);
}

void InetSocket::readPacket()
{
    connectionTimedOut->start(1000);
    while(udpSocket->hasPendingDatagrams()) {
        receivedDatagram.resize(int(udpSocket->pendingDatagramSize()));
        udpSocket->readDatagram(receivedDatagram.data(), receivedDatagram.size(), &m_RC_CarAddress);
    }
    if (!connectedToCar()) {
        if (searchBroadcastMessageSent &&
                receivedDatagram == QByteArray(searchIncomingAnswerMessage)) {
            udpSocket->writeDatagram(QByteArray(connectingMessage), m_RC_CarAddress, m_RC_CarPort);
            connectingMessageSent = true;
        } else if (connectingMessageSent && receivedDatagram == QByteArray(connectionEstablishedMessage)) {
            m_connectedToCar = true;
            sendNewPacketTimerDelay->start(50);
            Q_EMIT rc_carAddressChangedSignal(m_RC_CarAddress.toString());
            Q_EMIT rc_carConnectionStatusChangedSignal(m_connectedToCar);
            qDebug() << "Car address is " << m_RC_CarAddress;
        }
    } else {
        if (receivedDatagram == QByteArray("OK?")) {
            udpSocket->writeDatagram("OK", m_RC_CarAddress, m_RC_CarPort);
        }
    }
}

void InetSocket::sendPacket(QString actionCode, QString val1, QString val2)
{
    QByteArray packet = QByteArray::fromHex(actionCode.toUtf8() + val1.toUtf8() + val2.toUtf8());
    udpSocket->writeDatagram(packet, m_RC_CarAddress, m_RC_CarPort);
}

void InetSocket::searchBroadcastMessagePacketTimeoutSlot()
{
    searchBroadcastMessageSent = false;
    searchBroadcastMessagePacketTimeout->stop();
}

void InetSocket::connectingMessagePacketTimeoutSlot()
{
    connectingMessageSent = false;
    connectingMessagePacketTimeout->stop();
}

void InetSocket::resetConnectionSlot() {
    connectionTimedOut->stop();
    m_connectedToCar = false;
    sendNewPacketTimerDelay->stop();
    Q_EMIT rc_carAddressChangedSignal(m_RC_CarAddress.toString());
    Q_EMIT rc_carConnectionStatusChangedSignal(m_connectedToCar);
}
