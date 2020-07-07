import network
import socket
from machine import Pin, PWM
import time
import motor

NodeLED = Pin(16, Pin.OUT)
NodeLED.value(0)
time.sleep(1)
NodeLED.value(1)

# Словарь, содержащий настройки, считанные из файла settings.txt
settings = {}

def openSettingsFile(): # Открытие и считывание файла настроек
    with open("settings.txt") as file:
        for line in file:
            key, *value = line.split()
            settings[key] = value
        file.close()
        motor.motorSettings = settings.copy() # Копирование настроек в настройки моторов

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
    openSettingsFile()
# Инициализация базовых настроек, если файл не существует
except OSError:
    file = open("settings.txt", 'x')
    
    # Основные настройки сети
    file.write("intWiFiSSID RC-CAR\n") # Название точки доступа, создаваемой ESP
    file.write("intWiFiPass 1234568790\n") # Пароль точки доступа, создаваемой ESP
    file.write("extWiFi inactive\n") # Состояние подключения к точке доступа (inactive/active)
    file.write("extWiFiSSID NULL\n") # Название точки доступа, к которой будет подключена ESP
    file.write("extWiFiPass NULL\n") # Пароль точки доступа, к которой будет подключена ESP
    file.write("port 5000\n") # Порт, на котором идёт прослушивание команд
    
    # Настройки рулевого управления и ускорения
    file.write("moveDeadArea 0\n") # В процентах (от 0 до 100)
    file.write("steerDeadArea 0\n") # В процентах (от 0 до 100)
    file.write("moveForwardFrom 512\n") # Значение, при котором машина начнёт движение вперед
    file.write("moveForwardTo 1023\n") # Значение, при котором машина будет ехать вперед на полном ходу
    file.write("moveBackwardFrom 510\n") # Значение, при котором машина начнёт движение назад
    file.write("moveBackwardTo 0\n") # Значение, при котором машина будет ехать назад на полном ходу
    file.write("steerLeftFrom 510\n") # Значение, при котором машина начнёт поворачивать налево
    file.write("steerLeftTo 0\n") # Значение, при котором колёса у машины будут повёрнуты максимально влево
    file.write("steerRightFrom 512\n") # Значение, при котором машина начнёт поворачивать направо
    file.write("steerRightTo 1023\n") # Значение, при котором колёса у машина будут повёрнуты максимально вправо
    
    # Значения, в которые будет произведено преобразование полученных значений
    # Используется для калибровки движения машины
    file.write("mapMoveForwardFrom 150\n") # Начальное значение ШИМ, подаваемое на мотор
    file.write("mapMoveForwardTo 1023\n") # Конечное значение ШИМ, подаваемое на мотор
    file.write("mapMoveBackwardFrom 150\n")
    file.write("mapMoveBackwardTo 1023\n")
    file.write("mapSteerLeftFrom 150\n")
    file.write("mapSteerLeftTo 1023\n")
    file.write("mapSteerRightFrom 150\n")
    file.write("mapSteerRightTo 1023\n")
    
    file.close()
    openSettingsFile()

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
    except OSError:
        # print("timeout")
        sock.sendto(bytes("timeout", "utf-8"), addr)
        motor.stop()
        sock.settimeout(None)
