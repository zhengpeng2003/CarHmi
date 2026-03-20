QT += widgets quickwidgets quick positioning location network
INCLUDEPATH += /work/Qt_Object/maptest/nmealib_api/nmealib/include

LIBS += -L/work/Qt_Object/maptest/nmealib_api/nmealib/lib \
        -lnmea
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    gpsmanager.cpp \
    main.cpp \
    widget.cpp

HEADERS += \
    gpsmanager.h \
    widget.h

FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Keyboard.qml \
    MapTypePanel.qml \
    MapView.qml \
    MapView阿.qml \
    SearchBar.qml \
    main.qml

RESOURCES += \
    qml.qrc
