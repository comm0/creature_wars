#include "GameObserver.h"

#include <QMetaObject>

#include <utility>

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
    auto name = QString::fromStdString(creature_p.type().name());
    auto group = QString::fromStdString(creature_p.type().group());
    const auto color = QColor::fromRgb(creature_p.type().color());
    const auto marker_color_value = creature_p.type().marker_color();
    const auto marker_color = marker_color_value.has_value()
        ? QColor::fromRgb(*marker_color_value)
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
            emit creatureCreated(
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

void GameObserver::on_creature_moved(
    std::uint64_t id_p,
    position_t previous_position_p,
    position_t position_p
)
{
    QMetaObject::invokeMethod(
        this,
        [this, id_p, previous_position_p, position_p]() {
            emit creatureMoved(id_p, previous_position_p, position_p);
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

void GameObserver::on_creature_removed(std::uint64_t id_p)
{
    QMetaObject::invokeMethod(
        this,
        [this, id_p]() { emit creatureRemoved(id_p); },
        Qt::QueuedConnection
    );
}
