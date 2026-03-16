project(qofonoextdeclarative LANGUAGES CXX)

set(PLUGIN_SOURCES
    qofonoextdeclarativeplugin.cpp
    qofonoextmodemlistmodel.cpp
    qofonoextsimlistmodel.cpp
)

add_library(qofonoextdeclarative SHARED ${PLUGIN_SOURCES})

target_include_directories(qofonoextdeclarative PRIVATE
    ${CMAKE_SOURCE_DIR}/src ${QOFONO_QT_INCLUDE_DIRS}
)

target_link_libraries(qofonoextdeclarative PRIVATE
    ${QTQML_LIB}
    ${QTDBUS_LIB}
    qofonoext${QTVERSION_SUFFIX}
    ${QOFONO_QT_LIBRARIES}
)

# Set the output directory for the plugin and copy the qmldir file 
# to the appropriate location for qmlplugindump to find it.
set_target_properties(qofonoextdeclarative PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/org/nemomobile/ofono
)
configure_file(qmldir ${CMAKE_CURRENT_BINARY_DIR}/org/nemomobile/ofono/qmldir COPYONLY)

# qmlplugindump is deprecated in Qt 6, qmltyperegistrar should be used
# by the build system instead using QML_ELEMENT macros to declare the 
# types in the plugin.
if(QT_MAJOR_VERSION STREQUAL "5")
    find_program(QMLPLUGINDUMP_CMD NAMES qmlplugindump-qt5 qmlplugindump)

    add_custom_target(qmltypes
        COMMAND ${CMAKE_COMMAND} -E env "QML2_IMPORT_PATH=${CMAKE_CURRENT_BINARY_DIR}" ${QMLPLUGINDUMP_CMD} -nonrelocatable org.nemomobile.ofono 1.0 > ${CMAKE_CURRENT_SOURCE_DIR}/plugins.qmltypes
        DEPENDS qofonoextdeclarative
        COMMENT "Generating plugins.qmltypes"
    )
endif()

install(TARGETS qofonoextdeclarative
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/qt${QT_MAJOR_VERSION}/qml/org/nemomobile/ofono
)

install(FILES qmldir plugins.qmltypes
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/qt${QT_MAJOR_VERSION}/qml/org/nemomobile/ofono
)