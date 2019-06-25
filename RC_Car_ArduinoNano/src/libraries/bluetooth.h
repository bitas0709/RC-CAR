/* 
    Данная библиотека упрощает передачу пакетных сообщений через bluetooth.
    SoftwareSerial используется для симулирования COM порта с помощью обычных пинов
*/

#ifndef Bluetooth_h

#define Bluetooth_h

#include <Arduino.h>
#include <SoftwareSerial.h>

class Bluetooth {
    public:
    int RX, TX;
    Bluetooth(int RXpin, int TXpin); //получение номеров используемых контактов
    void sendPacket(int PacketType, int value);
    void sendPacket(int PacketType, char value);
    void available();
    void recievePacket(int PacketType, int value);
    void recievePacket(int PacketType, char value);

    enum {
        Acceleration, Steering, SteerAccel, LED
    };
};

#endif