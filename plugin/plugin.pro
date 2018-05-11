TARGET=qofonoextdeclarative
TEMPLATE = lib
CONFIG += plugin link_pkgconfig
PKGCONFIG += qofono-qt5
QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-psabi

QT_VERSION=$$[QT_VERSION]

QT += qml dbus
QT -= gui
LIBS += -L../src -lqofonoext

INCLUDEPATH += ../src

SOURCES = \
    qofonoextdeclarativeplugin.cpp \
    qofonoextmodemlistmodel.cpp \
    qofonoextsimlistmodel.cpp

HEADERS = \
    qofonoextdeclarativeplugin.h \
    qofonoextmodemlistmodel.h \
    qofonoextsimlistmodel.h

OTHER_FILES += qmldir plugins.qmltypes

target.path = $$[QT_INSTALL_QML]/org/nemomobile/ofono
qmldir.path = $$[QT_INSTALL_QML]/org/nemomobile/ofono
qmldir.files += qmldir plugins.qmltypes

INSTALLS += target qmldir

qmltypes.commands = qmlplugindump -nonrelocatable org.nemomobile.ofono 1.0 > $$PWD/plugins.qmltypes
QMAKE_EXTRA_TARGETS += qmltypes
