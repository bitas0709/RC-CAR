#ifndef INETSOCKET_H
#define INETSOCKET_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QHostAddress>
#include <QGyroscopeReading>
#include <QAccelerometer>
#include <QAccelerometerReading>
#include <QUdpSocket>
#include <qqml.h>

class InetSocket : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connectedToCar READ connectedToCar WRITE setConnectedToCar NOTIFY connectedToCarChanged)
    Q_PROPERTY(QString RC_CarAddress READ RC_CarAddress WRITE setRC_CarAddress NOTIFY RC_CarAddressChanged)
    Q_PROPERTY(quint16 RC_CarPort READ RC_CarPort WRITE setRC_CarPort NOTIFY RC_CarPortChanged)
    Q_PROPERTY(QString actionCode READ actionCode WRITE setActionCode NOTIFY actionCodeChanged)
    Q_PROPERTY(QString value1 READ value1 WRITE setValue1 NOTIFY value1Changed)
    Q_PROPERTY(QString value2 READ value2 WRITE setValue2 NOTIFY value2Changed)
    QML_ELEMENT
public:
    explicit InetSocket(QObject *parent = nullptr);
    Q_INVOKABLE void establishConnection();
    Q_SIGNAL void rc_carAddressChangedSignal(const QString &text);
    Q_SIGNAL void rc_carConnectionStatusChangedSignal(const bool status);

private:
    QUdpSocket *udpSocket = nullptr;
    QByteArray searchBroadcastMessage = "Looking for RC-CAR";
    QTimer *searchBroadcastMessagePacketTimeout;
    bool searchBroadcastMessageSent = false;
    QByteArray searchIncomingAnswerMessage = "Here it is";
    QByteArray connectingMessage = "Connecting";
    QTimer *connectingMessagePacketTimeout;
    bool connectingMessageSent = false;
    QByteArray connectionEstablishedMessage = "Ready to control";
    QTimer *connectionTimedOut;

    QString m_actionCode; // код действия
    QString m_value1; // значение первого параметра пакета после кода действия
    QString m_value2; // значение второго параметра пакета после кода действия

    QByteArray receivedDatagram;
    QTimer *sendNewPacketTimerDelay;
    QTimer *establishConnectionTimerDelay;
    QHostAddress m_RC_CarAddress = QHostAddress("255.255.255.255");
    quint16 m_RC_CarPort = 52250;
    bool m_connectedToCar = false;
    QString RC_CarAddress() {
        QString tempAddress;
        tempAddress = m_RC_CarAddress.toString().split(":").last();
        qDebug() << tempAddress;
        return tempAddress;
    }
    void setRC_CarAddress(QString value) {
        m_RC_CarAddress = QHostAddress(value);
    }
    quint16 RC_CarPort() {
        return m_RC_CarPort;
    }
    void setRC_CarPort(quint16 value) {
        m_RC_CarPort = value;
    }
    bool connectedToCar() {
        return m_connectedToCar;
    }
    void setConnectedToCar(bool value) {
        m_connectedToCar = value;
    }
    QString actionCode() {
        return m_actionCode;
    }
    void setActionCode(QString value) {
        m_actionCode = value;
    }
    QString value1() {
        return m_value1;
    }
    void setValue1(QString value) {
        m_value1 = value;
    }
    QString value2() {
        return m_value2;
    }
    void setValue2(QString value) {
        m_value2 = value;
    }

    enum CarControl {
        Acceleration, Steering, Moving, Lightning, Suspension
    };

private slots:
    void keepAlive();
    void readPacket();
    void sendPacket(QString actionCode, QString val1, QString val2);
    void searchBroadcastMessagePacketTimeoutSlot();
    void connectingMessagePacketTimeoutSlot();
    void resetConnectionSlot();

signals:
    void connectedToCarChanged();
    void RC_CarAddressChanged();
    void RC_CarPortChanged();
    void actionCodeChanged();
    void value1Changed();
    void value2Changed();
};

#endif // INETSOCKET_H
