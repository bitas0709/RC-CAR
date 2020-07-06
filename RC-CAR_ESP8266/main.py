import network
import socket
from machine import Pin, PWM
import time
import motor

NodeLED = Pin(16, Pin.OUT)
NodeLED.value(0)
time.sleep(1)
NodeLED.value(1)

IN1 = Pin(4, Pin.OUT)  # Вправо D2
IN2 = Pin(0, Pin.OUT)  # Влево D3
IN3 = Pin(14, Pin.OUT)  # Назад D5
IN4 = Pin(12, Pin.OUT)  # Вперед D6

ENA = PWM(Pin(5))  # Рулевое управление D1
ENB = PWM(Pin(13))  # Управление ускорением D7

# Словарь, содержащий настройки, считанные из файла settings.txt
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
    wlan.config(essid=settings['intWiFiSSID'])
    wlan.config(password=settings['intWiFiPass'])
    
def connectToStation():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(settings['extWiFiSSID'], settings['extWiFiPass'])
    for i in range(1, 3):
        if not wlan.isconnected():
            time.sleep(1)
    wlan.active(False)
    return wlan.isconnected()

try:  # Попытка открыть файл
    openSettingsFile()
# Инициализация базовых настроек, если файл не существует
except OSError:
    file = open("settings.txt", 'x')
    file.write("intWiFiSSID RC-CAR\n")
    file.write("intWiFiPass 1234568790\n")
    file.write("extWiFi inactive\n")
    file.write("extWiFiSSID NULL\n")
    file.write("extWiFiPass NULL\n")
    file.write("port 5000\n")
    file.write("moveDeadArea 0") # В процентах (от 0 до 100)
    file.close()
    openSettingsFile()

if (settings['extWiFi'] == "inactive") or (settings['extWiFiSSID'] == "NULL") or (settings['extWiFiPass'] == "NULL"):
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
            recvvalues = []
            recvvalues = strdata.split("$")
            if (recvvalues[0] == '01'): # Управление ускорением и рулём
                # На вход приходит код и два значения
                motor.move(recvvalues[1], recvvalues[2])
            elif (recvvalues[0] == '02'): # Получение настроек из машинки
                # На вход приходит код и одно значение
                try:
                    sock.sendto(bytes(settings[recvvalues[1]], "utf-8"), addr)
                except KeyError: # На случай, если данной настройки не существует
                    pass
                pass
            elif (recvvalues[0] == '03'): # Задание настроек машинки
                # На вход приходит код и два значения
                pass
            elif (recvvalues[0] == '04'): # Управление фарами
                pass
