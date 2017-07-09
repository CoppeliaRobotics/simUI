QT += core gui widgets printsupport 3dcore 3drender 3dinput 3dextras

TARGET = v_repExtCustomUI
TEMPLATE = lib

DEFINES -= UNICODE
DEFINES += QT_COMPIL
CONFIG += shared debug_and_release
INCLUDEPATH += "../include"
INCLUDEPATH += "external/QCustomPlot-2.0.0-beta"
INCLUDEPATH += "external/QDataflowCanvas"
INCLUDEPATH += "external"
INCLUDEPATH += "generated"

*-msvc* {
	QMAKE_CXXFLAGS += -O2
	QMAKE_CXXFLAGS += -W3
}
*-g++*|*clang* {
        CONFIG(debug, debug|release){
            QMAKE_CXXFLAGS += -g
            QMAKE_CFLAGS += -g
        } else {
            QMAKE_CXXFLAGS += -O3
            QMAKE_CFLAGS += -O3
        }
        QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter
        QMAKE_CXXFLAGS_WARN_OFF = -Wall -Wno-unused-parameter
}

win32 {
    DEFINES += WIN_VREP
    DEFINES += NOMINMAX
    DEFINES += strcasecmp=_stricmp
    INCLUDEPATH += "c:/local/boost_1_62_0"
}

macx {
    QMAKE_MAC_SDK = macosx10.12
    DEFINES += MAC_VREP
    INCLUDEPATH += "/usr/local/Cellar/boost/1.63.0/include/"
}

unix:!macx {
    DEFINES += LIN_VREP
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

gen_all.target = generated/stubs.h
gen_all.output = generated/stubs.h
gen_all.input = callbacks.xml simExtCustomUI.lua
gen_all.commands = python \"$$PWD/external/v_repStubsGen/generate.py\" --xml-file \"$$PWD/callbacks.xml\" --lua-file \"$$PWD/simExtCustomUI.lua\" --gen-all \"$$PWD/generated/\"
QMAKE_EXTRA_TARGETS += gen_all
PRE_TARGETDEPS += generated/stubs.h

widgets_html.target = generated/widgets.html
widgets_html.output = generated/widgets.html
widgets_html.input = widgets.xml
widgets_html.commands = xsltproc --path \"$$PWD/\" -o generated/widgets.html widgets.xsl widgets.xml
QMAKE_EXTRA_TARGETS += widgets_html
PRE_TARGETDEPS += generated/widgets.html

HEADERS += \
    debug.h \
    signal_spy.h \
    plugin.h \
    v_repExtCustomUI.h \
    ../include/v_repLib.h \
    generated/stubs.h \
    UIFunctions.h \
    UIProxy.h \
    widgets/Button.h \
    widgets/Checkbox.h \
    widgets/Combobox.h \
    widgets/Dataflow.h \
    widgets/Edit.h \
    widgets/Event.h \
    widgets/Group.h \
    widgets/HSlider.h \
    widgets/Image.h \
    widgets/Label.h \
    widgets/LayoutWidget.h \
    widgets/Plot.h \
    widgets/Progressbar.h \
    widgets/Radiobutton.h \
    widgets/Scene3D.h \
    widgets/Slider.h \
    widgets/Spinbox.h \
    widgets/Table.h \
    widgets/Tabs.h \
    widgets/TextBrowser.h \
    widgets/Tree.h \
    widgets/VSlider.h \
    widgets/Widget.h \
    widgets/Window.h \
    widgets/all.h \
    LuaCallbackFunction.h \
    Proxy.h \
    XMLUtils.h \
    tinyxml2.h \
    external/QCustomPlot-2.0.0-beta/qcustomplot.h \
    external/QDataflowCanvas/qdataflowmodel.h \
    external/QDataflowCanvas/qdataflowcanvas.h

SOURCES += \
    debug.cpp \
    signal_spy.cpp \
    v_repExtCustomUI.cpp \
    ../common/v_repLib.cpp \
    generated/stubs.cpp \
    UIFunctions.cpp \
    UIProxy.cpp \
    widgets/Button.cpp \
    widgets/Checkbox.cpp \
    widgets/Combobox.cpp \
    widgets/Dataflow.cpp \
    widgets/Edit.cpp \
    widgets/Event.cpp \
    widgets/Group.cpp \
    widgets/HSlider.cpp \
    widgets/Image.cpp \
    widgets/Label.cpp \
    widgets/LayoutWidget.cpp \
    widgets/Plot.cpp \
    widgets/Progressbar.cpp \
    widgets/Radiobutton.cpp \
    widgets/Scene3D.cpp \
    widgets/Slider.cpp \
    widgets/Spinbox.cpp \
    widgets/Table.cpp \
    widgets/Tabs.cpp \
    widgets/TextBrowser.cpp \
    widgets/Tree.cpp \
    widgets/VSlider.cpp \
    widgets/Widget.cpp \
    widgets/Window.cpp \
    LuaCallbackFunction.cpp \
    Proxy.cpp \
    XMLUtils.cpp \
    tinyxml2.cpp \
    external/QCustomPlot-2.0.0-beta/qcustomplot.cpp \
    external/QDataflowCanvas/qdataflowmodel.cpp \
    external/QDataflowCanvas/qdataflowcanvas.cpp


