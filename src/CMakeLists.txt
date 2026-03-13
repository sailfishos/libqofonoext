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

# pkg-config file (qmake create_pc)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qofonoext.pc.in ${CMAKE_CURRENT_BINARY_DIR}/qofonoext${QTVERSION_SUFFIX}.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qofonoext${QTVERSION_SUFFIX}.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
