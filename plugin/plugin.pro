TARGET=qofonoextdeclarative
TEMPLATE = lib
CONFIG += plugin
QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-psabi

QT_VERSION=$$[QT_VERSION]

QT += qml
QT -= gui
LIBS += -L../src -lqofonoext

INCLUDEPATH += ../src

SOURCES = \
    qofonoextdeclarativeplugin.cpp \
    qofonoextmodemlistmodel.cpp \
    qofonoextdeclarativemodemmanager.cpp

HEADERS = \
    qofonoextdeclarativeplugin.h \
    qofonoextmodemlistmodel.h \
    qofonoextdeclarativemodemmanager.h

OTHER_FILES += qmldir

target.path = $$[QT_INSTALL_QML]/org/nemomobile/ofono
qmldir.path = $$[QT_INSTALL_QML]/org/nemomobile/ofono
qmldir.files += qmldir

INSTALLS += target qmldir
