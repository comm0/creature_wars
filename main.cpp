#include <QApplication>
#include <QCoreApplication>
#include <QByteArray>
#include <QQmlApplicationEngine>
#include <QtLogging>
#include <QVariant>
#include <QVariantMap>

#include <cstdlib>

#include <FelgoApplication>

#include "backend/GameBackend.h"
#include "backend/debug_console.h"

#ifndef NDEBUG
namespace
{
void qt_message_handler(
    QtMsgType type_p,
    const QMessageLogContext& context_p,
    const QString& message_p
)
{
    if (qstrcmp(context_p.category, "qt.qpa.fonts") == 0) {
        return;
    }

    const auto message = qFormatLogMessage(type_p, context_p, message_p).toLocal8Bit();
    debug_console::print_message(message.constData());
}
}
#endif

int main(int argc, char* argv[])
{
#ifndef NDEBUG
    debug_console::initialize();
    debug_console::print_message(
        "Build branch: " CREATURE_WARS_BUILD_BRANCH
    );
    qInstallMessageHandler(qt_message_handler);
#endif

    QApplication app(argc, argv);

    FelgoApplication felgo;
    GameBackend game_backend;
    QQmlApplicationEngine engine;

    felgo.initialize(&engine);

    engine.setInitialProperties({
        {QStringLiteral("gameBackend"), QVariant::fromValue(&game_backend)}
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

    const auto exit_code = app.exec();
    game_backend.stop();
#ifndef NDEBUG
    debug_console::shutdown();
#endif
    return exit_code;
}
