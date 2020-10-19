import QtQuick 2.15
import QtQuick.Controls 2.15

Page {
    width: 600
    height: 400
    property alias accelerationSlider: accelerationSlider

    title: qsTr("Page 1")

    Slider {
        id: accelerationSlider
        x: 14
        y: 190
        snapMode: Slider.SnapOnRelease
        orientation: Qt.Vertical
        to: 255
        value: 128

        handle: Rectangle {
            x: accelerationSlider.leftPadding + accelerationSlider.availableWidth / 2 - width / 2
            y: accelerationSlider.topPadding + accelerationSlider.visualPosition
               * (accelerationSlider.availableHeight - height)
            implicitWidth: 26
            implicitHeight: 26
            radius: 13
            color: accelerationSlider.pressed ? "#f0f0f0" : "#f6f6f6"
            border.color: "#bdbebf"
        }

        Connections {
            target: accelerationSlider
            onPressedChanged: {
                if (!accelerationSlider.pressed) {
                    accelerationSlider.value = 128
                }
            }
        }
    }
}
