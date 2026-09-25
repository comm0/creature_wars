#include "GameBackend.h"

#include "game_constants.h"

#include <QColor>
#include <QFile>
#include <QIODevice>
#include <QString>

#include <array>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
constexpr std::array<double, 5> time_scales{1.0, 1.5, 2.0, 5.0, 10.0};

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
        load_resource(QStringLiteral(":/data/base_types.json")),
        load_resource(QStringLiteral(":/data/economy.json")),
        load_resource(QStringLiteral(":/data/tech_tree.json")),
        load_resource(QStringLiteral(":/data/ai_profiles.json"))
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
        &GameObserver::matchEnded,
        this,
        &GameBackend::matchEnded
    );
    connect(
        &game_observer_,
        &GameObserver::creatureTargetChanged,
        &creatures_model_,
        &CreaturesModel::update_creature_target
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
        &GameObserver::corpseCreated,
        &corpses_model_,
        &CorpsesModel::insert_corpse
    );
    connect(
        &game_observer_,
        &GameObserver::corpseRemoved,
        &corpses_model_,
        &CorpsesModel::remove_corpse
    );

    connect(
        &game_observer_,
        &GameObserver::baseCreated,
        &bases_model_,
        &BasesModel::insert_base
    );
    connect(
        &game_observer_,
        &GameObserver::baseChanged,
        &bases_model_,
        &BasesModel::update_base
    );
    connect(
        &game_observer_,
        &GameObserver::baseActionsChanged,
        this,
        [this](std::vector<base_action_state_t> actions_p) {
            base_actions_model_.set_actions(std::move(actions_p));
        }
    );
    connect(
        &game_observer_,
        &GameObserver::areaAttack,
        this,
        [this](position_t center_p, int radius_p, QColor color_p) {
            emit areaAttack(center_p.column_, center_p.row_, radius_p, color_p);
        }
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

    connect(
        &game_observer_,
        &GameObserver::playerStateChanged,
        this,
        [this](player_state_t state_p) {
            player_state_ = std::move(state_p);
            emit playerStateChanged();
        }
    );
    connect(
        &game_observer_,
        &GameObserver::baseControllerChanged,
        &bases_model_,
        &BasesModel::update_base_controller
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

CorpsesModel* GameBackend::corpsesModel()
{
    return &corpses_model_;
}

BasesModel* GameBackend::basesModel()
{
    return &bases_model_;
}

BaseActionsModel* GameBackend::baseActionsModel()
{
    return &base_actions_model_;
}

qulonglong GameBackend::playerBaseId() const
{
    return player_state_.base_id_;
}

bool GameBackend::running() const
{
    return game_running_;
}

double GameBackend::timeScale() const
{
    return time_scales[static_cast<std::size_t>(time_scale_index_)];
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

QString GameBackend::playerGroup() const
{
    return QString::fromStdString(player_state_.base_group_);
}

QString GameBackend::playerBaseName() const
{
    return QString::fromStdString(player_state_.base_name_);
}

int GameBackend::playerBaseLevel() const
{
    return player_state_.base_level_;
}

int GameBackend::playerGold() const
{
    return player_state_.gold_.amount_;
}

int GameBackend::playerFood() const
{
    return player_state_.food_.amount_;
}

int GameBackend::goldIncome() const
{
    return player_state_.gold_.income_;
}

int GameBackend::goldIncomeInterval() const
{
    return static_cast<int>(player_state_.gold_.income_interval_.count());
}

int GameBackend::goldIncomeCycle() const
{
    return player_state_.gold_.income_cycle_;
}

int GameBackend::foodIncome() const
{
    return player_state_.food_.income_;
}

int GameBackend::foodIncomeInterval() const
{
    return static_cast<int>(player_state_.food_.income_interval_.count());
}

int GameBackend::foodIncomeCycle() const
{
    return player_state_.food_.income_cycle_;
}

void GameBackend::start()
{
    if (game_running_) {
        return;
    }

    game_.start(game_observer_);
    game_.request_set_aggressive(aggressive_);
    game_.request_set_time_scale(timeScale());

    game_running_ = true;
    emit runningChanged();
}

void GameBackend::cycleTimeScale()
{
    time_scale_index_ = (time_scale_index_ + 1)
        % static_cast<int>(time_scales.size());

    if (game_running_) {
        game_.request_set_time_scale(timeScale());
    }

    emit timeScaleChanged();
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

void GameBackend::startMatch(
    const QString& player_base_identifier_p,
    const QString& minotaur_difficulty_p,
    const QString& orc_difficulty_p,
    const QString& dwarf_difficulty_p
)
{
    if (!game_running_) {
        return;
    }

    game_.request_start_match(
        player_base_identifier_p.toStdString(),
        {
            {
                "minotaur_base",
                ai_difficulty_from_string(minotaur_difficulty_p.toStdString())
            },
            {
                "orc_base",
                ai_difficulty_from_string(orc_difficulty_p.toStdString())
            },
            {
                "dwarf_base",
                ai_difficulty_from_string(dwarf_difficulty_p.toStdString())
            }
        }
    );
}

void GameBackend::spawnCreature(
    const QString& identifier_p,
    int column_p,
    int row_p
)
{
    if (!game_running_) {
        return;
    }

    game_.request_spawn_creature(
        identifier_p.toStdString(),
        position_t{column_p, row_p}
    );
}

void GameBackend::orderBaseAction(const QString& key_p)
{
    if (!game_running_) {
        return;
    }

    game_.request_order_base_action(key_p.toStdString());
}

void GameBackend::attackTarget(
    std::uint64_t creature_id_p,
    std::uint64_t target_id_p
)
{
    if (!game_running_) {
        return;
    }

    game_.request_attack_target(creature_id_p, target_id_p);
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
