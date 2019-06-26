#include <Arduino.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include <SoftwareSerial.h>

#define TO_DEG 57.29577951308232087679815481410517033f
#define T_OUT 20 // каждый 20 миллисекунд будем проводить вычисления 
#define P_OUT 50 // каждый 50 миллисекунд будем выводить данные
#define FK 0.1 // коэффициент комплементарного фильтра

#define BTKEY 4

MPU6050 accelgyro;
SoftwareSerial BTSerial(3,2);

float angle_ax, angle_ay, angle_az;
int mapped_ax, mapped_ay;
int dt = 0;
long int t_next, p_next;

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
    delay(300);
    digitalWrite(BTKEY, HIGH);
    delay(1000);
    BTSerial.begin(38400);
    delay(100);
    BTSerial.println("AT+INIT");
    delay(250);
    digitalWrite(BTKEY, LOW);
    //BTSerial.println("AT+BIND=0018,91,D72DC5");
    //BTSerial.println("AT+UART=38400,0,0");
    delay(100);
    BTSerial.println("AT+LINK=0018,91,D72DC5");
    delay(500);
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
        angle_ax = 90 - TO_DEG*acos(ay);
        ax = ax_raw / 16384.0;
        ax = clamp(ax, -1.0, 1.0);
        angle_ay = 90 - TO_DEG*acos(ax);
    }

    t = millis();
    // каждые P_OUT миллисекунд выводим результат в COM порт
    if( p_next < t ){
        p_next = t + P_OUT;
        mapped_ax = map(int(angle_ax), -90, 90, 0, 255);
        Serial.print(mapped_ax);
        Serial.print("/");
        Serial.println(angle_ay);
        //BTSerial.write("123");
        BTSerial.print(int(mapped_ax));
        BTSerial.print("$");
        BTSerial.print(int(angle_ay));
        BTSerial.print("~");
        delay(150);
    }
}