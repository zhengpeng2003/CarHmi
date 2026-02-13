QT       += core gui multimedia
 QT += widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    confirmpage.cpp \
    main.cpp \
    mainwindow.cpp \
    opentreethread.cpp \
    picanimation.cpp \
    picbutton.cpp \
    piclistitem.cpp \
    piclistwidget.cpp \
    picshow.cpp \
    prosetpage.cpp \
    protree.cpp \
    protreeitem.cpp \
    protreethread.cpp \
    protreewidget.cpp \
    removeprotree.cpp \
    slidebutton.cpp \
    slideshowdialog.cpp \
    type.cpp \
    wizard.cpp

HEADERS += \
    confirmpage.h \
    mainwindow.h \
    opentreethread.h \
    picanimation.h \
    picbutton.h \
    piclistitem.h \
    piclistwidget.h \
    picshow.h \
    prosetpage.h \
    protree.h \
    protreeitem.h \
    protreethread.h \
    protreewidget.h \
    removeprotree.h \
    slidebutton.h \
    slideshowdialog.h \
    type.h \
    wizard.h

FORMS += \
    confirmpage.ui \
    mainwindow.ui \
    picshow.ui \
    prosetpage.ui \
    protree.ui \
    removeprotree.ui \
    slideshowdialog.ui \
    wizard.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc


