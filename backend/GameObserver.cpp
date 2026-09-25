#include "GameObserver.h"

#include <QMetaObject>

#include <utility>

namespace
{
QString direction_name(direction_t direction_p)
{
    switch (direction_p) {
    case direction_t::north:
        return QStringLiteral("north");
    case direction_t::east:
        return QStringLiteral("east");
    case direction_t::south:
        return QStringLiteral("south");
    case direction_t::west:
        return QStringLiteral("west");
    }

    return QStringLiteral("south");
}
}

GameObserver::GameObserver(QObject* parent_p)
    : QObject(parent_p)
{
}

void GameObserver::on_game_tick()
{
    QMetaObject::invokeMethod(
        this,
        [this]() { emit gameTick(); },
        Qt::QueuedConnection
    );
}

void GameObserver::on_creature_created(const creature_t& creature_p)
{
    const auto id = creature_p.id();
    const auto position = creature_p.position();
    auto identifier = QString::fromStdString(creature_p.type().identifier());
    auto name = QString::fromStdString(creature_p.type().name());
    auto group = QString::fromStdString(creature_p.type().group());
    const auto color = QColor::fromRgb(creature_p.type().color());
    const auto marker_color_value = creature_p.type().marker_color();
    const auto marker_color = marker_color_value.has_value()
        ? QColor::fromRgb(*marker_color_value)
        : QColor(0, 0, 0, 0);
    const auto health = creature_p.health();
    const auto maximum_health = creature_p.type().health();
    const auto attack = creature_p.type().attack();
    const auto attack_range = creature_p.type().attack_range();
    const auto vision_range = creature_p.type().vision_range();
    const auto speed = creature_p.type().speed();

    QMetaObject::invokeMethod(
        this,
        [
            this,
            id,
            position,
            identifier = std::move(identifier),
            name = std::move(name),
            group = std::move(group),
            color,
            marker_color,
            health,
            maximum_health,
            attack,
            attack_range,
            vision_range,
            speed
        ]() mutable {
            emit creatureCreated(
                id,
                position,
                std::move(identifier),
                std::move(name),
                std::move(group),
                color,
                marker_color,
                health,
                maximum_health,
                attack,
                attack_range,
                vision_range,
                speed
            );
        },
        Qt::QueuedConnection
    );
}

void GameObserver::on_creature_moved(
    std::uint64_t id_p,
    position_t position_p,
    direction_t direction_p
)
{
    auto direction = direction_name(direction_p);

    QMetaObject::invokeMethod(
        this,
        [this, id_p, position_p, direction = std::move(direction)]() {
            emit creatureMoved(id_p, position_p, direction);
        },
        Qt::QueuedConnection
    );
}

void GameObserver::on_creature_health_changed(
    std::uint64_t id_p,
    int health_p
)
{
    QMetaObject::invokeMethod(
        this,
        [this, id_p, health_p]() {
            emit creatureHealthChanged(id_p, health_p);
        },
        Qt::QueuedConnection
    );
}

void GameObserver::on_creature_attack_performed(std::uint64_t id_p)
{
    QMetaObject::invokeMethod(
        this,
        [this, id_p]() { emit creatureAttackPerformed(id_p); },
        Qt::QueuedConnection
    );
}

void GameObserver::on_creature_walk_requested(std::uint64_t id_p)
{
    QMetaObject::invokeMethod(
        this,
        [this, id_p]() { emit creatureWalkRequested(id_p); },
        Qt::QueuedConnection
    );
}

void GameObserver::on_creature_state_changed(
    std::uint64_t id_p,
    creature_state_t state_p
)
{
    const auto state = state_p == creature_state_t::walking
        ? QStringLiteral("walk")
        : QStringLiteral("idle");

    QMetaObject::invokeMethod(
        this,
        [this, id_p, state]() {
            emit creatureStateChanged(id_p, state);
        },
        Qt::QueuedConnection
    );
}

void GameObserver::on_creature_spotted(
    std::uint64_t observer_id_p,
    std::uint64_t spotted_id_p
)
{
    QMetaObject::invokeMethod(
        this,
        [this, observer_id_p, spotted_id_p]() {
            emit creatureSpotted(observer_id_p, spotted_id_p);
        },
        Qt::QueuedConnection
    );
}

void GameObserver::on_creature_removed(std::uint64_t id_p)
{
    QMetaObject::invokeMethod(
        this,
        [this, id_p]() { emit creatureRemoved(id_p); },
        Qt::QueuedConnection
    );
}

void GameObserver::on_corpse_created(
    std::uint64_t id_p,
    position_t position_p,
    const std::string& identifier_p
)
{
    auto identifier = QString::fromStdString(identifier_p);

    QMetaObject::invokeMethod(
        this,
        [this, id_p, position_p, identifier = std::move(identifier)]() mutable {
            emit corpseCreated(id_p, position_p, std::move(identifier));
        },
        Qt::QueuedConnection
    );
}

void GameObserver::on_corpse_removed(std::uint64_t id_p)
{
    QMetaObject::invokeMethod(
        this,
        [this, id_p]() { emit corpseRemoved(id_p); },
        Qt::QueuedConnection
    );
}

