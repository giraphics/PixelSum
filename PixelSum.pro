include($$PWD/MemoryMgmt/MemoryMgmt.pri)
include($$PWD/PixelSum/PixelSum.pri)
include($$PWD/TestCases/TestCases.pri)

INCLUDEPATH += \
    $$PWD/MemoryMgmt \
    $$PWD/PixelSum \
    $$PWD/PixelSum/HelperClasses

SOURCES += \
    $$PWD/main.cpp
