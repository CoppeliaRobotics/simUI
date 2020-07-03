cmake_minimum_required(VERSION 2.8.11)
project(simExtCustomUI)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_MACOSX_RPATH 1)

find_package(Boost REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt53DCore REQUIRED)
find_package(Qt53DRender REQUIRED)
find_package(Qt53DInput REQUIRED)
find_package(Qt53DExtras REQUIRED)

if(NOT LIBPLUGIN_DIR)
    if(DEFINED ENV{COPPELIASIM_ROOT_DIR})
        set(LIBPLUGIN_DIR $ENV{COPPELIASIM_ROOT_DIR}/programming/libPlugin)
    else()
        set(LIBPLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libPlugin)
    endif()
endif()
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${LIBPLUGIN_DIR}/cmake)
find_package(CoppeliaSim 4.1.0.0 REQUIRED)

include_directories(${COPPELIASIM_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/QDataflowCanvas)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/QCustomPlot-2.0.1)
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DQT_COMPIL)

coppeliasim_generate_stubs(${CMAKE_CURRENT_BINARY_DIR}/generated XML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/callbacks.xml LUA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/simExtCustomUI.lua)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated/widgets.html
    COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/generated/widgets.html ${CMAKE_CURRENT_SOURCE_DIR}/widgets.xsl ${CMAKE_CURRENT_SOURCE_DIR}/widgets.xml
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/widgets.xml ${CMAKE_CURRENT_SOURCE_DIR}/widgets.xsl)

set(ENABLE_SIGNAL_SPY false CACHE BOOL "if Qt private headers are installed, enable this to debug Qt signals")
set(WIDGET_BUTTON true CACHE BOOL "include the button widget")
set(WIDGET_CHECKBOX true CACHE BOOL "include the checkbox widget")
set(WIDGET_COMBOBOX true CACHE BOOL "include the combobox widget")
set(WIDGET_DATAFLOW true CACHE BOOL "include the dataflow widget")
set(WIDGET_EDIT true CACHE BOOL "include the edit widget")
set(WIDGET_GROUP true CACHE BOOL "include the group widget")
set(WIDGET_HSLIDER true CACHE BOOL "include the hslider widget")
set(WIDGET_IMAGE true CACHE BOOL "include the image widget")
set(WIDGET_LABEL true CACHE BOOL "include the label widget")
set(WIDGET_PLOT true CACHE BOOL "include the plot widget")
set(WIDGET_PROGRESSBAR true CACHE BOOL "include the progressbar widget")
set(WIDGET_RADIOBUTTON true CACHE BOOL "include the radiobutton widget")
set(WIDGET_SCENE3D true CACHE BOOL "include the scene3d widget")
set(WIDGET_SPINBOX true CACHE BOOL "include the spinbox widget")
set(WIDGET_TABLE true CACHE BOOL "include the table widget")
set(WIDGET_TABS true CACHE BOOL "include the tabs widget")
set(WIDGET_TEXTBROWSER true CACHE BOOL "include the textbrowser widget")
set(WIDGET_TREE true CACHE BOOL "include the tree widget")
set(WIDGET_VSLIDER true CACHE BOOL "include the vslider widget")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(SOURCES
    Proxy.cpp
    UIFunctions.cpp
    UIProxy.cpp
    XMLUtils.cpp
    tinyxml2.cpp
    plugin.cpp
    c.cpp
    widgets/Event.cpp
    widgets/LayoutWidget.cpp
    widgets/Widget.cpp
    widgets/Window.cpp
    ${COPPELIASIM_EXPORTED_SOURCES}
)

set(LIBRARIES
    ${Boost_LIBRARIES}
    ${COPPELIASIM_LIBRARIES}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::PrintSupport
)

if(WIDGET_BUTTON)
    set(SOURCES ${SOURCES} widgets/Button.cpp)
endif()
if(WIDGET_CHECKBOX)
    set(SOURCES ${SOURCES} widgets/Checkbox.cpp)
endif()
if(WIDGET_COMBOBOX)
    set(SOURCES ${SOURCES} widgets/Combobox.cpp)
endif()
if(WIDGET_DATAFLOW)
    set(SOURCES ${SOURCES} widgets/Dataflow.cpp external/QDataflowCanvas/qdataflowcanvas.cpp external/QDataflowCanvas/qdataflowmodel.cpp)
endif()
if(WIDGET_EDIT)
    set(SOURCES ${SOURCES} widgets/Edit.cpp)
endif()
if(WIDGET_GROUP)
    set(SOURCES ${SOURCES} widgets/Group.cpp)
endif()
if(WIDGET_HSLIDER)
    set(SOURCES ${SOURCES} widgets/HSlider.cpp widgets/Slider.cpp)
endif()
if(WIDGET_IMAGE)
    set(SOURCES ${SOURCES} widgets/Image.cpp)
endif()
if(WIDGET_LABEL)
    set(SOURCES ${SOURCES} widgets/Label.cpp)
endif()
if(WIDGET_PLOT)
    set(SOURCES ${SOURCES} widgets/Plot.cpp external/QCustomPlot-2.0.1/qcustomplot.cpp)
endif()
if(WIDGET_PROGRESSBAR)
    set(SOURCES ${SOURCES} widgets/Progressbar.cpp)
endif()
if(WIDGET_RADIOBUTTON)
    set(SOURCES ${SOURCES} widgets/Radiobutton.cpp)
endif()
if(WIDGET_SCENE3D)
    set(SOURCES ${SOURCES} widgets/Scene3D.cpp)
    set(LIBRARIES ${LIBRARIES} Qt5::3DCore Qt5::3DRender Qt5::3DInput Qt5::3DExtras)
endif()
if(WIDGET_SPINBOX)
    set(SOURCES ${SOURCES} widgets/Spinbox.cpp)
endif()
if(WIDGET_TABLE)
    set(SOURCES ${SOURCES} widgets/Table.cpp)
endif()
if(WIDGET_TABS)
    set(SOURCES ${SOURCES} widgets/Tabs.cpp)
endif()
if(WIDGET_TEXTBROWSER)
    set(SOURCES ${SOURCES} widgets/TextBrowser.cpp)
endif()
if(WIDGET_TREE)
    set(SOURCES ${SOURCES} widgets/Tree.cpp)
endif()
if(WIDGET_VSLIDER)
    set(SOURCES ${SOURCES} widgets/VSlider.cpp widgets/Slider.cpp)
endif()

if(ENABLE_SIGNAL_SPY)
    set(SOURCES ${SOURCES} signal_spy.cpp)
endif()

add_library(simExtCustomUI SHARED ${SOURCES})
target_link_libraries(simExtCustomUI ${LIBRARIES})

install(TARGETS simExtCustomUI DESTINATION ${COPPELIASIM_PLUGINS_DIR})

