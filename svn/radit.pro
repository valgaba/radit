QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = radit

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    core/Clipboard.cpp \
    core/io.cpp \
    main.cpp \
    mainwindow.cpp \
    widgets/AudioItem.cpp \
    widgets/AudioItemFile.cpp \
    widgets/AudioItemFolder.cpp \
    widgets/AudioItemMini.cpp \
    widgets/AudioItemNet.cpp \
    widgets/ContentsPlayer.cpp \
    widgets/FormAbout.cpp \
    widgets/FormProperties.cpp \
    widgets/FormPropertiesAudioItem.cpp \
    widgets/ItemBase.cpp \
    widgets/TabAuto.cpp \
    widgets/TabPlayer.cpp \
    widgets/button.cpp \
    widgets/container.cpp \
    widgets/contents.cpp \
    widgets/contentsbase.cpp \
    widgets/frame.cpp \
    widgets/menu.cpp \
    widgets/scrollbar.cpp \
    widgets/tab.cpp \
    widgets/tabbar.cpp

HEADERS += \
    core/Clipboard.h \
    core/io.h \
    mainwindow.h \
    widgets/AudioItem.h \
    widgets/AudioItemFile.h \
    widgets/AudioItemFolder.h \
    widgets/AudioItemMini.h \
    widgets/AudioItemNet.h \
    widgets/ContentsPlayer.h \
    widgets/DraggableWidget.h \
    widgets/FormAbout.h \
    widgets/FormProperties.h \
    widgets/FormPropertiesAudioItem.h \
    widgets/ItemBase.h \
    widgets/TabAuto.h \
    widgets/TabPlayer.h \
    widgets/button.h \
    widgets/container.h \
    widgets/contents.h \
    widgets/contentsbase.h \
    widgets/frame.h \
    widgets/menu.h \
    widgets/scrollbar.h \
    widgets/tab.h \
    widgets/tabbar.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# -------------------------------------------------
# mis lib
# -------------------------------------------------


win32 {

LIBS += -L$$PWD/../lib/bass \
        -L$$PWD/../lib/bassenc


INCLUDEPATH += $$PWD/../lib/bass \
               $$PWD/../lib/bassenc

}

DESTDIR =     $$PWD/../out/bin
#OBJECTS_DIR = $$PWD/../out
#MOC_DIR =     $$PWD/../out





#-----------------------------------------------------------

unix {

   # LIBS += /usr/lib/libbass.so
           # /usr/lib/libbassenc.so

   # INCLUDEPATH = ../lib/bass
                #  ../lib/bassenc

}



#-------------------------------------------------------------








RESOURCES += \
    radit.qrc

RC_FILE = radit.rc


