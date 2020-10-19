import network
import socket
from machine import Pin, PWM, Timer
import time

NodeLED = Pin(16, Pin.OUT)
NodeLED.value(0)
time.sleep(1)
NodeLED.value(1)
#time.sleep(5)

IN1 = Pin(4, Pin.OUT)  # Вправо D2
IN2 = Pin(0, Pin.OUT)  # Влево D3
IN3 = Pin(14, Pin.OUT)  # Назад D5
IN4 = Pin(12, Pin.OUT)  # Вперед D6

ENA = PWM(Pin(5), freq=1000)  # Рулевое управление D1
ENB = PWM(Pin(13), freq=1000)  # Управление ускорением D7

wifiAddr1 = ("MikroTik-3063", "714866840")

controlAppAddress = []

carSettings = {}
connectedToControlUnit = False
searchBroadcastMessage = "Looking for RC-CAR"
searchReplyMessage = "Here it is"
searchConnectionMessage = "Connecting"
searchConnectionEstablishedMessage = "Ready to control"
keepAliveTimer = Timer(-1)
keepAlivePacketSent = False
socketCreated = False
wlan = network.WLAN(network.STA_IF)
host = wlan.ifconfig()[0]
sock = socket
port = 52250


def main():
    global connectedToControlUnit
    try:
        readCarSettings()
    except OSError:
        initializeCarSettings()
        readCarSettings()
    connectToStation()
    host = wlan.ifconfig()[0]
    while True:
        if not connectedToControlUnit:
            waitForConnection()
        else:
            data, addr = sock.recvfrom(1024)
            print("data received is", list(data))
            print("connectedToControlUnit is", connectedToControlUnit)
            try:
                updateKeepAliveTimer()
                if (len(list(data)) == 1):
                    val1 = list(data)[0]
                    if (val1 == 255):
                        updateKeepAliveTimer()
                elif (len(list(data)) == 2):
                    val1, val2 = list(data)
                    if (val1 == 1): # acceleration
                        if (val2 >= int(carSettings['moveBackwardTo']) and val2 <= int(carSettings['moveBackwardFrom'])):
                            moveBack(val2)
                        elif (val2 > int(carSettings['moveBackwardFrom']) and val2 < int(carSettings['moveForwardFrom'])):
                            moveStop()
                        elif (val2 >= int(carSettings['moveForwardFrom']) and val2 <= int(carSettings['moveForwardTo'])):
                            moveForward(val2)
                    elif (val1 == 2): # steering
                        if (val2 >= int(carSettings['steerLeftTo']) and val2 <= int(carSettings['steerLeftFrom'])):
                            steerLeft(val2)
                        elif (val2 > int(carSettings['steerLeftFrom']) and val2 < int(carSettings['steerRightFrom'])):
                            steerStop()
                        elif (val2 >= int(carSettings['steerRightFrom']) and val2 <= int(carSettings['steerRightTo'])):
                            steerRight(val2)
                elif (len(list(data)) == 3):
                    val1, val2, val3 = list(data)
                    if (val1 == 3): # move car. val2 and val3 are speed and steering values
                        move(val2, val3)
                    elif (val1 == 4): # lightning control
                        pass
                    elif (val1 == 5): # suspension control
                        pass
                else:
                    # valArray = list(data)
                    moveStop()
            except OSError:
                moveStop()               

def createAPNetwork():
    wlan = network.WLAN(network.AP_IF)
    wlan.active(True)
    wlan.config(essid="RC-CAR")
    wlan.config(password="714866840")
    
def connectToStation():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(wifiAddr1[0], wifiAddr1[1])
    print("wlan connect =", wlan.isconnected())
    return wlan.isconnected()
    
def waitForConnection():
    global connectedToControlUnit
    global sock
    global socketCreated
    global controlAppAddress
    if not socketCreated:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind((host, port))
        socketCreated = True
    while not connectedToControlUnit:
        data, addr = sock.recvfrom(1024)
        if (data == bytes(searchBroadcastMessage, "utf-8")):
            sock.sendto(bytes(searchReplyMessage, "utf-8"), addr)
        elif (data == bytes(searchConnectionMessage, "utf-8")):
            sock.sendto(bytes(searchConnectionEstablishedMessage, "utf-8"), addr)
            controlAppAddress = addr
            keepAliveTimer.init(period=200, mode=Timer.PERIODIC, callback=lambda t:keepAlive())
            connectedToControlUnit = True
            print("connection  =", connectedToControlUnit)
        else:
            sock.sendto(bytes("Connection not established!", "utf-8"), addr)
            
def updateKeepAliveTimer():
    global keepAlivePacketSent
    keepAliveTimer.deinit()
    keepAliveTimer.init(period=200, mode=Timer.PERIODIC, callback=lambda t:keepAlive())
    keepAlivePacketSent = False
    
