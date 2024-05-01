cmake_minimum_required(VERSION 3.16.3)
project(simUI)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_MACOSX_RPATH 1)

set(Qt Qt5 CACHE STRING "Qt version to use (e.g. Qt5)")
set_property(CACHE Qt PROPERTY STRINGS Qt5 Qt6)  #

find_package(Boost REQUIRED)
find_package(${Qt} COMPONENTS Core Gui Widgets PrintSupport REQUIRED)

if(NOT COPPELIASIM_INCLUDE_DIR)
    if(DEFINED ENV{COPPELIASIM_ROOT_DIR})
        set(COPPELIASIM_INCLUDE_DIR $ENV{COPPELIASIM_ROOT_DIR}/programming/include)
    else()
        set(COPPELIASIM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)
    endif()
endif()
list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${COPPELIASIM_INCLUDE_DIR}/cmake)
find_package(CoppeliaSim 4.5.1.7 REQUIRED)

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

include_directories(sourceCode)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/QCustomPlot-2.0.1)
add_definitions(-DQT_COMPIL)

coppeliasim_generate_stubs(${CMAKE_CURRENT_BINARY_DIR}/generated XML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/sourceCode/callbacks.xml LUA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/lua/simUI.lua)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated/simUI-widgets.htm
    COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/generated/simUI-widgets.htm ${CMAKE_CURRENT_SOURCE_DIR}/sourceCode/widgets.xsl ${CMAKE_CURRENT_SOURCE_DIR}/sourceCode/widgets.xml
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sourceCode/widgets.xml ${CMAKE_CURRENT_SOURCE_DIR}/sourceCode/widgets.xsl)
add_custom_target(generate_widgets_html ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/generated/simUI-widgets.htm)

set(ENABLE_SIGNAL_SPY false CACHE BOOL "if Qt private headers are installed, enable this to debug Qt signals")
set(WIDGET_BUTTON true CACHE BOOL "include the button widget")
set(WIDGET_CHECKBOX true CACHE BOOL "include the checkbox widget")
set(WIDGET_COMBOBOX true CACHE BOOL "include the combobox widget")
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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sourceCode/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(SOURCES
    sourceCode/Proxy.cpp
    sourceCode/SIM.cpp
    sourceCode/UI.cpp
    sourceCode/XMLUtils.cpp
    sourceCode/tinyxml2.cpp
    sourceCode/plugin.cpp
    sourceCode/c.cpp
    sourceCode/widgets/Event.cpp
    sourceCode/widgets/LayoutWidget.cpp
    sourceCode/widgets/Widget.cpp
    sourceCode/widgets/Window.cpp
)

set(LIBRARIES
    Boost::boost
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::PrintSupport
)

if(WIDGET_BUTTON)
    list(APPEND SOURCES sourceCode/widgets/Button.cpp)
endif()
if(WIDGET_CHECKBOX)
    list(APPEND SOURCES sourceCode/widgets/Checkbox.cpp)
endif()
if(WIDGET_COMBOBOX)
    list(APPEND SOURCES sourceCode/widgets/Combobox.cpp)
endif()
if(WIDGET_EDIT)
    list(APPEND SOURCES sourceCode/widgets/Edit.cpp)
endif()
if(WIDGET_GROUP)
    list(APPEND SOURCES sourceCode/widgets/Group.cpp)
endif()
if(WIDGET_HSLIDER)
    list(APPEND SOURCES sourceCode/widgets/HSlider.cpp sourceCode/widgets/Slider.cpp)
endif()
if(WIDGET_IMAGE)
    list(APPEND SOURCES sourceCode/widgets/Image.cpp)
endif()
if(WIDGET_LABEL)
    list(APPEND SOURCES sourceCode/widgets/Label.cpp)
endif()
if(WIDGET_PLOT)
    list(APPEND SOURCES sourceCode/widgets/Plot.cpp external/QCustomPlot-2.0.1/qcustomplot.cpp)
endif()
if(WIDGET_PROGRESSBAR)
    list(APPEND SOURCES sourceCode/widgets/Progressbar.cpp)
endif()
if(WIDGET_RADIOBUTTON)
    list(APPEND SOURCES sourceCode/widgets/Radiobutton.cpp)
endif()
if(WIDGET_SCENE3D)
    find_package(${Qt} COMPONENTS 3DCore 3DRender 3DInput 3DExtras REQUIRED)
    list(APPEND SOURCES sourceCode/widgets/Scene3D.cpp)
    list(APPEND LIBRARIES Qt::3DCore Qt::3DRender Qt::3DInput Qt::3DExtras)
endif()
if(WIDGET_SPINBOX)
    list(APPEND SOURCES external/XSpinBox/xspinbox.cpp sourceCode/widgets/Spinbox.cpp)
endif()
if(WIDGET_SVG)
    find_package(${Qt} COMPONENTS Svg REQUIRED)
    list(APPEND SOURCES sourceCode/widgets/SVG.cpp)
    list(APPEND LIBRARIES Qt::Svg)
endif()
if(WIDGET_TABLE)
    list(APPEND SOURCES sourceCode/widgets/Table.cpp)
endif()
if(WIDGET_TABS)
    list(APPEND SOURCES sourceCode/widgets/Tabs.cpp)
endif()
if(WIDGET_TEXTBROWSER)
    list(APPEND SOURCES sourceCode/widgets/TextBrowser.cpp)
endif()
if(WIDGET_TREE)
    list(APPEND SOURCES sourceCode/widgets/Tree.cpp)
endif()
if(WIDGET_VSLIDER)
    list(APPEND SOURCES sourceCode/widgets/VSlider.cpp sourceCode/widgets/Slider.cpp)
endif()

if(ENABLE_SIGNAL_SPY)
    list(APPEND SOURCES sourceCode/signal_spy.cpp)
endif()

coppeliasim_add_plugin(simUI SOURCES ${SOURCES})
target_link_libraries(simUI PRIVATE ${LIBRARIES})
coppeliasim_add_manual(${CMAKE_CURRENT_BINARY_DIR}/generated/simUI-widgets.htm)
