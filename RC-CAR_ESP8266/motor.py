# Настройки рулевого управления и ускорения (передаются из основных настроек)
motorSettings = {}

# Преобразование значения в диапазон, позволяющий управлять моторами
def map(value, fromMin, fromMax, toMin, toMax):
    return int((value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin)

# Основная функция для управления ускорением и рулевым механизмом
def move(speed, steer):
    # Управление ускорением (без мертвых зон)
    if (speed > motorSettings['moveBackwardFrom'] and speed < motorSettings['moveForwardFrom']):
        moveStop()
    elif (speed >= motorSettings['moveForwardFrom'] and speed <= motorSettings['moveForwardTo']):
        moveForward(speed)
    elif (speed <= motorSettings['moveBackwardFrom'] and speed >= motorSettings['moveBackwardTo']):
        moveBack(speed)
    else: # Что тогда сюда вообще пришло?
        moveStop()
    # Управление рулём (без мертвых зон)
    if (steer > motorSettings['steerLeftFrom'] and steer < motorSettings['steerRightFrom']):
        steerStop()
    elif (steer >= motorSettings['steerRightFrom'] and steer <= motorSettings['steerRightTo']):
        steerRight(steer)
    elif (steer <= motorSettings['steerLeftFrom'] and steer >= motorSettings['steerLeftTo']):
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
    ENA.duty(map(speed, motorSettings['moveForwardFrom'], motorSettings['moveForwardTo'], 
                 motorSettings['mapMoveForwardFrom'], motorSettings['mapMoveForwardTo']))

# Функция для прямого управления двигательным мотором для движения назад
def moveBack(speed): #движение назад
    IN1.off()
    IN2.on()
    ENA.duty(map(speed, motorSettings['moveBackwardFrom'], motorSettings['moveBackwardTo'],
                 motorSettings['mapMoveBackwardFrom'], motorSettings['mapMoveBackwardTo']))

# Функция для остановки двигательного мотора
def moveStop():
    IN1.off()
    IN2.off()
    ENA.duty(0)

# Функция для прямого управления поворотным двигателем для движения влево
def steerLeft(strength): #поворот колес влево
    IN3.on()
    IN4.off()
    ENB.duty(map(strength, motorSettings['steerLeftFrom'], motorSettings['steerLeftTo'],
                 motorSettings['mapSteerLeftFrom'], motorSettings['mapSteerLeftTo']))

# Функция для прямого управления поворотным двигателем для движения вправо
def steerRight(strength): #поворот колёс вправо
    IN3.off()
    IN4.on()
    ENB.duty(map(strength, motorSettings['steerRightFrom'], motorSettings['steerRightTo'],
                 motorSettings['mapSteerRightFrom'], motorSettings['mapSteerRightTo']))

# Функция для выпрямления колес
def steerStop():
    IN3.off()
    IN4.off()
    ENB.duty(0)
