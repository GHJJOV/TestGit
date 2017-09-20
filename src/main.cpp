#include <QCoreApplication>
#include <QDebug>
#include <QApplication>
#include <QNetworkInterface>
#include <QPluginLoader>
#include <QQmlApplicationEngine>
#include <QQmlExtensionPlugin>
#include <QScreen>
#include <QSurfaceFormat>
#include <QSysInfo>
#include <QtQml>

#include <fstream>
#include <iostream>

#include "apiclient.h"
#include "appconfiguration.h"
#include "apsettings.h"
#include "fileio.h"
#include "filereader.h"
#include "hardwareidentifier.h"
#include "launcher.h"
#include "networkhelper.h"
#include "networkmonitor/qmlnetworkmonitor.h"
#include "proxyutils.h"
#include "qinputmethodeventmanager.h"
#include "qmlactivationchecker.h"
#include "qrtcqquickfborender.h"
#include "qrtcsession.h"
#include "serialnumber.h"
#include "settings.h"
#include "shape.h"
#include "signaling.h"
#include "signalingfactory.h"
#include "standardpaths.h"
#include "systemlimitationconfiguration.h"
#include "tea.h"
#include "updatechecker.h"
#include "whiteboardcanvas.h"
#include "meetingidfactory.h"
#include "media/medialist.h"

#include "filesystemmodel.h"

#ifdef RECEIVER
#include "qrtcairplayvideorenderer.h"
#include "settingslockcontroller.h"
#endif
#include <QtWebView/QtWebView>

#ifdef __WIN32__
    #include <Windows.h>
    #include <Wininet.h>
    #include "webrtc/modules/video_coding/codecs/h264/include/h264.h"
    #ifdef RECEIVER
        #include "licensemanager.h"
        #include "trial.h"
    #endif
    #include <modules/eventfilter/nativeeventfiltermanager.h>
    #include <modules/servicechecker/servicecheckermanager.h>
#endif

#ifdef __ANDROID__
#include <QAndroidJniObject>
#include <QCryptographicHash>
#include <QDateTime>
#include <QtAndroid>
#include <QtAndroidExtras>
#include <jni.h>
#include "dummy_miracast.h"
#include "webrtc/api/android/jni/jni_helpers.h"
#include "webrtc/base/ssladapter.h"
#include "webrtc/api/android/jni/classreferenceholder.h"
#include "webrtc/modules/utility/include/helpers_android.h"
#include "webrtc/modules/utility/include/jvm_android.h"
#include <qmlandroidsystemsettings.h>
#include <qmlandroidaccesspoint.h>
const static QString passPhrase = "/+1983+/";
const static qint32 max = 30;
#endif

#ifndef __ANDROID__
#include "GenericCrashHandler.h" // Libra include
#endif

#if defined(__LINUX__) && !defined(__ANDROID__)
#include "networkinformation.h"
#include "qmlconnmanager.h"
#include "technologyinformation.h"
#include "udevusb.h"
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifndef VERSION_NUMBER
#define VERSION_NUMBER_STR "debug"
#else
#define VERSION_NUMBER_STR TOSTRING(VERSION_NUMBER)
#endif

#ifndef BUILD_NUMBER
#define BUILD_NUMBER_STR "debug"
#else
#define BUILD_NUMBER_STR TOSTRING(BUILD_NUMBER)
#endif

#define STATSDIR "_stats"

QString log_name = "";
void tracesOutputHandler(QtMsgType type, const QMessageLogContext& context, const QString & msg)
{
    Q_UNUSED(context)
    static std::ofstream ofstr(log_name.toStdString(), std::ios_base::trunc);
    QByteArray localMsg = msg.toLocal8Bit();

    ofstr << QDateTime::currentDateTime().toString("[dd/MM/yyyy hh:mm:ss::zzz] ").toStdString();

    switch (type) {
    case QtDebugMsg:
        ofstr << "Debug: " << localMsg.constData() << std::endl;
        break;
        /*case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;*/
    case QtWarningMsg:
        ofstr << "Warning: " << localMsg.constData() << std::endl;
        break;
    case QtCriticalMsg:
        ofstr << "Critical: " << localMsg.constData() << std::endl;
        break;
    case QtFatalMsg:
        ofstr << "Error: " << localMsg.constData() << std::endl;
    }
}
#ifdef __ANDROID__
static void onSettingsWritePermissionResult(JNIEnv * /*env*/, jobject thiz, jboolean result)
{
    qDebug()<< Q_FUNC_INFO <<result;
    // Here we can emit or send to Qml layer if we need the result of permission request.
}
static JNINativeMethod methods[] = {
    {"onSettingsWritePermissionResult", "(Z)V", (void *)onSettingsWritePermissionResult}
};

