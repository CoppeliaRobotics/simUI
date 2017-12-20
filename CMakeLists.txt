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
if(${VREP_QT3D})
set(WIDGET_SCENE3D true CACHE BOOL "include the scene3d widget (requires V-REP with Qt3D support)")
else()
set(WIDGET_SCENE3D false CACHE BOOL "include the scene3d widget (requires V-REP with Qt3D support)")
endif()
set(WIDGET_SPINBOX true CACHE BOOL "include the spinbox widget")
set(WIDGET_TABLE true CACHE BOOL "include the table widget")
set(WIDGET_TABS true CACHE BOOL "include the tabs widget")
set(WIDGET_TEXTBROWSER true CACHE BOOL "include the textbrowser widget")
set(WIDGET_TREE true CACHE BOOL "include the tree widget")
set(WIDGET_VSLIDER true CACHE BOOL "include the vslider widget")

configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h)

set(SOURCES
    Proxy.cpp
    UIFunctions.cpp
    UIProxy.cpp
    XMLUtils.cpp
    debug.cpp
    tinyxml2.cpp
    v_repExtCustomUI.cpp
    widgets/Event.cpp
    widgets/LayoutWidget.cpp
    widgets/Widget.cpp
    widgets/Window.cpp
    ${CMAKE_BINARY_DIR}/generated/stubs.cpp
    ${VREP_EXPORTED_SOURCES}
)

set(LIBRARIES
    ${Boost_LIBRARIES}
    ${VREP_LIBRARIES}
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
    set(SOURCES ${SOURCES} widgets/Plot.cpp external/QCustomPlot-2.0.0-beta/qcustomplot.cpp)
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

add_library(v_repExtCustomUI SHARED ${SOURCES})
target_link_libraries(v_repExtCustomUI ${LIBRARIES})

install(TARGETS v_repExtCustomUI DESTINATION ${VREP_ROOT})

