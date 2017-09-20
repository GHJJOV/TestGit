win32{
#CONFIG += no_keywords

    INCLUDEPATH += $$PWD/../qtwebdriver/inc/Test
    INCLUDEPATH += $$PWD/../qtwebdriver/inc
    HEADERS += $$PWD/../qtwebdriver/inc/Test/Headers.h
    LIBS += "user32.lib" "ws2_32.lib" "shell32.lib" "advapi32.lib" "psapi.lib" "kernel32.lib" "userenv.lib" "ws2_32.lib" "$$PWD\..\qtwebdriver\lib\chromium_base.lib" "$$PWD\..\qtwebdriver\lib\WebDriver_core.lib" "$$PWD\..\qtwebdriver\lib\WebDriver_extension_qt_base.lib" "$$PWD\..\qtwebdriver\lib\WebDriver_extension_qt_quick.lib" "$$PWD\..\qtwebdriver\lib\test_widgets.lib"

    DEFINES += QT_NO_SAMPLES="1" \
    _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS \
    QT_NO_DEBUG \
    QT_GUI_LIB \
    QT_CORE_LIB \
    QT_SHARED \
#    QT_NO_OPENGL \
    NDEBUG \
    _WIN32 \
    WIN32OS_WIN \
    NOMINMAX \
    _UNICODE \
    _WINSOCKAPI_ \
    OS_WIN

#    QMAKE_CXXFLAGS += -Zc:strictStrings-
#    QMAKE_CXXFLAGS += -Zc:wchar_t-
#    QMAKE_CXXFLAGS_RELEASE += -Zc:strictStrings-
#    QMAKE_CXXFLAGS_RELEASE += -Zc:wchar_t-


}

