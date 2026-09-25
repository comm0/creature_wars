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
    const auto maximum_health = type.health();
    const auto attack = type.attack();
    const auto attack_range = type.attack_range();
    auto spawn_creature_name = QString::fromStdString(base_p.spawn_type().name());

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
            attack_range,
            spawn_creature_name = std::move(spawn_creature_name)
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
                attack_range,
                std::move(spawn_creature_name)
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
