import network
import socket
from machine import Pin, PWM, Timer
import time
import settingsAPI
import motor
import light

NodeLED = Pin(16, Pin.OUT)
NodeLED.value(0)
time.sleep(1)
NodeLED.value(1)

# Словарь, содержащий настройки, считанные из файла settings.txt
settings = {}

def createAPNetwork(): # Создание собственной точки доступа
    wlan = network.WLAN(network.AP_IF)
    wlan.active(True)
    wlan.config(essid=settings['intWiFiSSID'])
    wlan.config(password=settings['intWiFiPass'])
    
def connectToStation(): # Подключение к внешней точке доступа, заданной в настройках
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(settings['extWiFiSSID'], settings['extWiFiPass'])
    for i in range(1, 3):
        if not wlan.isconnected():
            time.sleep(1)
    wlan.active(False)
    return wlan.isconnected() 

try:  # Попытка открыть файл
    settings.openSettingsFile()
# Инициализация базовых настроек, если файл не существует
except OSError:
    settings.setDefaultSettings()
    settings.openSettingsFile()

if (settings['extWiFi'] == "inactive") or (settings['extWiFiSSID'] == "NULL") or (settings['extWiFiPass'] == "NULL"):
    createAPNetwork()
else:
    if not connectToStation():
        createAPNetwork()

host = wlan.ifconfig()[0]
port = settings[port]

# Создание UDP сокета
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((host, port))

while True:
    try: # Получение данных с управляющего устройства
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
            elif (recvvalues[0] == '03'): # Задание настроек машинки
                # На вход приходит код и два значения
                if (settings.get(recvvalues[1])):
                    settings[recvvalues[1]] = recvvalues[2]
                else:
                    pass # Такой настройки не существует
            elif (recvvalues[0] == '04'): # Управление фарами
                pass # Пока не реализовано
            pass
        except OSError:
            pass
    except OSError:
        # print("timeout")
        sock.sendto(bytes("timeout", "utf-8"), addr)
        motor.stop()
        sock.settimeout(None)
