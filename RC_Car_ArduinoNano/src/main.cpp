//3,5,6,9,10,11 - PWM порты
#include <Arduino.h>
#include <SoftwareSerial.h>
//ENA, IN1, IN2 - рулевое управление
#define ENA 3
#define IN1 A4
#define IN2 A3
//ENB, IN3, IN4 - управление ускорением
#define IN3 A2
#define IN4 A1
#define ENB 5
#define BluetoothState 6

#define FoglightLED 6
#define HeadlightsLED 7
#define ReverseLED 8
#define StopLED 9
#define RightTurnLED 10
#define LeftTurnLED 11

//#define AccelerationForwardPWM 9
//#define AccelerationBackwardPWM 5

/* 
Далее идёт описание кодов определённых запросов:

AccelSteeringCode - получение информации о ускорении и положении руля одновременно.
  Использование: write(AccelSteeringCode, AccelVal, SteeringVal);
  AccelVal = 0 .. 255, SteeringVal = 0 .. 255
  Нулевое значение равен 127.
AcceleratonCode - получение информации только о ускорении.
  Использование: write(AccelerationCode, AccelVal);
SteeringCode - получение информации только о повороте руля.
  Использование: write(SteeringCode, SteeringVal);
KeepAlive - проверка связи между управляемым устройством и контроллером.
  Если ответ от контроллера не получен, то вызывается функция
  motor() с нулевыми значениями для предотвращения аварии из-за
  потери соединения.
  Время ожидания ответа настраивается вручную.
Все последующие коды, содержащие в имени LED, управляют определёнными светодиодами.
  Использование: write(FoglightLEDCode, value); value =  0 .. 1

Все коды должны совпадать как на контроллере, так и на управляемом устройстве
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

enum {
  TurnLeft, TurnRight, EmergencyLight, StopLight, HeadLight, BackLight, FogLight
};

#define BatteryLevelPin A0 //уровень заряда аккумулятора, полученный с делителя напряжения (10 кОм и 4.7 кОм)
int BatteryLevel;
unsigned long SendDataTimer = 10000; //количество миллисекунд, через которое будут отправлены данные с Adruino на удалённое устройство

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

unsigned long TurnLEDBlinkTime;
bool TurnLEDBlinkFlag;
bool TurnLEDBlinkActive;
bool TurnLEDIsOn;
unsigned long TurnLEDBlinkSpeed = 400;
int TurnLEDBlinkID;

unsigned long LastRecievedPacketTime;
bool LastRecievedPacketTimeFlag;
bool LastRecievedPacketTimeOldFlag = true;

unsigned long keepAliveLastTime;
unsigned long keepAliveWaitingTime = 500;
bool keepAliveLastTimeFlag;
bool keepAliveFlag = true;
bool keepAliveConfirmRecieved;

bool BluetoothStateLastFlag = true;

char incomingChar;

void motor(int acceleration, int accelerationSpeed, int steering, int steeringSpeed) {
  if (acceleration == 0) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 0);
    digitalWrite(ReverseLED, LOW);
  } else if (acceleration == -1) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, accelerationSpeed);
    digitalWrite(ReverseLED, HIGH);
  } else if (acceleration == 1) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, accelerationSpeed);
    digitalWrite(ReverseLED, LOW);
  }
  /*if (acceleration == 0) {
    digitalWrite(AccelerationForwardPWM, LOW);
    digitalWrite(AccelerationBackwardPWM, LOW);
    digitalWrite(ReverseLED, LOW);
  } else if (acceleration == -1) {
    analogWrite(AccelerationBackwardPWM, accelerationSpeed);
    digitalWrite(AccelerationForwardPWM, LOW);
    digitalWrite(ReverseLED, HIGH);
  } else if (acceleration == 1) {
    analogWrite(AccelerationForwardPWM, accelerationSpeed);
    digitalWrite(AccelerationBackwardPWM, LOW);
    digitalWrite(ReverseLED, LOW);
  }*/
  if (steering == 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
  } else if (steering == -1) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, steeringSpeed);
  } else if (steering == 1) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, steeringSpeed);
  }
}

void TurnOnLED(int LEDMode, int enable) { //0 - отключить, 1 - активировать
  switch (LEDMode) {
    case TurnLeft:
      if (enable == 1) {
        digitalWrite(LeftTurnLED, HIGH);
      } else {
        digitalWrite(LeftTurnLED, LOW);
      }
      break;
    case TurnRight:
      if (enable == 1) {
        digitalWrite(RightTurnLED, HIGH);
      } else {
        digitalWrite(RightTurnLED, LOW);
      }
      break;
    case EmergencyLight:
      if (enable == 1) {
        digitalWrite(LeftTurnLED, HIGH);
        digitalWrite(RightTurnLED, HIGH);
      } else {
        digitalWrite(LeftTurnLED, LOW);
        digitalWrite(RightTurnLED, LOW);
      }
      break;
    case HeadLight:
      if (enable == 1) {
        digitalWrite(HeadlightsLED, HIGH);
      } else {
        digitalWrite(HeadlightsLED, LOW);
      }
      break;
    case StopLight:
      //if (enable == 1) {
      //  digitalWrite(StopLED, HIGH);
      //} else {
      //  digitalWrite(StopLED, LOW);
      //}
      break;
    case BackLight:
      if (enable == 1) {
        digitalWrite(ReverseLED, HIGH);
      } else {
        digitalWrite(ReverseLED, LOW);
      }
      break;
    case FogLight:
      if (enable == 1) {
        digitalWrite(FoglightLED, HIGH);
      } else {
        digitalWrite(FoglightLED, LOW);
      }
      break;
  }
}

