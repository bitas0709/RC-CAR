# Преобразование значения в диапазон, позволяющий управлять моторами
def map(value, fromMin, fromMax, toMin, toMax):
    return int((value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin)

# Функция для прямого управления двигательным мотором для движения вперед
def moveForward(speed):
    IN1.on()
    IN2.off()
    ENA.duty(map(speed, 512, 1023, 150, 1023))

# Функция для прямого управления двигательным мотором для движения назад
def moveBack(speed): #движение назад
    IN1.off()
    IN2.on()
    ENA.duty(map(speed, 510, 0, 150, 1023))

# Функция для остановки двигательного мотора
def moveStop():
    IN1.off()
    IN2.off()
    ENA.duty(0)

# Функция для прямого управления поворотным двигателем для движения влево
def steerLeft(strength): #поворот колес влево
    IN3.on()
    IN4.off()
    ENB.duty(map(strength, 510, 0, 150, 1023))

# Функция для прямого управления поворотным двигателем для движения вправо
def steerRight(strength): #поворот колёс вправо
    IN3.off()
    IN4.on()
    ENB.duty(map(strength, 512, 1023, 150, 1023))

# Функция для выпрямления колес
def steerStop():
    IN3.off()
    IN4.off()
    ENB.duty(0)
