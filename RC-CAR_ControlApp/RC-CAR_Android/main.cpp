#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "inetsocket.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    app.setOrganizationName("ShakalCorp");
    app.setApplicationName("RC-CAR Controller");

    qmlRegisterType<InetSocket>("inetSocket", 1, 0, "InetSocket");
    InetSocket* inetSocket = new InetSocket();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("inetSocket", inetSocket);
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
