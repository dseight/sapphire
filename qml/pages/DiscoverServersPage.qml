import QtQuick 2.6
import Sailfish.Silica 1.0
import com.dseight.sapphire 1.0
import "../components"

Page {
    SilicaListView {
        anchors.fill: parent
        model: SketchServerModel

        header: Column {
            width: parent.width

            PageHeader {
                title: "Sketches"
            }

            Label {
                anchors {
                    left: parent.left
                    right: parent.right
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                }
                text: "If you’re having issues with device discovery, make sure that both devices are connected to the same Wi-Fi network."
                color: Theme.secondaryHighlightColor
                wrapMode: Text.WordWrap
                bottomPadding: Theme.paddingLarge
                font.pixelSize: Theme.fontSizeSmall
            }
        }

        delegate: SketchServerItem {
            text: model.name
            onClicked: {
                var server = SketchServerModel.get(model.index)
                server.connect()
                pageStack.replace("DocumentPage.qml", {server: server})
            }
        }

        VerticalScrollDecorator {}
    }
}
