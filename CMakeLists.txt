cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)

set(LIBQOFONOEXT_VERSION "1.0.32" CACHE STRING "libqofonoext version")
set(QT_MAJOR_VERSION 5 CACHE STRING "Qt major version")

project(libqofonoext
    VERSION ${LIBQOFONOEXT_VERSION}
    DESCRIPTION "Library for accessing Sailfish OS specific ofono extensions"
    HOMEPAGE_URL "https://github.com/sailfishos/libqofonoext"
    LANGUAGES CXX
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)
include(FeatureSummary)
find_package(PkgConfig)

pkg_check_modules(QOFONO_QT REQUIRED qofono-qt${QT_MAJOR_VERSION})

find_package(Qt${QT_MAJOR_VERSION} COMPONENTS DBus Qml)
set_package_properties(Qt${QT_MAJOR_VERSION}
    PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Qt${QT_MAJOR_VERSION} libraries"
    URL "https://www.qt.io"
)
SET(QTDBUS_LIB Qt${QT_MAJOR_VERSION}::DBus)
SET(QTQML_LIB Qt${QT_MAJOR_VERSION}::Qml)

set_package_properties(PkgConfig
    PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Helper tool for compiling applications and libraries"
    URL "https://www.freedesktop.org/wiki/Software/pkg-config/"
)

set_package_properties(QOFONO_QT
    PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Library for accessing the ofono daemon"
    URL "https://github.com/sailfishos/libqofono"
)

feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

if(QT_MAJOR_VERSION STREQUAL "6")
    set(QTVERSION_SUFFIX "-qt6")
else()
    SET(QTVERSION_SUFFIX "")
endif()

add_subdirectory(src)
add_subdirectory(plugin)