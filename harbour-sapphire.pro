TARGET = harbour-sapphire

QT += websockets

CONFIG += c++14
CONFIG += link_pkgconfig
CONFIG += sailfishapp
PKGCONFIG += \
    mlite5 \
    qmdnsengine

DEFINES += VERSION_STRING=\\\"$${VERSION}\\\"

SOURCES += \
    src/deviceinfo.cpp \
    src/sketchartboard.cpp \
    src/sketchdocument.cpp \
    src/sketchpage.cpp \
    src/sketchserver.cpp \
    src/sketchservermodel.cpp \
    src/main.cpp

HEADERS += \
    src/sketchartboard.h \
    src/sketchdocument.h \
    src/sketchpage.h \
    src/sketchserver.h \
    src/sketchservermodel.h \
    src/qqmlobjectlistmodel.h

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172
