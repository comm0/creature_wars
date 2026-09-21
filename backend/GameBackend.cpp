#include "GameBackend.h"

#include "game_constants.h"

#include <QColor>
#include <QFile>
#include <QIODevice>
#include <QMetaObject>
#include <QString>

#include <stdexcept>
#include <string>
#include <utility>

namespace
{
std::string load_creature_types()
{
    QFile creature_types_file(QStringLiteral(":/data/creature_types.json"));

    if (!creature_types_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Could not open creature type definitions.");
    }

    return creature_types_file.readAll().toStdString();
}
}

GameBackend::GameBackend(QObject* parent_p)
    : QObject(parent_p)
    , game_(load_creature_types())
{
    start();
}

GameBackend::~GameBackend()
{
    game_.stop();
}

CreaturesModel* GameBackend::creaturesModel()
{
    return &creatures_model_;
}

bool GameBackend::running() const
{
    return game_running_;
}

int GameBackend::mapColumnCount() const
{
    return game_constants::map_column_count;
}

int GameBackend::mapRowCount() const
{
    return game_constants::map_row_count;
}

void GameBackend::start()
{
    if (game_running_) {
        return;
    }

    game_.start(
        [this]() {
            QMetaObject::invokeMethod(
                this,
                [this]() { emit heartbeat(); },
                Qt::QueuedConnection
            );
        },
        [this](const creature_t& creature_p) {
            const auto id = creature_p.id();
            const auto position = creature_p.position();
            auto name = QString::fromStdString(creature_p.type().name());
            auto group = QString::fromStdString(creature_p.type().group());
            const auto color = QColor::fromRgb(creature_p.type().color());
            const auto marker_color = creature_p.type().marker_color().has_value()
                ? QColor::fromRgb(*creature_p.type().marker_color())
                : QColor(0, 0, 0, 0);
            const auto health = creature_p.health();
            const auto attack = creature_p.type().attack();
            const auto attack_range = creature_p.type().attack_range();
            const auto vision_range = creature_p.type().vision_range();

            QMetaObject::invokeMethod(
                this,
                [
                    this,
                    id,
                    position,
                    name = std::move(name),
                    group = std::move(group),
                    color,
                    marker_color,
                    health,
                    attack,
                    attack_range,
                    vision_range
                ]() mutable {
                    receive_creature(
                        id,
                        position,
                        std::move(name),
                        std::move(group),
                        color,
                        marker_color,
                        health,
                        attack,
                        attack_range,
                        vision_range
                    );
                },
                Qt::QueuedConnection
            );
        }
    );

    game_running_ = true;
    emit runningChanged();
}

void GameBackend::stop()
{
    if (!game_running_) {
        return;
    }

    game_.stop();
    game_running_ = false;
    emit runningChanged();
}

void GameBackend::spawnMinotaur()
{
    if (!game_running_) {
        return;
    }

    game_.spawn_creature("minotaurs");
}

void GameBackend::spawnOrc()
{
    if (!game_running_) {
        return;
    }

    game_.spawn_creature("orcs");
}

void GameBackend::receive_creature(
    std::uint64_t id_p,
    position_t position_p,
    QString name_p,
    QString group_p,
    QColor color_p,
    QColor marker_color_p,
    int health_p,
    int attack_p,
    int attack_range_p,
    int vision_range_p
)
{
    creatures_model_.upsert_creature(
        id_p,
        position_p,
        std::move(name_p),
        std::move(group_p),
        std::move(color_p),
        std::move(marker_color_p),
        health_p,
        attack_p,
        attack_range_p,
        vision_range_p
    );
}
