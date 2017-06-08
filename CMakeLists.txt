cmake_minimum_required(VERSION 2.8.11)
project(v_repExtCustomUI)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_MACOSX_RPATH 1)

find_package(Boost REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)

if(NOT DEFINED ENV{VREP_ROOT})
    message(FATAL_ERROR "Environment variable VREP_ROOT is not set")
endif()
file(TO_CMAKE_PATH "$ENV{VREP_ROOT}" VREP_ROOT)

if(WIN32)
    add_definitions(-DWIN_VREP)
    add_definitions(-DNOMINMAX)
    add_definitions(-Dstrcasecmp=_stricmp)
elseif(UNIX AND NOT APPLE)
    add_definitions(-DLIN_VREP)
elseif(UNIX AND APPLE)
    add_definitions(-DMAC_VREP)
endif()

include_directories(${VREP_ROOT}/programming/include)
include_directories(${CMAKE_BINARY_DIR}/generated)
include_directories(${CMAKE_SOURCE_DIR}/external)
include_directories(${CMAKE_SOURCE_DIR}/external/QDataflowCanvas)
include_directories(${CMAKE_SOURCE_DIR}/external/QCustomPlot-2.0.0-beta)
include_directories(${Boost_INCLUDE_DIRS})

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/generated/stubs.cpp ${CMAKE_BINARY_DIR}/generated/stubs.h ${CMAKE_BINARY_DIR}/generated/lua_calltips.cpp
    COMMAND python ${CMAKE_SOURCE_DIR}/external/v_repStubsGen/generate.py --xml-file ${CMAKE_SOURCE_DIR}/callbacks.xml --lua-file ${CMAKE_SOURCE_DIR}/simExtCustomUI.lua --gen-all ${CMAKE_BINARY_DIR}/generated
    DEPENDS ${CMAKE_SOURCE_DIR}/callbacks.xml)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/generated/widgets.html
    COMMAND xsltproc -o ${CMAKE_BINARY_DIR}/generated/widgets.html ${CMAKE_SOURCE_DIR}/widgets.xsl ${CMAKE_SOURCE_DIR}/widgets.xml
    DEPENDS ${CMAKE_SOURCE_DIR}/widgets.xml ${CMAKE_SOURCE_DIR}/widgets.xsl)

set(SOURCES
    Proxy.cpp
    UIFunctions.cpp
    UIProxy.cpp
    XMLUtils.cpp
    debug.cpp
    signal_spy.cpp
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
add_library(v_repExtCustomUI SHARED ${SOURCES} ${CMAKE_BINARY_DIR}/generated/stubs.cpp "${VREP_ROOT}/programming/common/v_repLib.cpp")

if(WIN32)
    set(SHLWAPI shlwapi)
else()
    set(SHLWAPI)
endif()

target_link_libraries(v_repExtCustomUI ${Boost_LIBRARIES} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport ${SHLWAPI})
