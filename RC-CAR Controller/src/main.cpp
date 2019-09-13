/*
    Данный код предназначен для перчатки, на которой расположен MPU6050.
    Перчатка может управлять только ускорением и поворотом колёс.
    Дополнительные функции по типу включения различных светодиодных огней
    есть в приложении на Qt, оптимизированном под PC и Android версией выше 4.1
*/

#define AccelSteeringCode 01
#define AccelerationCode 02
#define SteeringCode 03
#define KeepAliveCode 04
#define HeadlightsLEDCode 05
#define FoglightLEDCode 06
#define RightTurnLEDCode 07
#define LeftTurnLEDCode 08
#define EmergencyLightLEDCode 09
#define BatteryStatusCode 10

#include <Arduino.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include <SoftwareSerial.h>

#define TO_DEG 57.29577951308232087679815481410517033f
#define T_OUT 20 // каждый 20 миллисекунд будем проводить вычисления 
#define P_OUT 50 // каждый 50 миллисекунд будем выводить данные
#define FK 0.1 // коэффициент комплементарного фильтра

#define BTKEY 4
#define BTSTATE 7

MPU6050 accelgyro;
SoftwareSerial BTSerial(3,2);

float angle_ax, angle_ay, angle_az;
int mapped_ax, mapped_ay;
int dt = 0;
long int t_next, p_next;

bool bluetoothState;

// функция, которая не даёт значению выйти за пределы 
float clamp(float v, float minv, float maxv){
    if( v>maxv )
        return maxv;
    else if( v<minv )
        return minv;
    return v;
}

void setup() {
    Serial.begin(9600);
    // инициализация MPU6050
    accelgyro.initialize();
    pinMode(BTKEY, OUTPUT);
    pinMode(BTSTATE, INPUT);
    BTSerial.begin(38400);
    /*delay(300);
    digitalWrite(BTKEY, HIGH);
    delay(1000);
    BTSerial.begin(38400);
    delay(100);
    BTSerial.println("AT+INIT");
    delay(250);
    digitalWrite(BTKEY, LOW);
    BTSerial.println("AT+BIND=0018,91,D72DC5");
    BTSerial.println("AT+UART=38400,0,0");
    delay(100);
    BTSerial.println("AT+LINK=0018,91,D72DC5");
    delay(500);*/
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
}

void loop() {
    while (BTSerial.available()) {
        Serial.print(BTSerial.read());
    }
    long int t = millis();
    // каждые T_TO миллисекунд выполняем рассчет угла наклона
    if( t_next < t ){
        int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
        float ay, ax;

        t_next = t + T_OUT;
        // получаем сырые данные от датчиков в MPU6050
        accelgyro.getMotion6(&ax_raw, &ay_raw, &az_raw, &gx_raw, &gy_raw, &gz_raw);
        ay = ay_raw / 16384.0;
        ay = clamp(ay, -1.0, 1.0);
        angle_ay = 90 - TO_DEG*acos(ay);
        ax = ax_raw / 16384.0;
        ax = clamp(ax, -1.0, 1.0);
        angle_ax = 90 - TO_DEG*acos(ax);
    }

    t = millis();
    // каждые P_OUT миллисекунд выводим результат в COM порт
    if( p_next < t ){
        p_next = t + P_OUT;
        if (int(angle_ax) >= -45 && int(angle_ax) <= 45) {
            mapped_ax = map(int(angle_ax), -45, 45, 0, 255);
        } else if (int(angle_ax) < -45) {
            mapped_ax = 0;
        } else if (int(angle_ax) > 45) {
            mapped_ax = 255;
        }
        if (int(angle_ay) >= -20 && int(angle_ay) < 0) {
            mapped_ay = map(int(angle_ay), -20, -1, -90, -1);
        } else if (int(angle_ay) < -20) {
            mapped_ay = -90;
        } else {
            mapped_ay = angle_ay;
        }
        Serial.print(mapped_ax);
        Serial.print("/");
        Serial.println(-mapped_ay);
        bluetoothState = digitalRead(BTSTATE);
            
        if (bluetoothState == true) {
            BTSerial.print(AccelSteeringCode);
            BTSerial.print("$");
            BTSerial.print(int(mapped_ax));
            BTSerial.print("$");
            BTSerial.print(int(-mapped_ay));
            BTSerial.print("~");
        }
        delay(100);
    }
}