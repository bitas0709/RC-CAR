# Данный файл предназначен для настройки возможностей прошивки, отключения или включения её отдельных компонентов.

# В будущем надо сильно переделать оформление данной страницы, так как "раскидывание" каждой ""фичи"" в отдельные блоки ни к чему хорошему в плане читаемости не приведут.
# Так как на данный момент идёт только формирование списка возможностей прошивки, оптимизация оформления файла будет полностью игнорироваться.

# If steering wheels will be controlled by stepper motor, uncomment code line below
# steeringStepperMotorFeature = true

# Otherwise, if steering wheels will be controlled by brush DC motor, uncomment line below
# steeringBrushDCMotorFeature = true

# If you want to use servo in steering, uncomment line below
steeringServoMotorFeature = true

# Uncomment one of lines below which type of motor you want to use
#accelerationStepperMotorFeature = true
accelerationBrushDCMotorFeature = true
#accelerationBrushlessDCMotorFeature = true

# Uncomment lines below if car has some of this features
hasHeadlights = true
hasStoplights = true
hasTurnlights = true
hasReverseLights = true
hasFoglights = true

import json  

# Get enabled features in firmware. Makes JSON
def getFeatures():
    features = {}
    
    # get type of steering motor
    if 'steeringStepperMotorFeature' in globals():
        features['steeringMotor'] = 'stepper'
    elif 'steeringServoMotorFeature' in globals():
        features['steeringMotor'] = 'servo'
    elif 'steeringBrushDCMotorFeature' in globals():
        features['steeringMotor'] = 'brushDC'
    else:
        print('two or more types of motor has been uncommented! Enabling stepper by default')
        
    # get type of acceleration motor
    if 'accelerationBrushDCMotorFeature' in globals():
        features['accelerationMotor'] = 'brushDC'
    elif 'accelerationBrushlessDCMotorFeature' in globals():
        features['accelerationMotor'] = 'brushlessDC'
    elif 'accelerationStepperMotorFeature' in globals():
        features['accelerationMotor'] = 'stepper'
    
