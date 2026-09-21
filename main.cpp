#include <QApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QVariant>
#include <QVariantMap>

#include <cstdlib>

#include <FelgoApplication>

#include "backend/GameBackend.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    FelgoApplication felgo;
    QQmlApplicationEngine engine;

    felgo.initialize(&engine);

    GameBackend gameBackend;
    engine.setInitialProperties({
        {QStringLiteral("gameBackend"), QVariant::fromValue(&gameBackend)}
    });

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(EXIT_FAILURE); },
        Qt::QueuedConnection
    );

    felgo.setMainQmlFileName(QStringLiteral("qml/Main.qml"));
    engine.load(QUrl(felgo.mainQmlFileName()));

    return app.exec();
}
