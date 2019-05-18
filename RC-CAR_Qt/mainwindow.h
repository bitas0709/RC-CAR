#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBluetoothLocalDevice>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothSocket>
#include <QDataStream>
#include <QListWidget>
#include <QTimer>
#include <QDebug>
#include <QDialog>
#include <QGyroscopeReading>
#include <QAccelerometer>
#include <QAccelerometerReading>
#include <QSlider>
#include <QKeyEvent>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QBluetoothLocalDevice *localDevice;
    QList<QBluetoothDeviceInfo> listOfDevices;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    QBluetoothSocket *socket;
    QTimer *socketWriteTimer;
    QTimer *exitTimeoutTimer;
    bool exitTimeoutFlag = false;
    QByteArray sendData;
    int accValue;
    QString sendDataStr;
    const char* sendDataChr;
    int AccelerometerValue;
    QString sendAccelerometerStr;
    const char* sendAccelerometerChr;
    QAccelerometer *accelHard = new QAccelerometer();
    QAccelerometerReading *accel = new QAccelerometerReading();
    virtual void keyPressEvent(QKeyEvent *event);
    bool connectToKnownDeviceFlag = false;
    QString selectedDevice;

private slots:
    void on_SearchButton_clicked();

    void on_AccelerationSlider_sliderReleased();

    void startDiscovery();

    void deviceDiscoverFinished();

    void on_CancelButton_clicked();

    void on_DevicesList_itemDoubleClicked(QListWidgetItem *item);

    void socketConnected();

    void socketDisconnected();

    void socketRead();

    void socketWrite();

    void on_SettingsButton_clicked();

    void on_ExitButton_clicked();

    void ExitTimeout();

    void connectToKnownDevice();

    void on_comboBox_activated(int index);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
