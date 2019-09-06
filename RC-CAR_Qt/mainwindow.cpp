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
                                          "width: 200px"
                                          "} "
                                          "QSlider::handle:vertical { "
                                          "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); "
                                          "border: 1px solid #5c5c5c; "
                                          "margin: 0 -10px; "
                                          "height: 150px"
                                          "} ");
    connect(ui->BeginSearchButton, SIGNAL(clicked()), this, SLOT(startDiscovery()));
    socketWriteTimer = new QTimer();
    exitTimeoutTimer = new QTimer();
    connect(socketWriteTimer, SIGNAL(timeout()), this, SLOT(socketWrite()));
    connect(exitTimeoutTimer, SIGNAL(timeout()), this, SLOT(ExitTimeout()));
    if (settings.value("/Settings/ConnectionType", 0).toInt() == 0) {
        settings.setValue("/Settings/ConnectionType", 0);
        connectToKnownDeviceFlag = false;
    }
    //так как неоновые огни ещё не добавлены в прошивку для машины, кнопка активации неоновых огней будет неактивна
    ui->NeonLightButton->hide();
    //по умолчанию идёт подключение к известному MAC адресу машины
    //ui->comboBox->setCurrentIndex(1);
    ui->comboBox->setCurrentIndex(settings.value("/Settings/ConnectionType", 0).toInt());
    ui->comboBox_2->setCurrentIndex(settings.value("/Settings/AccelerometerAdj", 0).toInt());

#ifdef Q_OS_ANDROID
    accelHard->start();
    ui->SteeringSlider->hide();
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_SearchButton_clicked()
{
    if (connectToKnownDeviceFlag == false) {
        ui->stackedWidget->setCurrentIndex(1);
        ui->DevicesList->clear();
        ui->DevicesList->addItem("Для подключения к устройству дваждны нажмите на его адрес");
    } else if (connectToKnownDeviceFlag == true) {
        connectToKnownDevice();
    }
    /*if (settings.value("/Settings/LastConnectedDevice:", 0) == 0) {

    }*/
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
    selectedDevice = item->text();
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    socket->connectToService(QBluetoothAddress(selectedDevice), QBluetoothUuid(QBluetoothUuid::SerialPort));
    settings.setValue("/Settings/LastConnectedDevice", selectedDevice);
    connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(socketRead()));
}

void MainWindow::socketConnected() {
    lastStackedWidgetIndex = ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(2);
    socketWriteTimer->start(150);
}

