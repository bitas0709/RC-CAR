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
    Column {
        Row {
            spacing: 10
            Label {
                text: "Угол акселерометра"
            }
            ComboBox {
                width: 200
                model: ListModel {
                    id: accelListModelItems
                    ListElement { text: "90 градусов влево"; value: "-1" }
                    ListElement { text: "без корректировки"; value: "0" }
                    ListElement { text: "90 градусов вправо"; value: "1" }
                    ListElement { text: "разворот на 180 градусов"; value: "2" }
                }
            }
        }
        Row {
            spacing: 10
            Label {
                text: "IP машинки"
            }
            TextInput {
                width: 200
            }
        }
        Row {
            spacing: 10
            Label {
                text: "Порт"
            }
            TextInput {
                width: 200
            }
        }
    }
}
