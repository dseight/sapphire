import QtQuick 2.6
import Sailfish.Silica 1.0
import com.dseight.sapphire 1.0

Page {
    id: root

    property var server
    readonly property double thumbnailScale:
        Screen.sizeCategory >= Screen.Large ? 0.22 : 0.35
    property var _document: server.document
    property var _artboardView

    signal jumpToArtboard(var page, int artboardIndex)

    Connections {
        target: _document
        onCurrentIndexChanged: {
            if (_artboardView) {
                var page = _document.get(_document.currentIndex)
                _artboardView.page = page
                _artboardView.currentIndex = page.currentIndex
            }
        }
    }

    SilicaListView {
        id: pagesListView
        anchors.fill: parent
        opacity: server.connected ? 1.0 : 0.0
        Behavior on opacity { FadeAnimator { duration: 400 } }

        model: _document

        Connections {
            target: _document
            onCurrentIndexChanged: {
                pagesListView.positionViewAtIndex(_document.currentIndex, ListView.Center)
            }
        }

        header: PageHeader {
            title: server.connected ? _document.name : ""
        }

        footer: Item { height: Theme.paddingLarge; width: 1 }

        delegate: Column {
            property var sketchPage: _document.get(model.index)

            width: parent.width

            Component.onCompleted: {
                sketchPage.thumbnailWidth = Screen.width * thumbnailScale
                sketchPage.minThumbnailHeight = Screen.width * thumbnailScale
                sketchPage.maxThumbnailHeight = Screen.height * thumbnailScale
            }

            SectionHeader {
                text: model.name
            }

            SilicaListView {
                id: artboardsListView
                model: sketchPage

                width: parent.width
                // FIXME: properly calculate height for text, it is slightly out of size
                height: sketchPage.thumbnailHeight
                        + Theme.fontSizeExtraSmall
                        + 2 * Theme.paddingSmall

                spacing: Theme.paddingMedium
                orientation: ListView.Horizontal

                // Handle change of current artboard from ArtboardView
                Connections {
                    target: root
                    onJumpToArtboard: {
                        if (page === sketchPage) {
                            artboardsListView.positionViewAtIndex(
                                artboardIndex, ListView.Center)
                        }
                    }
                }

                // Handle change of current artboard by Sketch
                Connections {
                    target: sketchPage
                    onCurrentIndexChanged: {
                        artboardsListView.positionViewAtIndex(
                            sketchPage.currentIndex, ListView.Center)
                    }
                }

                // Don't stick to screen edges
                header: Item { height: 1; width: Theme.horizontalPageMargin }
                footer: Item { height: 1; width: Theme.horizontalPageMargin }

                delegate: Column {
                    width: sketchPage.thumbnailWidth

                    Image {
                        id: image
                        width: parent.width
                        height: sketchPage.thumbnailHeight
                        source: model.thumbnailSource
                        fillMode: Image.PreserveAspectCrop
                        smooth: false
                        asynchronous: true

                        BusyIndicator {
                            anchors.centerIn: parent
                            size: BusyIndicatorSize.Medium
                            running: image.status != Image.Ready
                        }

                        BackgroundItem {
                            id: imageHighlight
                            anchors.fill: parent
                            onClicked: {
                                _artboardView = pageStack.push("ArtboardView.qml", {
                                    page: sketchPage,
                                    currentIndex: model.index
                                })
                            }
                        }
                    }

                    Label {
                        width: parent.width
                        topPadding: Theme.paddingSmall
                        bottomPadding: Theme.paddingSmall
                        text: model.name
                        truncationMode: TruncationMode.Fade
                        font.pixelSize: Theme.fontSizeExtraSmall
                        color: imageHighlight.highlighted
                               ? Theme.highlightColor
                               : Theme.primaryColor
                    }
                }
            }
        }
    }

    // TODO: add some note that user must authorize device in the Sketch toolbar
    BusyLabel {
        running: !server.connected && !delayPageBusyIndicator.running
        text: "Connecting"

        Timer {
            id: delayPageBusyIndicator
            running: !server.connected
            interval: 1000
        }
    }

    Connections {
        target: server
        onConnectedChanged: {
            if (!server.connected) {
                pageStack.replaceAbove(null, "DiscoverServersPage.qml")
            }
        }
    }
}
