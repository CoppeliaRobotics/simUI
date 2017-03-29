QT += core gui widgets printsupport

TARGET = v_repExtCustomUI
TEMPLATE = lib

DEFINES -= UNICODE
DEFINES += QT_COMPIL
CONFIG += shared debug_and_release
INCLUDEPATH += "../include"
INCLUDEPATH += "external/QCustomPlot-2.0.0-beta"

*-msvc* {
	QMAKE_CXXFLAGS += -O2
	QMAKE_CXXFLAGS += -W3
}
*-g++*|*clang* {
	QMAKE_CXXFLAGS += -O3
	QMAKE_CFLAGS += -O3
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

stubs_h.target = stubs.h
stubs_h.output = stubs.h
stubs_h.input = callbacks.xml
stubs_h.commands = python -m v_repStubsGen -H stubs.h callbacks.xml
QMAKE_EXTRA_TARGETS += stubs_h
PRE_TARGETDEPS += stubs.h

stubs_cpp.target = stubs.cpp
stubs_cpp.output = stubs.cpp
stubs_cpp.input = callbacks.xml
stubs_cpp.commands = python -m v_repStubsGen -C stubs.cpp callbacks.xml
QMAKE_EXTRA_TARGETS += stubs_cpp
PRE_TARGETDEPS += stubs.cpp

reference_html.target = reference.html
reference_html.output = reference.html
reference_html.input = callbacks.xml
reference_html.commands = xsltproc --path \"$$PWD/\" -o reference.html callbacks.xsl callbacks.xml
QMAKE_EXTRA_TARGETS += reference_html
PRE_TARGETDEPS += reference.html

widgets_html.target = widgets.html
widgets_html.output = widgets.html
widgets_html.input = widgets.xml
widgets_html.commands = xsltproc --path \"$$PWD/\" -o widgets.html widgets.xsl widgets.xml
QMAKE_EXTRA_TARGETS += widgets_html
PRE_TARGETDEPS += widgets.html

HEADERS += \
    debug.h \
    signal_spy.h \
    plugin.h \
    v_repExtCustomUI.h \
    ../include/v_repLib.h \
    stubs.h \
    UIFunctions.h \
    UIProxy.h \
    widgets/Button.h \
    widgets/Checkbox.h \
    widgets/Combobox.h \
    widgets/Edit.h \
    widgets/Event.h \
    widgets/Group.h \
    widgets/HSlider.h \
    widgets/Image.h \
    widgets/Label.h \
    widgets/LayoutWidget.h \
    widgets/Plot.h \
    widgets/Radiobutton.h \
    widgets/Slider.h \
    widgets/Spinbox.h \
    widgets/Tabs.h \
    widgets/VSlider.h \
    widgets/Widget.h \
    widgets/Window.h \
    widgets/all.h \
    LuaCallbackFunction.h \
    Proxy.h \
    XMLUtils.h \
    tinyxml2.h \
    external/QCustomPlot-2.0.0-beta/qcustomplot.h

SOURCES += \
    debug.cpp \
    signal_spy.cpp \
    v_repExtCustomUI.cpp \
    ../common/v_repLib.cpp \
    stubs.cpp \
    UIFunctions.cpp \
    UIProxy.cpp \
    widgets/Button.cpp \
    widgets/Checkbox.cpp \
    widgets/Combobox.cpp \
    widgets/Edit.cpp \
    widgets/Event.cpp \
    widgets/Group.cpp \
    widgets/HSlider.cpp \
    widgets/Image.cpp \
    widgets/Label.cpp \
    widgets/LayoutWidget.cpp \
    widgets/Plot.cpp \
    widgets/Radiobutton.cpp \
    widgets/Slider.cpp \
    widgets/Spinbox.cpp \
    widgets/Tabs.cpp \
    widgets/VSlider.cpp \
    widgets/Widget.cpp \
    widgets/Window.cpp \
    LuaCallbackFunction.cpp \
    Proxy.cpp \
    XMLUtils.cpp \
    tinyxml2.cpp \
    external/QCustomPlot-2.0.0-beta/qcustomplot.cpp


