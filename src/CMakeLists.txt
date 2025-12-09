add_library(
    qofonoext SHARED
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
    qofonoext
    PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_compile_definitions(qofonoext PRIVATE QOFONOEXT_LIBRARY)
target_compile_options(qofonoext PRIVATE -Wno-unused-parameter -Wno-psabi)

target_include_directories(qofonoext PRIVATE ${QOFONO_QT_INCLUDE_DIRS})
target_link_libraries(qofonoext PRIVATE ${QOFONO_QT_LIBRARIES} Qt5::DBus)

### INSTALL ###

# Library
install(TARGETS qofonoext
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Public headers
install(FILES ${PUBLIC_HEADER_FILES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qofonoext
)

# pkg-config file (qmake create_pc)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qofonoext.pc.in ${CMAKE_CURRENT_BINARY_DIR}/qofonoext.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qofonoext.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
