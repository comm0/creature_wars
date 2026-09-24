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
std::string load_resource(const QString& path_p)
{
    QFile file(path_p);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Could not open " + path_p.toStdString());
    }

    return file.readAll().toStdString();
}
}

GameBackend::GameBackend(QObject* parent_p)
    : QObject(parent_p)
    , game_observer_(this)
    , game_(
        load_resource(QStringLiteral(":/data/creature_types.json")),
        load_resource(QStringLiteral(":/data/base_types.json"))
    )
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
            position_t position_p,
            QString direction_p
        ) {
            receive_creature_position(
                id_p,
                position_p,
                std::move(direction_p)
            );
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
        &GameObserver::creatureAttackPerformed,
        this,
        [this](std::uint64_t id_p) {
            creatures_model_.notify_creature_attack(id_p);
        }
    );
    connect(
        &game_observer_,
        &GameObserver::creatureWalkRequested,
        this,
        [this](std::uint64_t id_p) {
            creatures_model_.notify_creature_walk_command(id_p);
        }
    );
    connect(
        &game_observer_,
        &GameObserver::creatureStateChanged,
        this,
        [this](std::uint64_t id_p, QString state_p) {
            creatures_model_.update_creature_state(
                id_p,
                std::move(state_p)
            );
        }
    );
    connect(
        &game_observer_,
        &GameObserver::creatureSpotted,
        this,
        [this](std::uint64_t observer_id_p, std::uint64_t spotted_id_p) {
            static_cast<void>(spotted_id_p);
            creatures_model_.notify_creature_spotted(observer_id_p);
        }
    );
    connect(
        &game_observer_,
        &GameObserver::creatureRemoved,
        this,
        [this](std::uint64_t id_p) {
            creatures_model_.remove_creature(id_p);
            emit creatureRemoved(id_p);
        }
    );

    connect(
        &game_observer_,
        &GameObserver::baseCreated,
        &bases_model_,
        &BasesModel::insert_base
    );
    connect(
        &game_observer_,
        &GameObserver::baseHealthChanged,
        &bases_model_,
        &BasesModel::update_base_health
    );
    connect(
        &game_observer_,
        &GameObserver::baseAttackPerformed,
        &bases_model_,
        &BasesModel::notify_base_attack
    );
    connect(
        &game_observer_,
        &GameObserver::baseRemoved,
        &bases_model_,
        &BasesModel::remove_base
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

BasesModel* GameBackend::basesModel()
{
    return &bases_model_;
}

bool GameBackend::running() const
{
    return game_running_;
}

bool GameBackend::aggressive() const
{
    return aggressive_;
}

void GameBackend::setAggressive(bool aggressive_p)
{
    if (aggressive_ == aggressive_p) {
        return;
    }

    aggressive_ = aggressive_p;

    if (game_running_) {
        game_.request_set_aggressive(aggressive_p);
    }

    emit aggressiveChanged();
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
    game_.request_set_aggressive(aggressive_);

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

void GameBackend::spawnBase(
    const QString& identifier_p,
    int column_p,
    int row_p
)
{
    if (!game_running_) {
        return;
    }

    game_.request_spawn_base(
        identifier_p.toStdString(),
        position_t{column_p, row_p}
    );
}

void GameBackend::spawnCreatureFromBase(std::uint64_t base_id_p)
{
    if (!game_running_) {
        return;
    }

    game_.request_spawn_from_base(base_id_p);
}

void GameBackend::walkCreature(
    std::uint64_t creature_id_p,
    int column_p,
    int row_p
)
{
    if (!game_running_) {
        return;
    }

    game_.request_walk_to(
        creature_id_p,
        position_t{column_p, row_p}
    );
}

void GameBackend::receive_creature(
    std::uint64_t id_p,
    position_t position_p,
    QString identifier_p,
    QString name_p,
    QString group_p,
    QColor color_p,
    QColor marker_color_p,
    int health_p,
    int maximum_health_p,
    int attack_p,
    int attack_range_p,
    int vision_range_p,
    double speed_p
)
{
    creatures_model_.update_or_insert_creature(
        id_p,
        position_p,
        std::move(identifier_p),
        std::move(name_p),
        std::move(group_p),
        std::move(color_p),
        std::move(marker_color_p),
        health_p,
        maximum_health_p,
        attack_p,
        attack_range_p,
        vision_range_p,
        speed_p
    );
}

void GameBackend::receive_creature_position(
    std::uint64_t id_p,
    position_t position_p,
    QString direction_p
)
{
    creatures_model_.update_creature_position(
        id_p,
        position_p,
        std::move(direction_p)
    );
}
