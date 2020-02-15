#include "inetsocketclass.h"

InetSocketClass::InetSocketClass(QObject *parent) : QObject(parent)
{
    //hostAddress = QHostAddress("192.168.88.127"); //для локальной сети внутри роутера. В будущем это надо будет
    //переделать под настраиваемый пункт
    m_hostAddress = QHostAddress("192.168.88.127");
    m_port = 5000;
    setAccelVal(511);
    setSteerVal(511);
    socket = new QUdpSocket();
    sendTimer = new QTimer();
    valueChangedTimer = new QTimer();
    connect(sendTimer, SIGNAL(timeout()), SLOT(sendDatagram()));
    connect(valueChangedTimer, SIGNAL(timeout()), SLOT(disableSendingDatagrams()));
#ifdef Q_OS_ANDROID
    accelHard->start();
#endif
}

void InetSocketClass::test()
{
    qDebug() << "hehmda";
}

void InetSocketClass::sendDatagram() {
    QByteArray data;
    QString str;
    str.append("01$");
    str.append(QString::number(accelVal()));
    str.append("$");
    if (isAccelEnabled()) {
        if (accelHard->reading()->y() <= -2 || accelHard->reading()->y() >= 2) {
            str.append(QString::number(map(int(accelHard->reading()->y()), -10, 10, 0, 1023)));
            qDebug() << "accelHard =" << map(int(accelHard->reading()->y()), -10, 10, 0, 1023);
        } else {
            str.append(QString::number(0));
        }
    } else {
        str.append(QString::number(steerVal()));
    }
    data = str.toUtf8();
    socket->writeDatagram(data, data.size(), m_hostAddress, m_port);
}

void InetSocketClass::disableSendingDatagrams() {
    if (accelVal() == 511) {
        sendTimer->stop();
    }
}

void InetSocketClass::valueChanged() {
    if (!sendTimer->isActive()) {
        sendTimer->start(20);
    }
    valueChangedTimer->start(1000);
}

bool InetSocketClass::isAccelEnabled() {
    return accelHard->isActive();
}

int InetSocketClass::map(int value, int inMin, int inMax, int outMin, int outMax) {
    return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}
