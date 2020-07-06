import network
import socket
from machine import Pin, PWM
import time
import motor

NodeLED = Pin(16, Pin.OUT)
NodeLED.value(0)
time.sleep(1)
NodeLED.value(1)

IN1 = Pin(4, Pin.OUT)  # вправо D2
IN2 = Pin(0, Pin.OUT)  # влево D3
IN3 = Pin(14, Pin.OUT)  # назад D5
IN4 = Pin(12, Pin.OUT)  # вперед D6

ENA = PWM(Pin(5))  # рулевое управление D1
ENB = PWM(Pin(13))  # управление ускорением D7

# словарь, содержащий настройки, считанные из файла settings.txt
settings = {}

def openSettingsFile():
    with open("settings.txt") as file:
        for line in file:
            key, *value = line.split()
            settings[key] = value
        file.close()

def createAPNetwork():
    wlan = network.WLAN(network.AP_IF)
    wlan.active(True)
    wlan.config(essid=settings['intWi-FiSSID'])
    wlan.config(password=settings['intWi-FiPass'])
    
def connectToStation():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(settings['extWi-FiSSID'], settings['extWi-FiPass'])
    for i in range(1, 3):
        if not wlan.isconnected():
            time.sleep(1)
    wlan.active(False)
    return wlan.isconnected()

try:  # попытка открыть файл
    openSettingsFile()
# инициализация базовых настроек, если файл не существует
except OSError:
    file = open("settings.txt", 'x')
    file.write("intWi-FiSSID RC-CAR\n")
    file.write("intWi-FiPass 1234568790\n")
    file.write("extWi-Fi inactive\n")
    file.write("extWi-FiSSID NULL\n")
    file.write("extWi-FiPass NULL\n")
    file.write("port 5000\n")
    file.close()
    openSettingsFile()

if (settings['extWi-Fi'] == "inactive") or (settings['extWi-FiSSID'] == "NULL") or (settings['extWi-FiPass'] == "NULL"):
    createAPNetwork()
else:
    if not connectToStation():
        createAPNetwork()

host = wlan.ifconfig()[0]
port = settings[port]

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((host, port))

while True:
    try:
        data, addr = sock.recvfrom(1024)
        strdata = data.decode("utf-8")
        sock.settimeout(1)
        try:
            code, accelVal, steerVal = strdata.split("$")  # от 0 до 1023. 511 считается как 0.
            # на Pontiac Solstice основной мотор раскручивается на duty = 350 при clock = 500
            # code в данном случае оставлю на потом
            sock.sendto(bytes("gotIt", "utf-8"), addr)
            code = int(code)
            accelVal = int(accelVal)
            steerVal = int(steerVal)
            # print("code =", code)
            # print("accelVal =", accelVal)
            # print("steerVal =", steerVal)
            if (accelVal == 511):
                motor.moveStop()
            elif (accelVal > 511 and accelVal <= 1023):
                motor.moveForward(accelVal)
            elif (accelVal < 511 and accelVal >= 0):
                motor.moveBack(accelVal)
            if (steerVal == 511):  # здесь с минимальными пределами и мертвыми зонами еще надо разобраться
                motor.steerStop()
            elif (steerVal > 511 and steerVal <= 1023):
                motor.steerRight(steerVal)
            elif (steerVal < 511 and steerVal >= 0):
                motor.steerLeft(steerVal)
        except ValueError:
            try:
                sock.sendto(bytes("valueerror", "utf-8"), addr)
                code, value = strdata.split("$")
                # print("Попытка использования нереализованной функции! Пропуск")
            except ValueError:
                pass
            # print("Something went wrong! Stopping car")
            # accelFunc("stop", 0)
            # steerFunc("straight", 0)
            # sock.sendto(data, addr)
    except OSError:
        # print("timeout")
        sock.sendto(bytes("timeout", "utf-8"), addr)
        accelFunc("stop", 0)
        steerFunc("straight", 0)
        sock.settimeout(None)
    
