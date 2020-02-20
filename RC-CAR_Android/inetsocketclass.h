#ifndef INETSOCKETCLASS_H
#define INETSOCKETCLASS_H

#include <QObject>
#include <QAbstractSocket>
#include <QDebug>
#include <QHostAddress>
#include <QUdpSocket>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QGyroscopeReading>
#include <QAccelerometer>
#include <QAccelerometerReading>
#include <QTimer>

class InetSocketClass : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString hostAddress READ hostAddress WRITE setHostAddress NOTIFY hostAddressChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(int accelVal READ accelVal WRITE setAccelVal NOTIFY accelValChanged)
    Q_PROPERTY(int steerVal READ steerVal WRITE setSteerVal NOTIFY steerValChanged)
public:
    explicit InetSocketClass(QObject *parent = nullptr);

    Q_INVOKABLE void test();

    Q_INVOKABLE void valueChanged();
    Q_INVOKABLE bool isAccelEnabled();

    int map(int value, int inMin, int inMax, int outMin, int outMax);

    QString hostAddress() {
        return m_hostAddress.toString();
    }

    void setHostAddress(const QString &a) {
        m_hostAddress = QHostAddress(a);
    }

    int port() {
        return m_port;
    }

    void setPort(const int &a) {
        m_port = quint16(a);
    }

    int accelVal() {
        return m_accelVal;
    }

    void setAccelVal(const int &a) {
        m_accelVal = a;
    }

    int steerVal() {
        return m_steerVal;
    }

    void setSteerVal(const int &a) {
        m_steerVal = a;
    }

signals:
    void hostAddressChanged();
    void portChanged();
    void accelValChanged();
    void steerValChanged();
public slots:

private slots:
    void disableSendingDatagrams();
    void sendDatagram();
private:
    QUdpSocket* socket;
    QAccelerometer *accelHard = new QAccelerometer();
    QAccelerometerReading* accel = new QAccelerometerReading();
    QHostAddress m_hostAddress;
    quint16 m_port;
    int m_accelVal = 511;
    int m_steerVal = 511;

    QTimer *sendTimer; //при таймауте отправляет датаграмму с данными
    QTimer *valueChangedTimer; //при таймауте отключает передачу датаграмм

};

#endif // INETSOCKETCLASS_H
