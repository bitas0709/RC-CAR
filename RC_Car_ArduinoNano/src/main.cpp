#include <Arduino.h>
#include <SoftwareSerial.h>
//ENA, IN1, IN2 - рулевое управление
#define ENA 5
#define IN1 6
#define IN2 7
//ENB, IN3, IN4 - управление ускорением
#define IN3 8
#define IN4 9
#define ENB 10

#define RightTurnLED 3
#define LeftTurnLED 11
#define HeadlightsLED A1
#define StopLED 12
#define ReverseLED 13

#define BatteryLevelPin A0 //уровень заряда аккумулятора, полученный с делителя напряжения (10 кОм и 4.7 кОм)
int BatteryLevel;
unsigned long SendDataTimer = 60000; //количество миллисекунд, через которое будут отправлены данные с Adruino на удалённое устройство

SoftwareSerial BTSerial(2,4);

int currentString = 0;
const int maxStringCount = 3; //максимальное количество элементов в полученном массиве
//первая строка - движение вперёд/назад
//вторая строка - поворот налево/направо
String RecievedData[maxStringCount];
int RecievedIntData[maxStringCount];
bool allDataRecieved = 0;
int absAccelerationValue;
int accelerationValue;
int accelerationDirection;
int absSteeringValue;
int steeringValue;
int steeringDirection;

unsigned long SendDataLastTime;
bool SendDataLastTimeFlag;

char incomingChar;

unsigned long EmergencyLEDLastTime;
bool EmergencyLEDLastTimeFlag;
bool EmergencyLEDActive;
bool EmergencyLEDRiseUp = 1;
int EmergencyLEDBrightness = 0;

void motor(int acceleration, int accelerationSpeed, int steering, int steeringSpeed) {
  if (acceleration == 0) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 0);
    digitalWrite(ReverseLED, LOW);
  } else if (acceleration == -1) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, accelerationSpeed);
    digitalWrite(ReverseLED, HIGH);
  } else if (acceleration == 1) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, accelerationSpeed);
    digitalWrite(ReverseLED, LOW);
  }
  if (steering == 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
  } else if (steering == -1) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, steeringSpeed);
  } else if (steering == 1) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, steeringSpeed);
  }
}

void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(LeftTurnLED, OUTPUT);
  pinMode(RightTurnLED, OUTPUT);
  pinMode(StopLED, OUTPUT);
  pinMode(HeadlightsLED, OUTPUT);
  pinMode(ReverseLED, OUTPUT);

  BTSerial.begin(9600);
  Serial.begin(9600);
}

void loop() {
  //Чтение данных, принятых через Bluetooth
  while (BTSerial.available()) {
    incomingChar = (BTSerial.read());
    if (incomingChar == '$') { // $ - переход на ввод следующего значения в массив
      if (currentString < maxStringCount - 1) {
        currentString++;
      } else {
        Serial.println("Less data expected!!!");
        currentString = 0;
      }
    } else if (incomingChar == '~') { // ~ - конец передачи пакета данных
      for (int i = 0; i <= currentString; i++) {
        Serial.print("RecievedData[");
        Serial.print(i);
        Serial.print("] = ");
        Serial.println(RecievedData[i]);
      }
      allDataRecieved = true;
      currentString = 0;
    } else {
      RecievedData[currentString] += incomingChar;
    }
  }
  if (allDataRecieved == true) {
    for(int i = 0; i < maxStringCount; i++) {
      RecievedIntData[i] = RecievedData[i].toInt();
      Serial.print("RecievedIntData[");
      Serial.print(i);
      Serial.print("] = ");
      Serial.println(RecievedIntData[i]);
    }
    if (RecievedIntData[0] > 0) {
      accelerationValue = map(RecievedIntData[0], 1, 100, 1, 255);
      accelerationDirection = 1;
    } else if (RecievedIntData[0] < 0) {
      //accelerationValue = map(RecievedIntData[0], -1, -100, 1, 255);
      absAccelerationValue = abs(RecievedIntData[0]);
      accelerationValue = map(absAccelerationValue, 1, 100, 1, 255);
      accelerationDirection = -1;
    } else if (RecievedIntData[0] == 0) {
      accelerationValue = RecievedIntData[0];
      accelerationDirection = 0;
    }
    if (RecievedIntData[1] > 0) {
      steeringValue = map(RecievedIntData[1], 1, 100, 1, 255);
      steeringDirection = 1;
    } else if (RecievedIntData[1] < 0) {
      //steeringValue = map(RecievedIntData[1], 1, 100, 1, 255);
      absSteeringValue = abs(RecievedIntData[1]);
      steeringValue = map(RecievedIntData[1], 1, 100, 1, 255);
      steeringDirection = -1;
    } else if (RecievedIntData[1] == 0) {
      steeringValue = 0;
      steeringDirection = 0;
    }
    /*Serial.print("accelerationDirection = ");
    Serial.println(accelerationDirection);
    Serial.print("accelerationValue = ");
    Serial.println(accelerationValue);
    Serial.print("steeringDirection = ");
    Serial.println(steeringDirection);
    Serial.print("steeringValue = ");
    Serial.println(steeringValue);*/
    motor(accelerationDirection, accelerationValue, steeringDirection, steeringValue);
    allDataRecieved = false;
    for (int i = 0; i < maxStringCount; i++) {
      RecievedData[i] = "";
    }
  }
  
  /*while (Serial.available()) {
    BTSerial.write(Serial.read());
  }*/
  if (SendDataLastTimeFlag == 0) {
    SendDataLastTime = millis();
    SendDataLastTimeFlag = 1;
  }
  if (SendDataLastTimeFlag == 1 && millis() - SendDataLastTime > SendDataTimer) {
    BatteryLevel = map(analogRead(BatteryLevelPin), 580, 823, 0 , 100);
    Serial.print("BatteryLevel = ");
    Serial.println(BatteryLevel);
    BTSerial.write(BatteryLevel);
    SendDataLastTimeFlag = 0;
  }
  if (RecievedIntData[2] == 1) {
    EmergencyLEDActive = 1;
  } else {
    EmergencyLEDActive = 0;
    digitalWrite(LeftTurnLED, LOW);
    digitalWrite(RightTurnLED, LOW);
    EmergencyLEDBrightness = 0;
  }
  if(EmergencyLEDActive == 1) {
    if(EmergencyLEDLastTimeFlag == 0) {
      EmergencyLEDLastTime = millis();
      EmergencyLEDLastTimeFlag = 1;
    }
    if(EmergencyLEDLastTimeFlag == 1 && millis() - EmergencyLEDLastTime > 5) {
      if (EmergencyLEDRiseUp && EmergencyLEDBrightness < 251) {
        EmergencyLEDBrightness += 5;
        analogWrite(LeftTurnLED, EmergencyLEDBrightness);
        analogWrite(RightTurnLED, EmergencyLEDBrightness);
      } else if (EmergencyLEDRiseUp == 0 && EmergencyLEDBrightness > 4) {
        EmergencyLEDBrightness -= 5;
        analogWrite(LeftTurnLED, EmergencyLEDBrightness);
        analogWrite(RightTurnLED, EmergencyLEDBrightness);
      }
      if (EmergencyLEDBrightness >= 255) {
        EmergencyLEDRiseUp = 0;
      } else if (EmergencyLEDBrightness <= 0) {
        EmergencyLEDRiseUp = 1;
      }
      EmergencyLEDLastTimeFlag = 0;
    }
  }
}