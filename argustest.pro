TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += c++17

INCLUDEPATH += /usr/src/jetson_multimedia_api/argus/include/
INCLUDEPATH += /usr/include/opencv4

LIBS += -L"/usr/lib/aarch64-linux-gnu/tegra/"

message($$QMAKE_HOST.os)

LIBS += -lnvargus -lopencv_core -lopencv_highgui -lopencv_imgcodecs

SOURCES += main.cpp