void GameObserver::on_base_created(const base_t& base_p)
{
    const auto id = base_p.id();
    const auto position = base_p.position();
    const auto& type = base_p.type();
    const auto size = type.size();
    const auto level = base_p.level();
    auto identifier = QString::fromStdString(type.identifier());
    auto name = QString::fromStdString(type.name());
    auto group = QString::fromStdString(type.group());
    const auto color = QColor::fromRgb(type.color());
    const auto health = base_p.health();
    const auto maximum_health = base_p.stats().max_health_;
    const auto attack = base_p.stats().attack_;
    const auto range = base_p.stats().range_;

    QMetaObject::invokeMethod(
        this,
        [
            this,
            id,
            position,
            size,
            level,
            identifier = std::move(identifier),
            name = std::move(name),
            group = std::move(group),
            color,
            health,
            maximum_health,
            attack,
            range
        ]() mutable {
            emit baseCreated(
                id,
                position,
                size,
                level,
                std::move(identifier),
                std::move(name),
                std::move(group),
                color,
                health,
                maximum_health,
                attack,
                range
            );
        },
        Qt::QueuedConnection
    );
}

void GameObserver::on_base_health_changed(std::uint64_t id_p, int health_p)
{
    QMetaObject::invokeMethod(
        this,
        [this, id_p, health_p]() { emit baseHealthChanged(id_p, health_p); },
        Qt::QueuedConnection
    );
}

void GameObserver::on_base_attack_performed(
    std::uint64_t id_p,
    position_t target_position_p
)
{
    QMetaObject::invokeMethod(
        this,
        [this, id_p, target_position_p]() {
            emit baseAttackPerformed(id_p, target_position_p);
        },
        Qt::QueuedConnection
    );
}

void GameObserver::on_base_removed(std::uint64_t id_p)
{
    QMetaObject::invokeMethod(
        this,
        [this, id_p]() { emit baseRemoved(id_p); },
        Qt::QueuedConnection
    );
}

void GameObserver::on_player_state_changed(const player_state_t& state_p)
{
    QMetaObject::invokeMethod(
        this,
        [this, state = state_p]() { emit playerStateChanged(state); },
        Qt::QueuedConnection
    );
}

void GameObserver::on_base_controller_changed(
    const base_controller_state_t& state_p
)
{
    const auto base_id = state_p.resources_.base_id_;
    const auto gold = state_p.resources_.gold_.amount_;
    const auto food = state_p.resources_.food_.amount_;
    const auto gold_income = state_p.resources_.gold_.income_;
    const auto food_income = state_p.resources_.food_.income_;
    auto difficulty = QString::fromStdString(state_p.difficulty_);
    auto strategy = QString::fromStdString(state_p.strategy_);
    const auto human_controlled = state_p.human_controlled_;

    QMetaObject::invokeMethod(
        this,
        [
            this,
            base_id,
            gold,
            food,
            gold_income,
            food_income,
            difficulty = std::move(difficulty),
            strategy = std::move(strategy),
            human_controlled
        ]() mutable {
            emit baseControllerChanged(
                base_id,
                gold,
                food,
                gold_income,
                food_income,
                std::move(difficulty),
                std::move(strategy),
                human_controlled
            );
        },
        Qt::QueuedConnection
    );
}

void GameObserver::on_base_changed(const base_t& base_p)
{
    const auto id = base_p.id();
    const auto level = base_p.level();
    const auto health = base_p.health();
    const auto& stats = base_p.stats();
    const auto maximum_health = stats.max_health_;
    const auto attack = stats.attack_;
    const auto range = stats.range_;

    QMetaObject::invokeMethod(
        this,
        [this, id, level, health, maximum_health, attack, range]() {
            emit baseChanged(id, level, health, maximum_health, attack, range);
        },
        Qt::QueuedConnection
    );
}

void GameObserver::on_base_actions_changed(
    std::uint64_t base_id_p,
    const std::vector<base_action_state_t>& actions_p
)
{
    static_cast<void>(base_id_p);

    QMetaObject::invokeMethod(
        this,
        [this, actions = actions_p]() mutable {
            emit baseActionsChanged(std::move(actions));
        },
        Qt::QueuedConnection
    );
}

void GameObserver::on_area_attack(
    position_t center_p,
    int radius_p,
    std::uint32_t color_p
)
{
    const auto color = QColor::fromRgb(color_p);

    QMetaObject::invokeMethod(
        this,
        [this, center_p, radius_p, color]() {
            emit areaAttack(center_p, radius_p, color);
        },
        Qt::QueuedConnection
    );
}

void GameObserver::on_creature_target_changed(
    std::uint64_t id_p,
    std::uint64_t target_id_p
)
{
    QMetaObject::invokeMethod(
        this,
        [this, id_p, target_id_p]() { emit creatureTargetChanged(id_p, target_id_p); },
        Qt::QueuedConnection
    );
}

void GameObserver::on_match_ended(bool victory_p, std::chrono::milliseconds duration_p)
{
    const auto duration_ms = static_cast<int>(duration_p.count());

    QMetaObject::invokeMethod(
        this,
        [this, victory_p, duration_ms]() { emit matchEnded(victory_p, duration_ms); },
        Qt::QueuedConnection
    );
}
