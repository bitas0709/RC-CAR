import QtQuick 2.15
import QtQuick.Controls 2.15

Page {
    width: 600
    height: 400

    title: qsTr("Home")

    Column {

        x: 191
        y: 309

        Row {

            Column {

                Text {
                    id: connectionInfo
                    color: "#ffffff"
                    text: qsTr("Status:")
                    font.pointSize: 11
                }

                Text {
                    id: addressInfo
                    color: "#ffffff"
                    text: qsTr("RC-Car Address:")
                    font.pointSize: 11
                }

                Text {
                    id: portInfo
                    color: "#ffffff"
                    text: qsTr("Port:")
                    font.pointSize: 11
                }
            }

            Column {

                Text {
                    id: connectionStatus
                    color: "#ffffff"
                    text: inetSocket.connectedToCar ? "Connected" : "Not connected"
                    font.pointSize: 11

                    Connections {
                        target: inetSocket
                        onRc_carConnectionStatusChangedSignal: connectionStatus.text = "Connected"
                    }
                }

                Text {
                    id: addressRCCAR
                    color: "#ffffff"
                    text: inetSocket.RC_CarAddress
                    //text: qsTr("Not Connected")
                    font.pointSize: 11

                    Connections {
                        target: inetSocket
                        onRc_carAddressChangedSignal: addressRCCAR.text = text
                    }
                }

                Text {
                    id: portRCCAR
                    color: "#ffffff"
                    text: inetSocket.RC_CarPort
                    //text: qsTr("52500")
                    font.pointSize: 11
                }
            }
        }

        Button {
            id: establishConnectionButton
            text: "Press dat button"

            Connections {
                target: establishConnectionButton
                onPressed: {
                    inetSocket.establishConnection()
                }
            }
        }
    }
}
