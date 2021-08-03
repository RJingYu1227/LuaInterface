#-------------------------------------------------
#
# Project created by QtCreator 2021-05-13T10:18:56
#
#-------------------------------------------------

QT       -= gui

TARGET = LuaInterface
TEMPLATE = lib
CONFIG += plugin no_plugin_name_prefix
DEFINES += LUAINTERFACE_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        luainterface.cpp \
        objecttranslator.cpp \
        bit.c \
        lfs.c \
        luafunction.cpp

HEADERS += \
        luainterface.h \
        luainterface_global.h \ 
        objecttranslator.h \
        luamethodwrapper.h \
        lfs.h \
        luafunction.h

CONFIG(debug, debug|release) {
    OUTPUT = Debug
} else {
    OUTPUT = Release
}

target.path = $$PWD/$${OUTPUT}
INSTALLS += target

INCLUDEPATH += $$PWD/../include/

windows{
    LIBS += -L$$PWD/../lib/ -llua51
}
unix{
    LIBS += -L$$PWD/../lib/ -llua
}
