import QtQuick 2.6
import Sailfish.Silica 1.0
import com.dseight.sapphire 1.0

FullscreenContentPage {
    id: root

    property var page
    property alias currentIndex: slideshowView.currentIndex

    onCurrentIndexChanged: {
        if (status !== PageStatus.Active) {
            return
        }
        pageStack.previousPage(root).jumpToArtboard(page, currentIndex)
    }

    // Handle change of current artboard by Sketch
    Connections {
        target: page
        onCurrentIndexChanged: {
            slideshowView.positionViewAtIndex(page.currentIndex, PathView.Center)
        }
    }

    SlideshowView {
        id: slideshowView
        anchors.fill: parent

        model: page
        itemWidth: width
        itemHeight: height

        delegate: Loader {
            // We have to get page object here to listen for source changes.
            // because direct usage of model.fullscreenSource will be evaluated
            // only once.
            property var _page: page.get(index)

            width: root.width
            height: root.height
            asynchronous: !PathView.isCurrentItem

            sourceComponent: SilicaFlickable {
                anchors.fill: parent
                flickableDirection: Flickable.VerticalFlick
                contentWidth: root.width
                contentHeight: Math.max(root.height, image.height)

                Image {
                    id: image
                    anchors.centerIn: parent
                    source: _page.fullscreenSource
                    asynchronous: true
                }

                Item {
                    width: root.width
                    height: root.height
                    visible: image.status != Image.Ready
                    ProgressCircle {
                        anchors.centerIn: parent
                        value: image.progress
                        // Explicitly set colors, so circle will not alternate them
                        progressColor: palette.highlightColor
                        backgroundColor: palette.highlightDimmerColor
                    }
                }
            }
        }
    }
}
