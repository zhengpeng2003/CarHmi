QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    anmationpixmap.cpp \
    bgmcontrol.cpp \
    card.cpp \
    cardpanel.cpp \
    cards.cpp \
    endpanel.cpp \
    gamecontrol.cpp \
    loadprocess.cpp \
    main.cpp \
    maingame.cpp \
    mybutton.cpp \
    mybuttongroup.cpp \
    player.cpp \
    playhand.cpp \
    robot.cpp \
    robotgetloard.cpp \
    robotplayhand.cpp \
    showscore.cpp \
    strategy.cpp \
    timecount.cpp \
    user.cpp

HEADERS += \
    anmationpixmap.h \
    bgmcontrol.h \
    card.h \
    cardpanel.h \
    cards.h \
    endpanel.h \
    gamecontrol.h \
    loadprocess.h \
    maingame.h \
    mybutton.h \
    mybuttongroup.h \
    player.h \
    playhand.h \
    robot.h \
    robotgetloard.h \
    robotplayhand.h \
    showscore.h \
    strategy.h \
    timecount.h \
    user.h

FORMS += \
    maingame.ui \
    mybuttongroup.ui \
    showscore.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
