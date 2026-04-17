add_library(
    qofonoext${QTVERSION_SUFFIX} SHARED
    qofonoext.cpp
    qofonoextcell.cpp
    qofonoextcellinfo.cpp
    qofonoextcellwatcher.cpp
    qofonoextmodemmanager.cpp
    qofonoextsiminfo.cpp
)

set(PUBLIC_HEADER_FILES
    qofonoextcell.h
    qofonoextcellinfo.h
    qofonoextcellwatcher.h
    qofonoextmodemmanager.h
    qofonoextsiminfo.h
    qofonoext_types.h
)

set_target_properties(
    qofonoext${QTVERSION_SUFFIX}
    PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_compile_definitions(qofonoext${QTVERSION_SUFFIX} PRIVATE QOFONOEXT_LIBRARY)
target_compile_options(qofonoext${QTVERSION_SUFFIX} PRIVATE -Wno-unused-parameter -Wno-psabi)

target_include_directories(qofonoext${QTVERSION_SUFFIX} PRIVATE ${QOFONO_QT_INCLUDE_DIRS})
target_link_libraries(qofonoext${QTVERSION_SUFFIX} PRIVATE ${QOFONO_QT_LIBRARIES} ${QTDBUS_LIB})

### INSTALL ###

# Library
install(TARGETS qofonoext${QTVERSION_SUFFIX}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Public headers
install(FILES ${PUBLIC_HEADER_FILES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qofonoext${QTVERSION_SUFFIX}
)

include(ECMGeneratePkgConfigFile)
ecm_generate_pkgconfig_file(
    BASE_NAME qofonoext${QTVERSION_SUFFIX}
    INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/qofonoext${QTVERSION_SUFFIX}
    URL https://github.com/sailfishos/libqofonoext
    DESCRIPTION "Qt bindings for ofono extensions"
    INSTALL
)

