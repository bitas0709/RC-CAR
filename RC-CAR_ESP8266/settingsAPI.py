def openSettingsFile(): # Открытие и считывание файла настроек
    with open("settings.txt") as file:
        for line in file:
            key, *value = line.split()
            settings[key] = value
        file.close()
        motor.motorSettings = settings.copy() # Копирование настроек в настройки моторов

def setDefaultSettings():
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