TEMPLATE = app
QT += qml av avwidgets quick multimedia svg sensors websockets concurrent core quickcontrols2 webview widgets

# Build Type
BUILD_TYPE=Debug
CONFIG(release, debug|release) {
    BUILD_TYPE=Release
}
else {
    android{
        DEFINES += _DEBUG
    }
    else{
        DEFINES += _DEBUG _GLIBCXX_DEBUG=1
    }
}

#Platform
BUILD_PLATFORM=linux
win32: BUILD_PLATFORM=windows
android: BUILD_PLATFORM=android
message("Configuring Montage" $$BUILD_PLATFORM $$BUILD_TYPE)

INCLUDEPATH += src
INCLUDEPATH += $$PWD
INCLUDEPATH += $$(QTDIR)/include/QtPlatformSupport/$$QT_VERSION

SOURCES +=  src/main.cpp \
            src/signaling.cpp \
            src/xmppmessenger.cpp \
            src/settings.cpp \
            src/launcher.cpp \
            src/tea.cpp \
            src/filereader.cpp \
            src/fileio.cpp \
            src/signalingfactory.cpp \
            src/signalingprocessing.cpp \
            src/qinputmethodeventmanager.cpp \
            src/workerexecutor.cpp \
            src/updatechecker.cpp \
            src/apiclient.cpp \
            src/hardwareidentifier.cpp \
            src/activationchecker.cpp \
            src/qmlactivationchecker.cpp \
            src/appconfiguration.cpp \
            apsettings.cpp \
            src/networkhelper.cpp \
            src/standardpaths.cpp \
            src/proxyutils.cpp \
            src/networkaccessmanagerfactory.cpp \
            src/networkaccessmanager.cpp \
            src/modules/eventfilter/nativeeventfiltermanager.cpp \
            src/meetingidfactory.cpp \
            src/modules/servicechecker/servicecheckermanager.cpp \
	    src/media/filemedia.cpp \
	    src/media/media.cpp \
	    src/media/videofile.cpp \
	    src/media/webmedia.cpp \
	    src/media/medialist.cpp \
    src/media/mediaparser.cpp \
    filesystemmodel.cpp


HEADERS += \
    src/signaling.h \
    src/settings.h \
    src/launcher.h \
    src/dsh.h \
    src/tea.h \
    src/filereader.h \
    src/fileio.h \
    src/signalingfactory.h \
    src/signalingprocessing.h \
    src/qinputmethodeventmanager.h \
    src/workerexecutor.h \
    src/updatechecker.h \
    src/apiclient.h \
    src/xmppmessenger.h \
    src/hardwareidentifier.h \
    src/activationchecker.h \
    src/qmlactivationchecker.h \
    src/appconfiguration.h \
    src/appconfigurationitems.h \
    apsettings.h \
    src/standardpaths.h \
    src/networkhelper.h \
    src/proxyutils.h \
    src/networkaccessmanagerfactory.h \
    src/networkaccessmanager.h \
    src/modules/eventfilter/nativeeventfilter.h \
    src/modules/eventfilter/nativeeventfiltermanager.h \
    src/meetingidfactory.h \
    src/modules/servicechecker/servicechecker.h \
    src/modules/servicechecker/servicecheckermanager.h \
    src/media/filemedia.h \
    src/media/media.h \
    src/media/videofile.h \
    src/media/webmedia.h \
    src/media/medialist.h \
    src/media/mediaparser.h \
    filesystemmodel.h

RESOURCES += qml.qrc

NETWORKMONITOR_PATH = $$PWD/networkmonitor

# Set WebRTC path and library path ( decide if having different path per platform ).
WEBRTC_PATH = $$PWD/../webrtc/src
WEBRTC_LIB=$$WEBRTC_PATH/out-$$BUILD_PLATFORM/$$BUILD_TYPE

# Set qxmpp path
QXMPP_PATH = $$PWD/qxmpp
INCLUDEPATH += $$QXMPP_PATH/src/client
INCLUDEPATH += $$QXMPP_PATH/src/server
INCLUDEPATH += $$QXMPP_PATH/src/base

CONFIG += no_keywords
CONFIG += Montage
CONFIG += c++11
CONFIG += link_pkgconfig

QSG_RENDER_LOOP=threaded

TARGET = Montage

linux-g++ {
    DEFINES += __LINUX__

    #By default Android and Linux builds are receivers
    CONFIG += RECEIVER

    SOURCES += bonjourservicemanager.cpp \
               src/udevusb.cpp \
               src/modules/eventfilter/nativeeventfilterlinux.cpp \
               src/modules/servicechecker/servicecheckerlinux.cpp


    HEADERS += bonjourservicemanager.h \
               src/udevusb.h \
               src/modules/eventfilter/nativeeventfilterlinux.h \
               src/modules/servicechecker/servicecheckerlinux.h

    LIBS    += -ludev

    PKGCONFIG   += libpulse

    QXMPP_LIB=$$PWD/lib/qxmpp/linux/$$BUILD_TYPE

    RESOURCES += linux.qrc

    include(src/webrtc-linux.pri)
    include(src/playmate.pri)
    include(tentacool/tentacool.pri)
    include(conncontrol/conncontrol.pri)
}