extern "C" jint JNIEXPORT JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    // Just to init the stuff of configuration, as hamlet needs it pre-filled.
    AppConfiguration::getInstance();

    // Call hamlet to init
    QAndroidJniObject activity = QtAndroid::androidActivity();
    QAndroidJniObject context = activity.callObjectMethod("getApplicationContext","()Landroid/content/Context;");
    QAndroidJniObject::callStaticMethod<void>("com.displaynote.hamlet.Hamlet", "init", "(Landroid/content/Context;)V", context.object<jobject>());

    // Set WebRTC stuff that depends on JNI
    QAndroidJniEnvironment env;
    webrtc_jni::InitGlobalJniVariables(env.javaVM()); // For most of WebRTC calls.
    webrtc::JVM::Initialize(env.javaVM(),context.object<jobject>()); // AudioEngine uses this instead.
    rtc::InitializeSSL();
    webrtc_jni::LoadGlobalClassReferenceHolder();

    // search for Java class which declares the native methods
    jclass javaClass = env->FindClass("com/displaynote/montage/MontageActivity");
    if (!javaClass)
      return JNI_ERR;

    // register our native methods
    if (env->RegisterNatives(javaClass, methods,
                          sizeof(methods) / sizeof(methods[0])) < 0) {
      return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

extern "C" void JNIEXPORT JNICALL JNI_OnUnLoad(JavaVM *jvm, void *reserved)
{
    // Free WebRTC stuff
    webrtc_jni::FreeGlobalClassReferenceHolder();
    webrtc::JVM::Uninitialize();
    rtc::CleanupSSL();
}

int platformMain(QGuiApplication &app, QQmlApplicationEngine &engine)
{
    qmlRegisterType<QObject>("ConnManager", 1, 0,"FakeConnManager");
    engine.rootContext()->setContextProperty("udev", NULL);
    return 0;
}

QString writableLocation()
{
    return StandardPaths::writableLocation(QStandardPaths::DownloadLocation);
}

qreal dpi(QGuiApplication &app)
{
    QAndroidJniObject qtActivity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative", "activity", "()Landroid/app/Activity;");
    QAndroidJniObject resources = qtActivity.callObjectMethod("getResources", "()Landroid/content/res/Resources;");
    QAndroidJniObject displayMetrics = resources.callObjectMethod("getDisplayMetrics", "()Landroid/util/DisplayMetrics;");
    return displayMetrics.getField<int>("densityDpi");
}

QString version()
{
    return QString("v") + QString(VERSION_NUMBER_STR) + QString(".") + QString(BUILD_NUMBER_STR);
}

bool isAtheros(Launcher &launcher)
{
    return false;
}

QString hostname(Settings * montageSettings)
{
    QVariant hostName = montageSettings->value("hostname");
    if(!hostName.isNull())
        return hostName.toString();
    return AppConfiguration::getInstance()->hostname();
}
#endif

#ifdef __WIN32__

//shared memory to know if Montage is running
QSharedMemory montageRunningChecker("Montage");

bool isRunning(const QString &process, const qint64 appId)
{
    return !montageRunningChecker.create(1);
}

int platformMain(QGuiApplication &app, QQmlApplicationEngine &engine)
{
    // TODO: Uncomment when we can use ANGLE with D3D11 (in QTAV we need to load the submodule capi)
    // See:
    //      https://bugreports.qt.io/browse/QTBUG-54905
    //      https://codereview.qt-project.org/#/c/169614/
    //      https://github.com/Microsoft/angle/issues/70
//    QGuiApplication::setAttribute(Qt::AA_UseOpenGLES);

    QString exeName = AppConfiguration::getInstance()->appName() + ".exe";
    if (montageRunningChecker.attach() || isRunning(exeName, app.applicationPid()))
    {
        qDebug() << AppConfiguration::getInstance()->appName() + " is running. You can not have two instance of " + AppConfiguration::getInstance()->appName() + " running";
        MessageBox(NULL, QString(AppConfiguration::getInstance()->appName() + " is already running.").toStdWString().c_str(),
                   AppConfiguration::getInstance()->appName().toStdWString().c_str(), MB_OK|MB_ICONWARNING);
        app.exit();
        return -1;
    }
    qDebug() << "HW Acceleration supported: " << webrtc::H264Encoder::IsSupported();

    qmlRegisterType<QObject>("ConnManager", 1, 0,"FakeConnManager");

    if(QSysInfo::windowsVersion() == QSysInfo::WV_WINDOWS7) {
        engine.rootContext()->setContextProperty("enableAutoAD", false);
    }
    engine.rootContext()->setContextProperty("udev", NULL);

    return 0;
}

QString writableLocation()
{
    return StandardPaths::writableLocation(QStandardPaths::DownloadLocation);
}

qreal dpi(QGuiApplication &app)
{
    QScreen *screen = app.screens().at(0);
    return screen->logicalDotsPerInch() * app.devicePixelRatio();
}

QString version()
{
    return QString("v") + QString(VERSION_NUMBER_STR) + QString(".") + QString(BUILD_NUMBER_STR);
}

bool isAtheros(Launcher &launcher)
{
    return false;
}

QString hostname(Settings * montageSettings)
{
    QVariant hostName = montageSettings->value("hostname");
    qDebug() << "hostname: " << hostName;
    if(!hostName.isNull())
        return hostName.toString();
    return AppConfiguration::getInstance()->hostname();
}
#endif

#if defined(__LINUX__) && !defined(__ANDROID__)
QMap<QString, QString> getMACs()
{
    QMap<QString, QString> map;
    Q_FOREACH(QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
    {
        // Return only the valid, active, run and non-loopback MAC Address
        if (!(netInterface.flags() & QNetworkInterface::IsLoopBack))
        {
            qDebug() << "Interface" << netInterface.name() << "has the MAC" << netInterface.hardwareAddress();
            map.insert(netInterface.name(), netInterface.hardwareAddress());
        }
    }
    return map;
}

int platformMain(QGuiApplication &app, QQmlApplicationEngine &engine)
{
    qmlRegisterSingletonType<QmlConnManager>("ConnManager", 1, 0, "ConnManager", instanceOfQmlConnManager);
    qmlRegisterType<NetworkInformation>("ConnManager", 1, 0,"NetworkInformation");
    qmlRegisterType<TechnologyInformation>("ConnManager", 1, 0, "TechnologyInformation");

    // Getting MAC address
    QMap<QString, QString> macs = getMACs();
    engine.rootContext()->setContextProperty("eth0", macs["eth0"]);
    engine.rootContext()->setContextProperty("wlan0", macs["wlan0"]);

    return 0;
}

QString writableLocation()
{
    return "/Downloads";
}

qreal dpi(QGuiApplication &app)
{
    QScreen *screen = app.screens().at(0);
    return screen->physicalDotsPerInch() * app.devicePixelRatio();
}

QString version()
{
    return QString(VERSION_NUMBER_STR) + QString("-") + QString(BUILD_NUMBER_STR);
}

bool isAtheros(Launcher &launcher)
{
    return launcher.isAtheros();
}

QString hostname(Settings * montageSettings)
{
    QVariant hostName = montageSettings->value("hostname");
    if(!hostName.isNull())
        return hostName.toString();
    return AppConfiguration::getInstance()->hostname();
}
#endif

int main(int argc, char *argv[])
{
    QThread::currentThread()->setPriority(QThread::TimeCriticalPriority);

    QDir log_dir(StandardPaths::montageAppDataLocation());
    if (!log_dir.exists("log"))
    {
        log_dir.mkpath("log");
    }
    QString log_path = log_dir.absoluteFilePath("log");
    log_name = log_path + "/montage" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss.log");

#ifndef _DEBUG
    // Start traces
    qInstallMessageHandler(tracesOutputHandler);
#ifndef __ANDROID__
    // Start Libra
    GenericCrashManager::GenericCrashHandler::instance()->init(log_path);
#endif
#endif
    qDebug() << "Plugins path:" << QLibraryInfo::location(QLibraryInfo::PluginsPath);

    QString gateway = NetworkHelper::DefaultGateway();
    qDebug() << "Gateway:" << gateway;

    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    QtWebView::initialize();

    // Set application manufacturer data
    QCoreApplication::setOrganizationName("DisplayNoteTechnologiesLtd");
    QCoreApplication::setOrganizationDomain("displaynote.com");
    QCoreApplication::setApplicationName("Montage");

    // NativeEventFilterManager is prepared but not implemented in Linux and Android.
    // At the moment just for use in Windows platform and for that is that define.
#ifdef __WIN32__
    NativeEventFilterManager nativeEventFilterManager;
    NativeEventFilter * nativeEventFilter = nativeEventFilterManager.CreateNativeEventFilter();

    app.installNativeEventFilter(nativeEventFilter);

    engine.rootContext()->setContextProperty("NativeEventFilter", nativeEventFilter);
#endif

    // ServiceCheckerManager is prepared but not implemented in Linux and Android.
    // At the moment just for use in Windows platform and for that is that define.
#ifdef __WIN32__
    ServiceCheckerManager serviceCheckerManager;
    ServiceChecker * serviceChecker = serviceCheckerManager.CreateServiceChecker();

    engine.rootContext()->setContextProperty("ServiceChecker", serviceChecker);
#endif

    // Setup proxy at start
    ProxyUtils proxyUtils;
    proxyUtils.applyProxySettings();

    ActivationChecker *activationChecker = new ActivationChecker(); //Will be destroyed on QmlActivationChecker destructor
    AppConfiguration::getInstance()->setActivationChecker(activationChecker);

    qmlRegisterType<UpdateChecker>("Updater", 1, 0, "UpdateChecker");
#ifdef RECEIVER
    qmlRegisterType<LocalServerMessenger>("Signaling", 1, 0, "LocalServerMessenger");
    qmlRegisterType<QRTCAirplayVideoRenderer>("WebRTC", 1, 0, "QRTCAirplayVideoRenderer");
    qmlRegisterType<Airplay>("Playmate",1,0, "Airplay");
    qmlRegisterType<PlaybackInfo>("Playmate", 1, 0, "PlaybackInfo");
    qmlRegisterType<Miracast>("Playmate",1,0, "Miracast");
#else
    qmlRegisterType<LocalClientMessenger>("Signaling", 1, 0, "LocalClientMessenger");
#endif
    qmlRegisterSingletonType<APSettings>("Settings",1,0,"APSettings", instanceOfAPSettings);
    qmlRegisterType<QRTCVideoCapturer>("WebRTC", 1, 0, "QRTCVideoCapturer");
    qmlRegisterType<QRTCVideoRenderer>("WebRTC", 1, 0, "QRTCVideoRenderer");
    qmlRegisterType<QRTCSession>("WebRTC", 1, 0, "QRTCSession");
    qmlRegisterType<QRTCQQuickFBORenderer>("WebRTC",1,0, "QRTCQQuickFBORender");
    qmlRegisterType<QRTCDevice>("WebRTC", 1, 0, "QRTCDevice");
    qmlRegisterType<QRTCStatsReport>("WebRTC", 1, 0, "QRTCStatsReport");
    qmlRegisterType<TEA>("TEA",1,0,"TEA");
    qmlRegisterType<Shape>("Angus", 1, 0, "Shape");
    qmlRegisterType<WhiteboardCanvas>("Angus", 1, 0, "WhiteboardCanvas");
    qmlRegisterUncreatableType<AngusTypes>("Angus", 1, 0, "AngusTypes", "");
    qmlRegisterType<FileIO, 1>("FileIO", 1, 0, "FileIO");
    qmlRegisterType<APIClient>("APIClient", 1, 0, "APIClient");
    qmlRegisterType<QInputMethodEventManager>("Input", 1, 0, "InputMethodEventManager");
    qmlRegisterSingletonType<QmlNetworkMonitor>("NetworkMonitor", 1, 0, "NetworkMonitor", instanceOfQmlNetworkMonitor);
    qmlRegisterUncreatableType<ProxyUtils>("ProxyUtils", 1, 0, "ProxyUtils", "");
    qmlRegisterType<MeetingIDFactory>("MeetingIDFactory",1,0,"MeetingIDFactory");
    qmlRegisterType<montage::media::Media>("Media", 1, 0, "Media");
    qRegisterMetaType<montage::media::MediaPtr>("MediaPtr");
    qRegisterMetaType<montage::media::MediaPtrList>("MediaPtrList");
#ifdef __ANDROID__
    engine.rootContext()->setContextProperty("AndroidSettings", new QmlAndroidSystemSettings());
    engine.rootContext()->setContextProperty("AndroidAccessPoint", new QmlAndroidAccessPoint());
#endif

#ifdef RECEIVER
    SettingsLockController * settingsLockController = new SettingsLockController(AppConfiguration::getInstance());
    engine.rootContext()->setContextProperty("SettingsLockController", settingsLockController);

    SystemLimitationConfiguration systemLimitationConfiguration(AppConfiguration::getInstance());
    engine.rootContext()->setContextProperty("SystemLimitationConfiguration", &systemLimitationConfiguration);

#ifdef __WIN32__
    Trial trial(AppConfiguration::getInstance());
    engine.rootContext()->setContextProperty("trial", &trial);

    LicenseManager licenseManager(&systemLimitationConfiguration, settingsLockController);
    engine.rootContext()->setContextProperty("licenseManager", &licenseManager);
#endif
#endif

    QDir downloadPath(writableLocation());
    qDebug() << "downloadPath.absolutePath()" << downloadPath.absolutePath();
    engine.rootContext()->setContextProperty("download_path", downloadPath.absolutePath());
#ifdef RECEIVER
    QDir media_dir(StandardPaths::montageAppDataLocation());
    if (!media_dir.exists("media"))
    {
        media_dir.mkpath("media");
    }
    QString media_path = media_dir.absoluteFilePath("media");
#else
    QString media_path = downloadPath.absolutePath();
#endif
    engine.rootContext()->setContextProperty("media_path", media_path);
    qmlRegisterType<FileReader>("FileIO", 1, 0, "FileReader");
    qRegisterMetaType<FileReader::open_mode>("OpenMode");

#ifndef __ANDROID__
    // Real time stats folder
    QString statsDirectory(log_name);
    statsDirectory.replace(".log", STATSDIR);
    qDebug() << Q_FUNC_INFO << statsDirectory;
    engine.rootContext()->setContextProperty("statsDirectory", statsDirectory);

    rtc::InitializeSSL();
#endif
    qDebug() << "buildAbi" << QSysInfo::buildAbi();
    qDebug() << "buildCpuArchitecture" << QSysInfo::buildCpuArchitecture();
    qDebug() << "currentCpuArchitecture" << QSysInfo::currentCpuArchitecture();
    qDebug() << "kernelType" << QSysInfo::kernelType();
    qDebug() << "kernelVersion" << QSysInfo::kernelVersion();

    // We need to create and init the PeerConnectionFactory here for get initialized in QML layer
    QRTCPeerConnectionFactory* pcf = instanceOfQRTCPeerConnectionFactory(&engine);
    qmlRegisterUncreatableType<QRTCPeerConnectionFactory>("WebRTC", 1, 0, "PeerConnectionFactory", "C++ Singleton");
    engine.rootContext()->setContextProperty("peerConnectionFactory", pcf);

    qmlRegisterSingletonType<SignalingFactory>("Signaling", 1, 0, "SignalingFactory", instanceOfSignalingFactory);

    Settings * montageSettings = new Settings();

    engine.rootContext()->setContextProperty("version_number", version());
    engine.rootContext()->setContextProperty("MontageSettings", montageSettings);

    // Check if Montage is configured with Miracast mode
    Launcher launcher;

    //Set settings panel values
#ifdef Q_OS_ANDROID
    // Read settings
    bool fileSharing = true;
    bool audioSharing = false;
    bool iOSEnabled = true;
    bool cloudConnections = true;
    bool enablePin = false;
    bool enableAP = false;
    QString hostName = QHostInfo::localHostName();
#else
    // Read settings
    montageSettings->setValue("file-sharing", true);
    bool fileSharing = true;
    bool audioSharing = montageSettings->boolValue("audio-sharing", true);
    bool iOSEnabled = montageSettings->boolValue("ios-miracast-receiver", true);
    bool cloudConnections = montageSettings->boolValue("cloud-connections", true);
    bool enablePin = montageSettings->boolValue("enable-pin", true);
    bool enableAP = montageSettings->boolValue("enable-access-point", false);
    QString hostName = montageSettings->value("hostname", QHostInfo::localHostName()).toString();
#endif

    montageSettings->beginGroup("QQControlsFileDialog");
    montageSettings->setValue("width", 800);
    montageSettings->setValue("height", 600);
    montageSettings->endGroup();

    montage::media::MediaList mediaList;
    mediaList.setMediaPath(media_path);

    engine.rootContext()->setContextProperty("MediaList", &mediaList);
    engine.rootContext()->setContextProperty("ScriptLauncher", &launcher);
    engine.rootContext()->setContextProperty("ProxyUtils", &proxyUtils);
    engine.rootContext()->setContextProperty("fileSharing", fileSharing);
    engine.rootContext()->setContextProperty("audioSharing", audioSharing);
    engine.rootContext()->setContextProperty("iOSEnabled", iOSEnabled);
    engine.rootContext()->setContextProperty("cloudConnections", cloudConnections);
    engine.rootContext()->setContextProperty("enablePin", enablePin);
    engine.rootContext()->setContextProperty("enableAP", enableAP);
    engine.rootContext()->setContextProperty("gateway", gateway);
    engine.rootContext()->setContextProperty("enableAutoAD", true);
    engine.rootContext()->setContextProperty("ProxyUtils", &proxyUtils);
    engine.rootContext()->setContextProperty("screenDPI", dpi(app));
    engine.rootContext()->setContextProperty("isAtheros", isAtheros(launcher));
    engine.rootContext()->setContextProperty("appName", AppConfiguration::getInstance()->appName());
    engine.rootContext()->setContextProperty("mobileAppsName", AppConfiguration::getInstance()->mobileAppsName());
    engine.rootContext()->setContextProperty("supportEmail", AppConfiguration::getInstance()->supportEmail());
    engine.rootContext()->setContextProperty("salesEmail", AppConfiguration::getInstance()->salesEmail());
    engine.rootContext()->setContextProperty("contactUrl", AppConfiguration::getInstance()->contactUrl());
    engine.rootContext()->setContextProperty("first_start",AppConfiguration::getInstance()->serialNumber().isEmpty() || AppConfiguration::getInstance()->serialNumber().isNull() );
    engine.rootContext()->setContextProperty("hostName", hostname(montageSettings));
    engine.rootContext()->setContextProperty("xmppServer", AppConfiguration::getInstance()->xmppHost());
    engine.rootContext()->setContextProperty("xmppPort", AppConfiguration::getInstance()->xmppPort());
    engine.rootContext()->setContextProperty("xmppPassword", AppConfiguration::getInstance()->xmppPasswd());
    engine.rootContext()->setContextProperty("xmppDomain", AppConfiguration::getInstance()->xmppDomain());
    engine.rootContext()->setContextProperty("apiVersion", AppConfiguration::getInstance()->restVersion());
    engine.rootContext()->setContextProperty("xmppResource", AppConfiguration::getInstance()->xmppResource());
    engine.rootContext()->setContextProperty("xmppNode", AppConfiguration::getInstance()->xmppNode());
    engine.rootContext()->setContextProperty("joinmontageUrl", AppConfiguration::getInstance()->joinmontageUrl());
    engine.rootContext()->setContextProperty("showRegisterForm", AppConfiguration::getInstance()->showRegisterForm());
    engine.rootContext()->setContextProperty("debug", AppConfiguration::getInstance()->isOnDebug());
    if(AppConfiguration::getInstance()->isOnDebug()){
#ifndef _DEBUG
#define _DEBUG
#endif
    }

    // IMPORTANT: We need to registry the ActivationCheckerInstance to be used by
    //            QmlActivationChecker. To see why it's so important go to
    //            "instanceOfQmlActivationChecker" method in "qmlactivationchecker.h" file,
    //            or ask fmoya
    engine.rootContext()->setContextProperty("ActivationCheckerInstance", activationChecker);
    qmlRegisterSingletonType<QmlActivationChecker>("ActivationChecker", 1, 0, "ActivationChecker", instanceOfQmlActivationChecker);

#if defined(__LINUX__) && !defined(__ANDROID__)
    UdevUSB udevUsb;
    engine.rootContext()->setContextProperty("udev", &udevUsb);
#endif

// Send info about debug is enabled or not to QML layer
#ifdef _DEBUG
    engine.rootContext()->setContextProperty("debug", true);
#else
    engine.rootContext()->setContextProperty("debug", false);
#endif

#ifdef RECEIVER
    engine.rootContext()->setContextProperty("receiver", true);
#else
    engine.rootContext()->setContextProperty("receiver", false);
#endif

    qmlRegisterSingletonType<FileSystemModel>("FileSystemBrowser", 1, 0, "FileSystemModel", instanceOfFileSystemModel);

    int r = platformMain(app, engine);
    if (r) return r;

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    r = app.exec();

#ifdef __WIN32__
    montageRunningChecker.detach();
#endif

#if defined(__LINUX__) && !defined(__ANDROID__)
    udevUsb.stop();
#endif

#ifdef __WIN32__
    delete nativeEventFilter;
#endif

#ifndef __ANDROID__
    rtc::CleanupSSL();
#endif
    delete montageSettings;

#ifdef RECEIVER
    delete settingsLockController;
#endif

    return r;
	return nueva linea;
}
