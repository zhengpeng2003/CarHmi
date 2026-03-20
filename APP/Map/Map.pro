QT += widgets quickwidgets quick positioning location network

# ========== 平台路径配置 ==========
# 通过外部变量 PLATFORM 来切换
# 桌面环境: qmake
# 板子环境: qmake "PLATFORM=arm"
isEmpty(PLATFORM) {
    PLATFORM = desktop
}


    NMEA_PATH = /work/Qt_Object/maptest/nmealib_api/nmealib
    message("Using desktop NMEA path")

    #NMEA_PATH = /work/Qt_Object/maptest/nmealib_api/nmelib_arm
  # 改为你板子上的实际路径
    #message("Using ARM board NMEA path")


INCLUDEPATH += $$NMEA_PATH/include
LIBS += -L$$NMEA_PATH/lib -lnmea

# ========== 以下内容保持不变 ==========
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
    SearchBar.qml \
    main.qml

RESOURCES += \
    qml.qrc
