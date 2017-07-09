cmake_minimum_required(VERSION 2.8.11)
project(v_repExtCustomUI)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_MACOSX_RPATH 1)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

find_package(Boost REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt53DCore REQUIRED)
find_package(Qt53DRender REQUIRED)
find_package(Qt53DInput REQUIRED)
find_package(Qt53DExtras REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
find_package(VREP REQUIRED)

include_directories(${VREP_INCLUDE})
include_directories(${CMAKE_BINARY_DIR}/generated)
include_directories(${CMAKE_SOURCE_DIR}/external)
include_directories(${CMAKE_SOURCE_DIR}/external/QDataflowCanvas)
include_directories(${CMAKE_SOURCE_DIR}/external/QCustomPlot-2.0.0-beta)
include_directories(${Boost_INCLUDE_DIRS})

vrep_generate_stubs(${CMAKE_BINARY_DIR}/generated XML_FILE ${CMAKE_SOURCE_DIR}/callbacks.xml LUA_FILE ${CMAKE_SOURCE_DIR}/simExtCustomUI.lua)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/generated/widgets.html
    COMMAND xsltproc -o ${CMAKE_BINARY_DIR}/generated/widgets.html ${CMAKE_SOURCE_DIR}/widgets.xsl ${CMAKE_SOURCE_DIR}/widgets.xml
    DEPENDS ${CMAKE_SOURCE_DIR}/widgets.xml ${CMAKE_SOURCE_DIR}/widgets.xsl)

set(ENABLE_SIGNAL_SPY false CACHE BOOL "if Qt private headers are installed, enable this to debug Qt signals")

configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h)

set(SOURCES
    Proxy.cpp
    UIFunctions.cpp
    UIProxy.cpp
    XMLUtils.cpp
    debug.cpp
    tinyxml2.cpp
    v_repExtCustomUI.cpp
    widgets/Button.cpp
    widgets/Checkbox.cpp
    widgets/Combobox.cpp
    widgets/Dataflow.cpp
    widgets/Edit.cpp
    widgets/Event.cpp
    widgets/Group.cpp
    widgets/HSlider.cpp
    widgets/Image.cpp
    widgets/Label.cpp
    widgets/LayoutWidget.cpp
    widgets/Plot.cpp
    widgets/Progressbar.cpp
    widgets/Radiobutton.cpp
    widgets/Scene3D.cpp
    widgets/Slider.cpp
    widgets/Spinbox.cpp
    widgets/Table.cpp
    widgets/Tabs.cpp
    widgets/TextBrowser.cpp
    widgets/Tree.cpp
    widgets/VSlider.cpp
    widgets/Widget.cpp
    widgets/Window.cpp
    external/QCustomPlot-2.0.0-beta/qcustomplot.cpp
    external/QDataflowCanvas/qdataflowcanvas.cpp
    external/QDataflowCanvas/qdataflowmodel.cpp
)
if(ENABLE_SIGNAL_SPY)
    set(SOURCES ${SOURCES} signal_spy.cpp)
endif()
add_library(v_repExtCustomUI SHARED ${SOURCES} ${CMAKE_BINARY_DIR}/generated/stubs.cpp ${VREP_EXPORTED_SOURCES})
target_link_libraries(v_repExtCustomUI ${Boost_LIBRARIES} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport Qt5::3DCore Qt5::3DRender Qt5::3DInput Qt5::3DExtras ${VREP_LIBRARIES})