def keepAlive():
    global sock
    global keepAlivePacketSent
    global connectedToControlUnit
    global controlAppAddress
    if not keepAlivePacketSent:
        sock.sendto(bytes("OK?", "utf-8"), controlAppAddress)
        keepAlivePacketSent = True
    else:
        moveStop()
        connectedToControlUnit = False
        keepAliveTimer.deinit()
        keepAlivePacketSent = False
        print("Car disconnected!")
        print("connectedToControlUnit is", connectedToControlUnit)
    
# Преобразование значения в диапазон, позволяющий управлять моторами
def map(value, fromMin, fromMax, toMin, toMax):
    return int((value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin)

# Основная функция для управления ускорением и рулевым механизмом
def move(speed, steer):
    # Управление ускорением (без мертвых зон)
    if (speed > int(carSettings['moveBackwardFrom']) and speed < int(carSettings['moveForwardFrom'])):
        print("stopping the car")
        moveStop()
    elif (speed >= int(carSettings['moveForwardFrom']) and speed <= int(carSettings['moveForwardTo'])):
        print("moving forward...")
        moveForward(speed)
    elif (speed <= int(carSettings['moveBackwardFrom']) and speed >= int(carSettings['moveBackwardTo'])):
        print("moving backwards...")
        moveBack(speed)
    else: # Что тогда сюда вообще пришло?
        print("What the fuck!? Stopping the car!")
        moveStop()
    # Управление рулём (без мертвых зон)
    if (steer > int(carSettings['steerLeftFrom']) and steer < int(carSettings['steerRightFrom'])):
        steerStop()
    elif (steer >= int(carSettings['steerRightFrom']) and steer <= int(carSettings['steerRightTo'])):
        print("steering right...")
        steerRight(steer)
    elif (steer <= int(carSettings['steerLeftFrom']) and steer >= int(carSettings['steerLeftTo'])):
        print("steering left...")
        steerLeft(steer)
    else: # Что тогда сюда вообще пришло?
        steerStop()

#Функция для полной остановки машины
def stop():
    moveStop()
    steerStop()

# Функция для прямого управления двигательным мотором для движения вперед
def moveForward(speed):
    IN1.on()
    IN2.off()
    ENA.duty(map(speed, int(carSettings['moveForwardFrom']), int(carSettings['moveForwardTo']), 
                 int(carSettings['mapMoveForwardFrom']), int(carSettings['mapMoveForwardTo'])))

# Функция для прямого управления двигательным мотором для движения назад
def moveBack(speed): #движение назад
    IN1.off()
    IN2.on()
    ENA.duty(map(speed, int(carSettings['moveBackwardFrom']), int(carSettings['moveBackwardTo']),
                 int(carSettings['mapMoveBackwardFrom']), int(carSettings['mapMoveBackwardTo'])))

# Функция для остановки двигательного мотора
def moveStop():
    IN1.off()
    IN2.off()
    ENA.duty(0)

# Функция для прямого управления поворотным двигателем для движения влево
def steerLeft(strength): #поворот колес влево
    IN3.off()
    IN4.on()
    ENB.duty(map(strength, int(carSettings['steerLeftFrom']), int(carSettings['steerLeftTo']),
                 int(carSettings['mapSteerLeftFrom']), int(carSettings['mapSteerLeftTo'])))

# Функция для прямого управления поворотным двигателем для движения вправо
def steerRight(strength): #поворот колёс вправо
    IN3.on()
    IN4.off()
    ENB.duty(map(strength, int(carSettings['steerRightFrom']), int(carSettings['steerRightTo']),
                 int(carSettings['mapSteerRightFrom']), int(carSettings['mapSteerRightTo'])))

# Функция для выпрямления колес
def steerStop():
    IN3.off()
    IN4.off()
    ENB.duty(0)
    
def readCarSettings():
    global carSettings
    with open("carSettings.txt") as file:
        for line in file:
            key, *value = line.split()
            carSettings[key] = value[0]
        file.close()
    
def initializeCarSettings():
    file = open("carSettings.txt", 'x')
    
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
    file.write("moveForwardFrom 128\n") # Значение, при котором машина начнёт движение вперед
    file.write("moveForwardTo 255\n") # Значение, при котором машина будет ехать вперед на полном ходу
    file.write("moveBackwardFrom 126\n") # Значение, при котором машина начнёт движение назад
    file.write("moveBackwardTo 0\n") # Значение, при котором машина будет ехать назад на полном ходу
    file.write("steerLeftFrom 126\n") # Значение, при котором машина начнёт поворачивать налево
    file.write("steerLeftTo 0\n") # Значение, при котором колёса у машины будут повёрнуты максимально влево
    file.write("steerRightFrom 128\n") # Значение, при котором машина начнёт поворачивать направо
    file.write("steerRightTo 255\n") # Значение, при котором колёса у машина будут повёрнуты максимально вправо
    
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
    
def sendCarSettingsToControlApp():
    pass

#There it is
main()
