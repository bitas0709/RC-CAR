#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    ui->AccelerationSlider->setStyleSheet("QSlider::groove:vertical { "
                                          "border: 1px solid #999999; "
                                          "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4); "
                                          "margin: 2px 0; "
                                          "} "
                                          "QSlider::handle:vertical { "
                                          "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); "
                                          "border: 1px solid #5c5c5c; "
                                          "margin: 0 -10px; "
                                          "height: 50px"
                                          "} ");
    connect(ui->BeginSearchButton, SIGNAL(clicked()), this, SLOT(startDiscovery()));
    socketWriteTimer = new QTimer();
    connect(socketWriteTimer, SIGNAL(timeout()), this, SLOT(socketWrite()));
    accelHard->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_SearchButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_ExitButton_clicked()
{

}

void MainWindow::on_AccelerationSlider_sliderReleased()
{
    ui->AccelerationSlider->setValue(127);
}

void MainWindow::startDiscovery() {
    localDevice = new QBluetoothLocalDevice(this);
    if (localDevice->isValid()) {
        qDebug() << "Bluetooth is available on this device";
        localDevice->powerOn();
        discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
        connect(discoveryAgent, SIGNAL(finished()), this, SLOT(deviceDiscoverFinished()));
        discoveryAgent->start();
        ui->DevicesList->clear();
        ui->DevicesList->addItem("Поиск...");
    } else {
        qDebug() << "Bluetooth is not available on this device";
        ui->DevicesList->addItem("Bluetooth на данном устройстве недоступен");
    }
}

void MainWindow::deviceDiscoverFinished() {
    listOfDevices = discoveryAgent->discoveredDevices();
    ui->DevicesList->clear();
    if (listOfDevices.isEmpty()) {
        qDebug() << "No devices found";
        ui->DevicesList->addItem("Устройства не найдены");
    } else {
        for (int i = 0; i < listOfDevices.size(); i++) {
            ui->DevicesList->addItem(listOfDevices.at(i).address().toString().trimmed());
        }
    }

}

void MainWindow::on_CancelButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_DevicesList_itemDoubleClicked(QListWidgetItem *item)
{
    QString selectedDevice = item->text();
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    socket->connectToService(QBluetoothAddress(selectedDevice), QBluetoothUuid(QBluetoothUuid::SerialPort));
    connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(socketRead()));
}

void MainWindow::socketConnected() {
    ui->stackedWidget->setCurrentIndex(2);
    //connect(socketWriteTimer, SIGNAL(timeout()), this, SLOT(socketWrite()));
    //socketWriteTimer->start(10);
    socketWriteTimer->start(150);
}

void MainWindow::socketRead() {

}

void MainWindow::socketDisconnected() {

}

void MainWindow::socketWrite() {
    //qDebug() << "hehmda, it's time to send data";
    //sendDataStr = char(ui->AccelerationSlider->value());
    sendDataStr = QString::number(ui->AccelerationSlider->value());
    sendDataChr = sendDataStr.toStdString().c_str();
    socket->write(sendDataChr);
    //socket->write("~");
    socket->write("$");
    sendAccelerometerStr = QString::number(int(accelHard->reading()->y()*10));
    sendAccelerometerChr = sendAccelerometerStr.toStdString().c_str();
    qDebug() << "accel = " << int(accelHard->reading()->y()*10);
    socket->write(sendAccelerometerChr);
    socket->write("~");
}
