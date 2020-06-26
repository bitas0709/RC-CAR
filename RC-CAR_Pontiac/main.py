import network
import socket
from machine import Pin, PWM
import time

NodeLED = Pin(16, Pin.OUT)
NodeLED.value(0)
time.sleep(1)
NodeLED.value(1)

IN1 = Pin(4, Pin.OUT) #вправо D2
IN2 = Pin(0, Pin.OUT) #влево D3
IN3 = Pin(14, Pin.OUT) #назад D5
IN4 = Pin(12, Pin.OUT) #вперед D6

ENA = PWM(Pin(5)) #рулевое управление D1
ENB = PWM(Pin(13)) #управление ускорением D7

def accelFunc(direction, speed): #управление ускорением
  if (direction == "forward"): #движение вперед
    IN3.off()
    IN4.on()
    ENB.duty(speed)
  elif (direction == "back"): #движение назад
    IN3.on()
    IN4.off()
    ENB.duty(speed)
  elif (direction == "stop"): #остановка
    IN3.off()
    IN4.off()
    ENB.duty(0)

def steerFunc(direction, speed): #рулевое управление
  if (direction == "left"): #поворот налево
    IN1.off()
    IN2.on()
    ENA.duty(speed)
  elif (direction == "right"): #поворот направо
    IN1.on()
    IN2.off()
    ENA.duty(speed)
  elif (direction == "straight"): #движение прямо
    IN1.off()
    IN2.off()
    ENA.duty(0)
    
def map(value, fromMin, fromMax, toMin, toMax): #преобразование числа из одного диапазона в число из другого диапазона
    return int((value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin)

wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect('MikroTik-3063', '714866840')
while not wlan.isconnected():
    pass
host = wlan.ifconfig()[0]

#ap = network.WLAN(network.AP_IF)
#ap.active(True)
#ap.config(essid='RC-CAR')
#ap.config(password='86651370')
#host = ap.ifconfig()[0]

port = 5000
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((host, port))
#sock.settimeout(1)

while True:
    try:
        data, addr = sock.recvfrom(1024)
        strdata = data.decode("utf-8")
        sock.settimeout(1)
        try:
            code, accelVal, steerVal = strdata.split("$") #от 0 до 1023. 511 считается как 0.
            #на Pontiac Solstice основной мотор раскручивается на duty = 350 при clock = 500
            #code в данном случае оставлю на потом
            sock.sendto(bytes("gotIt", "utf-8"), addr)
            code = int(code)
            accelVal = int(accelVal)
            steerVal = int(steerVal)
            #print("code =", code)
            #print("accelVal =", accelVal)
            #print("steerVal =", steerVal)
            if (accelVal == 511):
                accelFunc("stop", 0)
            elif (accelVal > 511 and accelVal <= 1023):
                accelFunc("forward", map(accelVal, 512, 1023, 350, 1023))
            elif (accelVal < 511 and accelVal >= 0):
                accelFunc("back", map(accelVal, 510, 0, 350, 1023))
            if (steerVal == 511): #здесь с минимальными пределами и мертвыми зонами еще надо разобраться
                steerFunc("straight", 0)
            elif (steerVal > 511 and steerVal <= 1023):
                steerFunc("right", map(steerVal, 512, 1023, 0, 1023))
            elif (steerVal < 511 and steerVal >= 0):
                steerFunc("left", map(steerVal, 510, 0, 0, 1023))
        except ValueError:
            try:
                sock.sendto(bytes("valueerror", "utf-8"), addr)
                code, value = strdata.split("$")
                #print("Попытка использования нереализованной функции! Пропуск")
            except ValueError:
                pass
            #print("Something went wrong! Stopping car")
            #accelFunc("stop", 0)
            #steerFunc("straight", 0)
            #sock.sendto(data, addr)
    except OSError:
        #print("timeout")
        sock.sendto(bytes("timeout", "utf-8"), addr)
        accelFunc("stop", 0)
        steerFunc("straight", 0)
        sock.settimeout(None)
