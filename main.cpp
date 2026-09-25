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
QtMessageHandler felgo_message_handler = nullptr;

void qt_message_handler(
    QtMsgType type_p,
    const QMessageLogContext& context_p,
    const QString& message_p
)
{
    thread_local auto forwarding = false;

    if (qstrcmp(context_p.category, "qt.qpa.fonts") == 0) {
        return;
    }

    if (felgo_message_handler != nullptr && !forwarding) {
        forwarding = true;
        felgo_message_handler(type_p, context_p, message_p);
        forwarding = false;
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
#endif

    QApplication app(argc, argv);

    FelgoApplication felgo;
    GameBackend game_backend;
    QQmlApplicationEngine engine;

    felgo.initialize(&engine);
#ifndef NDEBUG
    felgo_message_handler = qInstallMessageHandler(qt_message_handler);
#endif

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
