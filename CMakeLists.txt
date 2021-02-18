cmake_minimum_required(VERSION 2.8.11)
project(simExtUI)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_MACOSX_RPATH 1)

find_package(Boost REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)

if(NOT LIBPLUGIN_DIR)
    if(DEFINED ENV{COPPELIASIM_ROOT_DIR})
        set(LIBPLUGIN_DIR $ENV{COPPELIASIM_ROOT_DIR}/programming/libPlugin)
    else()
        set(LIBPLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libPlugin)
    endif()
endif()
list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${LIBPLUGIN_DIR}/cmake)
find_package(CoppeliaSim 4.1.0.0 REQUIRED)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/QDataflowCanvas)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/QCustomPlot-2.0.1)
add_definitions(-DQT_COMPIL)

coppeliasim_generate_stubs(${CMAKE_CURRENT_BINARY_DIR}/generated XML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/callbacks.xml LUA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/simUI.lua)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated/simUI-widgets.htm
    COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/generated/simUI-widgets.htm ${CMAKE_CURRENT_SOURCE_DIR}/widgets.xsl ${CMAKE_CURRENT_SOURCE_DIR}/widgets.xml
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/widgets.xml ${CMAKE_CURRENT_SOURCE_DIR}/widgets.xsl)
add_custom_target(generate_widgets_html ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/generated/simUI-widgets.htm)

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
set(WIDGET_SVG true CACHE BOOL "include the svg widget")
set(WIDGET_TABLE true CACHE BOOL "include the table widget")
set(WIDGET_TABS true CACHE BOOL "include the tabs widget")
set(WIDGET_TEXTBROWSER true CACHE BOOL "include the textbrowser widget")
set(WIDGET_TREE true CACHE BOOL "include the tree widget")
set(WIDGET_VSLIDER true CACHE BOOL "include the vslider widget")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(SOURCES
    Proxy.cpp
    SIM.cpp
    UI.cpp
    XMLUtils.cpp
    tinyxml2.cpp
    plugin.cpp
    c.cpp
    widgets/Event.cpp
    widgets/LayoutWidget.cpp
    widgets/Widget.cpp
    widgets/Window.cpp
)

set(LIBRARIES
    Boost::boost
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::PrintSupport
)

if(WIDGET_BUTTON)
    list(APPEND SOURCES widgets/Button.cpp)
endif()
if(WIDGET_CHECKBOX)
    list(APPEND SOURCES widgets/Checkbox.cpp)
endif()
if(WIDGET_COMBOBOX)
    list(APPEND SOURCES widgets/Combobox.cpp)
endif()
if(WIDGET_DATAFLOW)
    list(APPEND SOURCES widgets/Dataflow.cpp external/QDataflowCanvas/qdataflowcanvas.cpp external/QDataflowCanvas/qdataflowmodel.cpp)
endif()
if(WIDGET_EDIT)
    list(APPEND SOURCES widgets/Edit.cpp)
endif()
if(WIDGET_GROUP)
    list(APPEND SOURCES widgets/Group.cpp)
endif()
if(WIDGET_HSLIDER)
    list(APPEND SOURCES widgets/HSlider.cpp widgets/Slider.cpp)
endif()
if(WIDGET_IMAGE)
    list(APPEND SOURCES widgets/Image.cpp)
endif()
if(WIDGET_LABEL)
    list(APPEND SOURCES widgets/Label.cpp)
endif()
if(WIDGET_PLOT)
    list(APPEND SOURCES widgets/Plot.cpp external/QCustomPlot-2.0.1/qcustomplot.cpp)
endif()
if(WIDGET_PROGRESSBAR)
    list(APPEND SOURCES widgets/Progressbar.cpp)
endif()
if(WIDGET_RADIOBUTTON)
    list(APPEND SOURCES widgets/Radiobutton.cpp)
endif()
if(WIDGET_SCENE3D)
    find_package(Qt53DCore REQUIRED)
    find_package(Qt53DRender REQUIRED)
    find_package(Qt53DInput REQUIRED)
    find_package(Qt53DExtras REQUIRED)
    list(APPEND SOURCES widgets/Scene3D.cpp)
    list(APPEND LIBRARIES Qt5::3DCore Qt5::3DRender Qt5::3DInput Qt5::3DExtras)
endif()
if(WIDGET_SPINBOX)
    list(APPEND SOURCES widgets/Spinbox.cpp)
endif()
if(WIDGET_SVG)
    find_package(Qt5Svg REQUIRED)
    list(APPEND SOURCES widgets/SVG.cpp)
    list(APPEND LIBRARIES Qt5::Svg)
endif()
if(WIDGET_TABLE)
    list(APPEND SOURCES widgets/Table.cpp)
endif()
if(WIDGET_TABS)
    list(APPEND SOURCES widgets/Tabs.cpp)
endif()
if(WIDGET_TEXTBROWSER)
    list(APPEND SOURCES widgets/TextBrowser.cpp)
endif()
if(WIDGET_TREE)
    list(APPEND SOURCES widgets/Tree.cpp)
endif()
if(WIDGET_VSLIDER)
    list(APPEND SOURCES widgets/VSlider.cpp widgets/Slider.cpp)
endif()

if(ENABLE_SIGNAL_SPY)
    list(APPEND SOURCES signal_spy.cpp)
endif()

coppeliasim_add_plugin(simExtUI SOURCES ${SOURCES})
target_link_libraries(simExtUI ${LIBRARIES})
coppeliasim_add_helpfile(${CMAKE_CURRENT_BINARY_DIR}/generated/simUI-widgets.htm)