win32 {
    DEFINES += __WIN32__
    QT += gui-private

    SOURCES += bonjourservicemanager.cpp \
               src/modules/eventfilter/nativeeventfilterwin.cpp \
               src/modules/servicechecker/servicecheckerwin.cpp
    HEADERS += bonjourservicemanager.h \
               src/modules/eventfilter/nativeeventfilterwin.h \
               src/modules/servicechecker/servicecheckerwin.h

    contains(CONFIG, RECEIVER) {
        message("Entering windows RECEIVER configuration")

        SOURCES += src/licenseapi.cpp \
                   src/licensemanager.cpp

        HEADERS += src/licenseapi.h \
                   src/licensemanager.h
    }else{
        message("Entering windows CLIENT configutation")
        TARGET = Montage-Client
    }

    RC_ICONS += images/montage-icon.ico

    QMAKE_LFLAGS += /SAFESEH:NO
    QMAKE_LFLAGS += /LARGEADDRESSAWARE
    QMAKE_CXXFLAGS -= -Zc:strictStrings
    QMAKE_CXXFLAGS_RELEASE -= -Zc:strictStrings
    QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO -= -Zc:strictStrings
    QMAKE_LFLAGS_WINDOWS += /NODEFAULTLIB:libcmt.lib

    QXMPP_LIB=$$PWD/lib/qxmpp/windows/$$BUILD_TYPE

    include(src/webrtc-windows.pri)
    include(src/playmate.pri)
    include(tentacool/tentacool.pri)
    include(networkmonitor/networkmonitor.pri)

    RESOURCES += windows.qrc

    OTHER_FILES += \
        windows/ssl_cert.pem \
        windows/montagebox_key-nopass.pem
}

android {
    DEFINES += __ANDROID__
    QT += androidextras

    #By default Android and Linux builds are receivers
    CONFIG += RECEIVER

    QXMPP_LIB=$$PWD/lib/qxmpp/android/$$BUILD_TYPE

    include(src/webrtc-android.pri)
    include(src/playmate.pri)

    SOURCES += src/qmlandroidsystemsettings.cpp \
               src/qmlandroidaccesspoint.cpp \
               src/modules/eventfilter/nativeeventfilterandroid.cpp \
               src/modules/servicechecker/servicecheckerandroid.cpp
    HEADERS += src/qmlandroidsystemsettings.h \
               src/qmlandroidaccesspoint.h \
               src/modules/eventfilter/nativeeventfilterandroid.h \
               src/modules/servicechecker/servicecheckerandroid.h

    # Android app template ( AndroidManifest.xml, gradle, etc).
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
}

contains(CONFIG, RECEIVER) {
    message("Entering RECEIVER configuration")

    SOURCES += src/localservermessenger.cpp \
                    src/cipher.cpp \
		    src/trial.cpp \
                    src/systemlimitationconfiguration.cpp \
                    src/settingslockcontroller.cpp

    HEADERS += src/localservermessenger.h \
                    src/cipher.h \
                    src/trial.h \
                    src/systemlimitationconfiguration.h \
                    src/settingslockcontroller.h

    DEFINES += RECEIVER

    CONFIG(debug, debug|release) {
        DESTDIR = debug
        OBJECTS_DIR = debug
        MOC_DIR = debug
    } else {
        DESTDIR = release
        OBJECTS_DIR = release
        MOC_DIR = release
    }
}else{
    message("Entering CLIENT configutation")

    SOURCES += src/localclientmessenger.cpp
    HEADERS += src/localclientmessenger.h

    CONFIG(debug, debug|release) {
        DESTDIR = debug-client
        OBJECTS_DIR = debug-client
        MOC_DIR = debug-client
    } else {
        DESTDIR = release-client
        OBJECTS_DIR = release-client
        MOC_DIR = release-client
    }
}

TRANSLATIONS = locale_en.ts \
               locale_fr.ts \
               locale_es.ts \
               locale_it.ts

CODECFORTR = UTF-8
lupdate_only {
SOURCES = *.qml \
          js/config.js \
          components/*.qml \
          monitor/*.qml \
          network/*.qml \
          types/*.qml \
          views/*.qml
}

# Getting the version
VERSION_NUMBER=$$(VERSION_NUMBER)
!isEmpty(VERSION_NUMBER) {
    DEFINES += VERSION_NUMBER=$$VERSION_NUMBER
}

BUILD_NUMBER=$$(BUILD_NUMBER)
!isEmpty(BUILD_NUMBER) {
    DEFINES += BUILD_NUMBER=$$BUILD_NUMBER
}

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

# xmpp
include(xmpp/xmpp.pri)

# Angus
include(angus/angus.pri)

# BreakPad Libra project not for Android
include(libra/libra.pri)

# WebRTC
include(src/webrtc-core.pri)

#NetworkMonitor
include(networkmonitor/networkmonitor.pri)

#NetworkMonitorNuevo
include(networkmonitor/networkmonitor.pri Nuevo)
