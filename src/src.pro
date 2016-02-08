TARGET = qofonoext
CONFIG += create_pc create_prl no_install_prl link_pkgconfig
PKGCONFIG += qofono-qt5

QT += dbus
QT -= gui
QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-psabi

include(version.pri)

TEMPLATE = lib

DEFINES += QOFONOEXT_LIBRARY

isEmpty(PREFIX) {
  PREFIX=/usr
}

XML_FILES += \
    dbus/org.nemomobile.ofono.ModemManager.xml \
    dbus/org.nemomobile.ofono.SimInfo.xml \
    dbus/org.nemomobile.ofono.SimSettings.xml

OTHER_FILES += $$XML_FILES\
    qofonoext.prf \
    version.pri

SOURCES += \
    qofonoextmodemmanager.cpp \
    qofonoextsiminfo.cpp \
    qofonoextsimsettings.cpp

PUBLIC_HEADERS += \
    qofonoextmodemmanager.h \
    qofonoextsiminfo.h \
    qofonoextsimsettings.h \
    qofonoext_types.h

HEADERS += \
    $$PUBLIC_HEADERS \
    qofonoext_p.h

DBUS_INTERFACES += org_nemomobile_ofono_modem_manager
org_nemomobile_ofono_modem_manager.files = dbus/org.nemomobile.ofono.ModemManager.xml
org_nemomobile_ofono_modem_manager.header_flags = -N -c QOfonoExtModemManagerProxy -i qofonoext_p.h
org_nemomobile_ofono_modem_manager.source_flags = -N -c QOfonoExtModemManagerProxy

DBUS_INTERFACES += org_nemomobile_ofono_sim_settings
org_nemomobile_ofono_sim_settings.files = dbus/org.nemomobile.ofono.SimSettings.xml
org_nemomobile_ofono_sim_settings.header_flags = -N -c QOfonoExtSimSettingsProxy
org_nemomobile_ofono_sim_settings.source_flags = -N -c QOfonoExtSimSettingsProxy

DBUS_INTERFACES += org_nemomobile_ofono_sim_info
org_nemomobile_ofono_sim_info.files = dbus/org.nemomobile.ofono.SimInfo.xml
org_nemomobile_ofono_sim_info.header_flags = -N -c QOfonoExtSimInfoProxy
org_nemomobile_ofono_sim_info.source_flags = -N -c QOfonoExtSimInfoProxy

target.path = $$[QT_INSTALL_LIBS]

headers.files = $$PUBLIC_HEADERS
headers.path = $$INSTALL_ROOT$$PREFIX/include/qofonoext

pkgconfig.files = qofonoext.pc
pkgconfig.path = $$[QT_INSTALL_LIBS]/pkgconfig

QMAKE_PKGCONFIG_NAME = $$TARGET
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_INCDIR = $$headers.path
QMAKE_PKGCONFIG_DESCRIPTION = Qt bindings for ofono extensions
QMAKE_PKGCONFIG_PREFIX = $$PREFIX
QMAKE_PKGCONFIG_VERSION = $$VERSION

INSTALLS += target headers pkgconfig
