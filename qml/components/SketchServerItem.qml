import QtQuick 2.6
import Sailfish.Silica 1.0

BackgroundItem {
    id: root

    property alias text: label.text

    height: Theme.itemHeightSmall

    HighlightImage {
        id: icon
        x: Theme.horizontalPageMargin
        anchors.verticalCenter: parent.verticalCenter
        source: "image://theme/icon-m-computer"
    }

    Label {
        id: label
        anchors {
            left: icon.right
            leftMargin: icon.width > 0 ? Theme.paddingMedium : 0
            verticalCenter: parent.verticalCenter
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
        }
        truncationMode: TruncationMode.Fade
        color: root.highlighted ? Theme.highlightColor : Theme.primaryColor
    }
}