void MainWindow::socketDisconnected() {
    lastStackedWidgetIndex = ui->stackedWidget->currentIndex();
    QMessageBox::warning(this, "Отключено", "Соединение с удалённым устройством потеряно");
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::socketRead() {
    recievedData = socket->readAll();
    //QString recievedDataStr = recievedData;
    QString recievedDataStr = QString::fromUtf8(recievedData);
    qDebug() << "recievedDataStr = " << recievedDataStr;
    /*if (formattedData.size() == 2) {
        switch(formattedData[0].toInt()) {
        case 04:
            socket->write("04");
            socket->write("$");
            socket->write("1");
            socket->write("~");
            break;
        case 10:
            ui->BatteryLevelValueLabel->setText(formattedData[1]);
            break;
        }
    }*/
}

void MainWindow::socketWrite() {
#ifdef Q_OS_ANDROID
    switch(ui->comboBox_2->currentIndex()) {
    case 0:
        AccelerometerValue = int(accelHard->reading()->y()*10);
        break;
    case 1:
        AccelerometerValue = int(accelHard->reading()->x()*10*-1);
        break;
    case 2:
        AccelerometerValue = int(accelHard->reading()->x()*10);
        break;
    }
    if (ui->AccelerationSlider->value() != AccelerationLastValue && AccelerometerValue != SteeringLastValue) {
        socket->write("01");
        socket->write("$");
        sendDataStr = QString::number(ui->AccelerationSlider->value());
        sendDataChr = sendDataStr.toStdString().c_str();
        socket->write(sendDataChr);
        socket->write("$");
        sendDataStr = QString::number(AccelerometerValue);
        sendDataChr = sendDataStr.toStdString().c_str();
        socket->write(sendDataChr);
        socket->write("~");
        AccelerationLastValue = ui->AccelerationSlider->value();
        SteeringLastValue = AccelerometerValue;
    } else if (ui->AccelerationSlider->value() != AccelerationLastValue) {
        socket->write("02");
        socket->write("$");
        sendDataStr = QString::number(ui->AccelerationSlider->value());
        sendDataChr = sendDataStr.toStdString().c_str();
        socket->write(sendDataChr);
        socket->write("~");
        AccelerationLastValue = ui->AccelerationSlider->value();
    } else if (AccelerometerValue != SteeringLastValue) {
        socket->write("03");
        socket->write("$");
        sendDataStr = QString::number(AccelerometerValue);
        sendDataChr = sendDataStr.toStdString().c_str();
        socket->write(sendDataChr);
        socket->write("~");
        SteeringLastValue = AccelerometerValue;
    }
#else
    AccelerometerValue = ui->SteeringSlider->value();
    if (ui->AccelerationSlider->value() != AccelerationLastValue && AccelerometerValue != SteeringLastValue) {
        socket->write("01");
        socket->write("$");
        sendDataStr = QString::number(ui->AccelerationSlider->value());
        sendDataChr = sendDataStr.toStdString().c_str();
        socket->write(sendDataChr);
        socket->write("$");
        sendDataStr = QString::number(AccelerometerValue);
        sendDataChr = sendDataStr.toStdString().c_str();
        socket->write(sendDataChr);
        socket->write("~");
        AccelerationLastValue = ui->AccelerationSlider->value();
        SteeringLastValue = AccelerometerValue;
    } else if (ui->AccelerationSlider->value() != AccelerationLastValue) {
        socket->write("02");
        socket->write("$");
        sendDataStr = QString::number(ui->AccelerationSlider->value());
        sendDataChr = sendDataStr.toStdString().c_str();
        socket->write(sendDataChr);
        socket->write("~");
    } else if (AccelerometerValue != SteeringLastValue) {
        socket->write("03");
        socket->write("$");
        sendDataStr = QString::number(AccelerometerValue);
        sendDataChr = sendDataStr.toStdString().c_str();
        socket->write(sendDataChr);
        socket->write("~");
    }
#endif
    if (lastLeftButtonFlag != LeftButtonFlag) {
        socket->write("08");
        socket->write("$");
        if (LeftButtonFlag) {
            socket->write("1");
        } else {
            socket->write("0");
        }
        lastLeftButtonFlag = LeftButtonFlag;
        socket->write("~");
    } else if (lastRightButtonFlag != RightButtonFlag) {
        socket->write("07");
        socket->write("$");
        if (RightButtonFlag) {
            socket->write("1");
        } else {
            socket->write("0");
        }
        lastRightButtonFlag = RightButtonFlag;
        socket->write("~");
    } else if (lastEmergencyButtonFlag != EmergencyButtonFlag) {
        socket->write("09");
        socket->write("$");
        if (EmergencyButtonFlag) {
            socket->write("1");
        } else {
            socket->write("0");
        }
        lastEmergencyButtonFlag = EmergencyButtonFlag;
        socket->write("~");
    } else if (lastHeadLightButtonFlag != HeadLightButtonFlag) {
        socket->write("05");
        socket->write("$");
        if (HeadLightButtonFlag) {
            socket->write("1");
        } else {
            socket->write("0");
        }
        lastHeadLightButtonFlag = HeadLightButtonFlag;
        socket->write("~");
    }
}

void MainWindow::on_SettingsButton_clicked()
{
    lastStackedWidgetIndex = ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(3);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch(event->key()) {
    case Qt::Key_Back:
        if (exitTimeoutFlag == false) {
            if (ui->stackedWidget->currentIndex() == 2) {
                socket->disconnectFromService();
                socketWriteTimer->stop();
            }
            ui->stackedWidget->setCurrentIndex(lastStackedWidgetIndex);
            //ui->stackedWidget->setCurrentIndex(0);
            exitTimeoutTimer->start(1000);
            exitTimeoutFlag = true;
        } else {
            QApplication::quit();
        }
        break;
    case Qt::Key_W:
        ui->AccelerationSlider->setValue(255);
        break;
    case Qt::Key_S:
        ui->AccelerationSlider->setValue(0);
        break;
    case Qt::Key_A:
        ui->SteeringSlider->setValue(-90);
        break;
    case Qt::Key_D:
        ui->SteeringSlider->setValue(90);
        break;
    case Qt::Key_Escape:
        if (exitTimeoutFlag == false) {
            if (ui->stackedWidget->currentIndex() == 2) {
                socket->disconnectFromService();
                socketWriteTimer->stop();
            }
            ui->stackedWidget->setCurrentIndex(0);
            exitTimeoutTimer->start(1000);
            exitTimeoutFlag = true;
        } else {
            QApplication::quit();
        }
        break;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_W:
        ui->AccelerationSlider->setValue(127);
        break;
    case Qt::Key_S:
        ui->AccelerationSlider->setValue(127);
        break;
    case Qt::Key_A:
        ui->SteeringSlider->setValue(0);
        break;
    case Qt::Key_D:
        ui->SteeringSlider->setValue(0);
        break;
    }
}

void MainWindow::on_ExitButton_clicked()
{
    QApplication::quit();
}

void MainWindow::ExitTimeout() {
    exitTimeoutFlag = false;
}

void MainWindow::connectToKnownDevice() {
    //selectedDevice = "98:D3:32:11:1A:D9";
    selectedDevice = settings.value("/Settings/LastConnectedDevice", 0).toString();
    if (selectedDevice == "0") {
        QMessageBox::warning(this, "Ошибка", "Ни одно устройство ранее не было подключено");
    } else {
        socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
        socket->connectToService(QBluetoothAddress(selectedDevice), QBluetoothUuid(QBluetoothUuid::SerialPort));
        connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
        connect(socket, SIGNAL(readyRead()), this, SLOT(socketRead()));
    }
}

void MainWindow::on_comboBox_activated(int index)
{
    if (index == 0) {
        connectToKnownDeviceFlag = false;
    } else if (index == 1) {
        connectToKnownDeviceFlag = true;
    }
    settings.setValue("/Settings/ConnectionType", index);
}

void MainWindow::on_ReturnSettingsButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_SteeringSlider_sliderReleased()
{
    ui->SteeringSlider->setValue(0);
}

void MainWindow::on_comboBox_2_activated(int index)
{
    settings.setValue("/Settings/AccelerometerAdj", index);
}

void MainWindow::on_TurnLeftButton_clicked()
{
    LeftButtonFlag = !LeftButtonFlag;
}

void MainWindow::on_EmergencyLightButton_clicked()
{
    EmergencyButtonFlag = !EmergencyButtonFlag;
}

void MainWindow::on_TurnRightButton_clicked()
{
    RightButtonFlag = !RightButtonFlag;
}

void MainWindow::on_HeadlightButton_clicked()
{
    HeadLightButtonFlag = !HeadLightButtonFlag;
}