void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  //pinMode(AccelerationForwardPWM, OUTPUT);
  //pinMode(AccelerationBackwardPWM, OUTPUT);

  pinMode(LeftTurnLED, OUTPUT);
  pinMode(RightTurnLED, OUTPUT);
  pinMode(StopLED, OUTPUT);
  pinMode(HeadlightsLED, OUTPUT);
  pinMode(ReverseLED, OUTPUT);

  pinMode(BluetoothState, INPUT);

  BTSerial.begin(38400);
  Serial.begin(9600);
}

void loop() {
  //Чтение данных, принятых через Bluetooth
  while (BTSerial.available()) {
    incomingChar = (BTSerial.read());
    Serial.println(incomingChar);
    if (incomingChar == '$') { // $ - переход на ввод следующего значения в массив
      if (currentString < maxStringCount - 1) {
        currentString++;
      } else {
        Serial.println("Less data expected!!!");
        currentString = 0;
      }
    } else if (incomingChar == '~') { // ~ - конец передачи пакета данных
      allDataRecieved = true;
      currentString = 0;
    } else {
      RecievedData[currentString] += incomingChar;
    }
  }
  if (allDataRecieved == true) {
    //LastRecievedPacketTime = millis();
    //LastRecievedPacketTimeFlag = true;
    for(int i = 0; i < maxStringCount; i++) {
      RecievedIntData[i] = RecievedData[i].toInt();
      Serial.print("RecievedIntData[");
      Serial.print(i);
      Serial.print("] = ");
      Serial.println(RecievedIntData[i]);
    }
    switch(RecievedIntData[0]) {
      case AccelSteeringCode:
        if (RecievedIntData[1] > 127) {
          accelerationValue = map(RecievedIntData[1], 128, 255, 1, 255);
          accelerationDirection = 1;
        } else if (RecievedIntData[1] < 127) {
          absAccelerationValue = abs(RecievedIntData[1]);
          accelerationValue = map(absAccelerationValue, 0, 126, 255, 1);
          accelerationDirection = -1;
        } else if (RecievedIntData[1] == 127) {
          accelerationValue = 0;
          accelerationDirection = 0;
        }
        if (RecievedIntData[2] > 10 && RecievedIntData[2] < 50) {
          steeringValue = map(RecievedIntData[2], 10, 50, 1, 127);
          steeringDirection = -1;
        } else if (RecievedIntData[2] > 50) {
          steeringValue = 127;
          steeringDirection = -1;
        } else if (RecievedIntData[2] > -10 && RecievedIntData[2] < 10) {
          steeringDirection = 0; 
        } else if (RecievedIntData[2] < -10 && RecievedIntData[2] > -50) {
          absSteeringValue = abs(RecievedIntData[2]);
         steeringValue = map(absSteeringValue, 10, 50, 1, 127);
         steeringDirection = 1;
        } else if (RecievedIntData[2] < -50) {
          steeringValue = 127;
          steeringDirection = 1;
        }
        break;
      case AccelerationCode:
        if (RecievedIntData[1] > 127) {
          accelerationValue = map(RecievedIntData[1], 128, 255, 1, 255);
          accelerationDirection = 1;
        } else if (RecievedIntData[1] < 127) {
          absAccelerationValue = abs(RecievedIntData[1]);
          accelerationValue = map(absAccelerationValue, 0, 126, 255, 1);
          accelerationDirection = -1;
        } else if (RecievedIntData[1] == 127) {
          accelerationValue = 0;
          accelerationDirection = 0;
        }
        break;
      case SteeringCode:
        if (RecievedIntData[1] > 10 && RecievedIntData[1] < 50) {
          steeringValue = map(RecievedIntData[1], 10, 50, 1, 127);
          steeringDirection = -1;
        } else if (RecievedIntData[1] > 50) {
          steeringValue = 127;
          steeringDirection = -1;
        } else if (RecievedIntData[1] > -10 && RecievedIntData[1] < 10) {
          steeringDirection = 0; 
        } else if (RecievedIntData[1] < -10 && RecievedIntData[1] > -50) {
          absSteeringValue = abs(RecievedIntData[1]);
         steeringValue = map(absSteeringValue, 10, 50, 1, 127);
         steeringDirection = 1;
        } else if (RecievedIntData[1] < -50) {
          steeringValue = 127;
          steeringDirection = 1;
        }
        break;
      case KeepAliveCode:
        keepAliveConfirmRecieved = true;
        break;
      case HeadlightsLEDCode:
        if (RecievedIntData[1] == true) {
          TurnOnLED(HeadLight, 1);
        } else if (RecievedIntData[1] == false) {
          TurnOnLED(HeadLight, 0);
        }
        break;
      case FoglightLEDCode:
        if (RecievedIntData[1] == true) {
          TurnOnLED(FogLight, 1);
        } else if (RecievedIntData[1] == false) {
          TurnOnLED(FogLight, 0);
        }
        break;
      case RightTurnLEDCode:
        if (RecievedIntData[1] == true) {
          TurnLEDBlinkActive = 1;
          TurnLEDBlinkID = TurnRight;
          TurnLEDIsOn = 1;
          TurnOnLED(TurnLEDBlinkID, TurnLEDIsOn);
        } else {
          TurnLEDBlinkActive = 0;
          TurnOnLED(TurnRight, 0);
        }
        break;
      /*case LeftTurnLEDCode:
        if (RecievedIntData[1] == true) {
          TurnLEDBlinkActive = 1;
          TurnLEDBlinkID = TurnLeft;
          TurnLEDIsOn = 1;
          TurnOnLED(TurnLEDBlinkID, TurnLEDIsOn);
        } else {
          TurnLEDBlinkActive = 0;
          TurnOnLED(TurnLeft, 0);
        }
        break;*/
    }
    motor(accelerationDirection, accelerationValue, steeringDirection, steeringValue);
    allDataRecieved = false;
    for (int i = 0; i < maxStringCount; i++) {
      RecievedData[i] = "";
    }
  }
  if (SendDataLastTimeFlag == false) {
    SendDataLastTime = millis();
    SendDataLastTimeFlag = true;
  }
  if (SendDataLastTimeFlag == true && millis() - SendDataLastTime > SendDataTimer) {
    BatteryLevel = map(analogRead(BatteryLevelPin), 580, 823, 0 , 100);
    //Serial.print("BatteryLevel = ");
    //Serial.println(BatteryLevel);
    BTSerial.write(BatteryStatusCode);
    BTSerial.write("$");
    BTSerial.write(BatteryLevel);
    BTSerial.write("~");
    SendDataLastTimeFlag = false;
  }
  if (TurnLEDBlinkActive == 1) {
    if (TurnLEDBlinkFlag == 0) {
      TurnLEDBlinkTime = millis();
      TurnLEDBlinkFlag = 1;
    }
    if (TurnLEDBlinkFlag == 1 && millis() - TurnLEDBlinkTime > TurnLEDBlinkSpeed) {
      if (TurnLEDIsOn == false) {
        TurnOnLED(TurnLEDBlinkID, 1);
        TurnLEDIsOn = 1;
      } else {
        TurnOnLED(TurnLEDBlinkID, 0);
        TurnLEDIsOn = 0;
      }
      TurnLEDBlinkFlag = 0;
    }
  }
  if (digitalRead(BluetoothState) == false) {
    if (BluetoothStateLastFlag != digitalRead(BluetoothState)) {
      accelerationDirection = 0;
      accelerationValue = 0;
      steeringDirection = 0;
      steeringValue = 0;
      Serial.println("Disconnected!");
      motor(accelerationDirection, accelerationValue, steeringDirection, steeringValue);
      BluetoothStateLastFlag = false;
    }
  } else if (digitalRead(BluetoothState) == true && BluetoothStateLastFlag == false) {
    Serial.println("Connected!");
    BluetoothStateLastFlag = true;
  }
  if (LastRecievedPacketTimeFlag == true && millis() - LastRecievedPacketTime > 200) {
    /*accelerationDirection = 0;
    accelerationValue = 0;
    steeringDirection = 0;
    steeringValue = 0;
    motor(accelerationDirection, accelerationValue, steeringDirection, steeringValue);*/
    BTSerial.write(KeepAliveCode);
    BTSerial.write("~");
    keepAliveLastTime = millis();
    keepAliveLastTimeFlag = true;
    LastRecievedPacketTimeFlag = false;
    //keepAliveFlag = false;
  }
  if (keepAliveLastTimeFlag == true && millis() - keepAliveLastTime > keepAliveWaitingTime) {
    if (keepAliveConfirmRecieved) {
      keepAliveFlag = true;
    } else {
      accelerationDirection = 0;
      accelerationValue = 0;
      steeringDirection = 0;
      steeringValue = 0;
      motor(accelerationDirection, accelerationValue, steeringDirection, steeringValue);
    }
    keepAliveLastTimeFlag = false;
    keepAliveConfirmRecieved = false;
  }
}