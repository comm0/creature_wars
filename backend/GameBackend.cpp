#include "GameBackend.h"

#include "game_constants.h"

#include <QColor>
#include <QFile>
#include <QIODevice>
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
    , game_observer_(this)
    , game_(load_creature_types())
{
    connect(
        &game_observer_,
        &GameObserver::gameTick,
        this,
        &GameBackend::heartbeat
    );
    connect(
        &game_observer_,
        &GameObserver::creatureCreated,
        this,
        &GameBackend::receive_creature
    );
    connect(
        &game_observer_,
        &GameObserver::creatureMoved,
        this,
        [this](
            std::uint64_t id_p,
            position_t previous_position_p,
            position_t position_p
        ) {
            static_cast<void>(previous_position_p);
            receive_creature_position(id_p, position_p);
        }
    );
    connect(
        &game_observer_,
        &GameObserver::creatureHealthChanged,
        this,
        [this](std::uint64_t id_p, int health_p) {
            creatures_model_.update_creature_health(id_p, health_p);
        }
    );
    connect(
        &game_observer_,
        &GameObserver::creatureRemoved,
        this,
        [this](std::uint64_t id_p) {
            creatures_model_.remove_creature(id_p);
        }
    );

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

    game_.start(game_observer_);

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
    creatures_model_.update_or_insert_creature(
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

void GameBackend::receive_creature_position(
    std::uint64_t id_p,
    position_t position_p
)
{
    creatures_model_.update_creature_position(id_p, position_p);
}
