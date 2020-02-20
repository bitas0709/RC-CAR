import QtQuick 2.12
import QtQuick 2.0
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls 1.4
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Universal 2.12
import Qt.labs.settings 1.0

Page {
    onVisibleChanged: {
        accelSlider.value = 511
        inetSocket.accelVal = 511
    }

    /*Shortcut {
        sequence: "W"
        onActivated: accelSlider.value = 1023
    }
    Shortcut {
        sequence: "S"
        onActivated: accelSlider.value = 0
    }
    Shortcut {
        sequence: "A"
        onActivated: steeringSlider.value = 0
    }
    Shortcut {
        sequence: "D"
        onActivated: steeringSlider.value = 1023
    }*/ //шорткаты ещё надо доработать, чтобы при отпускании кнопки значение возвращалось на первоначальную позицию

    MultiPointTouchArea {
        anchors.fill: parent
        touchPoints: [
            TouchPoint { id: touchPoint1 },
            TouchPoint { id: touchPoint2 }
        ]
    }

    Slider {
        id: accelSlider
        //handle: vertical
        orientation: Qt.Vertical
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 20
        value: 511
        stepSize: 1.0
        minimumValue: 0
        maximumValue: 1023
        height: 200
        onValueChanged: {
            inetSocket.accelVal = accelSlider.value
            inetSocket.valueChanged()
        }
        onPressedChanged: {
            if (!pressed) {
                accelSlider.value = 511
                inetSocket.accelVal = accelSlider.value
            }
        }
    }
    Slider {
        id: steeringSlider
        //handle: horizontal
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20
        value: 511
        minimumValue: 0
        maximumValue: 1023
        visible: !inetSocket.isAccelEnabled()
        width: 200
        onValueChanged: {
            inetSocket.steerVal = steeringSlider.value
            inetSocket.valueChanged()
        }
        onPressedChanged: {
            if (!pressed) {
                steeringSlider.value = 511
                inetSocket.steerVal = steeringSlider.value
            }
        }
    }
    Button {
        id: testButton
        anchors.bottom: parent.bottom
        anchors.centerIn: parent
    }
}
