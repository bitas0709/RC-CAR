import QtQuick 2.13
import QtQuick.Window 2.13
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.1
import inetSocket 1.0
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Universal 2.12

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("RC-CAR")
    id: window
    InetSocketClass {
        id: inetSocket
    }

    Shortcut {
        sequences: ["Esc", "Back"]
        //enabled: Дописать при добавлении бокового меню
        onActivated: {

        }
    }

    header: ToolBar {
        id: upperToolBar

        background: Rectangle {
            anchors.fill: parent
            color: "blue"
        }

        RowLayout {
            spacing: 20
            anchors.fill: parent

            ToolButton {
                background: Rectangle {
                    anchors.fill: parent
                    color: "blue"
                }
                icon.source: mainDrawer.visible ? "qrc:/src/icons/back.png" : "qrc:/src/icons/drawer.png"
                onClicked: {
                    mainDrawer.visible ? mainDrawer.close(): mainDrawer.open()
                }
            }

            Label {
                id: titleLabel
                color: "white"
                text: listView.currentIndex ? "RC-CAR" : listView.currentItem.txt
                font.pixelSize: 20
                font.bold: true
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignCenter
                Layout.fillWidth: true
            }
        }
    }

    Drawer {
        y: header.height
        id: mainDrawer
        width: Math.min(window.width, window.height) / 3 * 2
        height: parent.height - header.height
        interactive: stackView.depth === 1

        ListView {
            id: listView
            focus: true
            height: parent.height
            width: parent.width - 10
            currentIndex: -1
            leftMargin: 10

            delegate: ItemDelegate {
                width: parent.width
                Row {
                    Image {
                        height: parent.height
                        width: parent.height
                        source: model.imgsrc
                    }
                    Text {
                        horizontalAlignment: Text.AlignHCenter
                        text: model.txt
                    }
                }

                highlighted: ListView.isCurrentItem
                onClicked: {
                    if (!index) {
                        stackView.pop()
                        mainDrawer.close()
                    } else {
                        listView.currentIndex = index
                        stackView.pop()
                        stackView.push(model.source)
                        mainDrawer.close()
                    }
                }
            }
            model: ListModel {
                ListElement { imgsrc: "qrc:/src/icons/home.png"; txt: "Главная"; source: "qrc:/main.qml" }
                ListElement { imgsrc: "qrc:/src/icons/gamepad.png"; txt: "Управление"; source: "qrc:/controlPage.qml" }
                ListElement { imgsrc: "qrc:/src/icons/settings.png"; txt: "Настройки"; source: "qrc:/settings.qml" }
            }
            ScrollIndicator.vertical: ScrollIndicator { }
        }
    }

    StackView {
        id: stackView
        anchors.fill: parent

        initialItem: Pane {
            id: pane
            Column {
                anchors.centerIn: parent
                Row {
                    Label {
                        text: "Connect to: "
                    }
                    Label {
                        text: inetSocket.hostAddress
                    }
                }
                Row {
                    Label {
                        text: "Port: "
                    }
                    Label {
                        text: inetSocket.port
                    }
                }
            }
        }
    }
}
