#pragma once

#include <QColor>
#include <QObject>
#include <QString>

#include "game_observer.h"

class GameObserver : public QObject, public igame_observer_t
{
    Q_OBJECT

public:
    explicit GameObserver(QObject* parent_p = nullptr);

    void on_game_tick() override;
    void on_creature_created(const creature_t& creature_p) override;
    void on_creature_moved(
        std::uint64_t id_p,
        position_t position_p,
        direction_t direction_p
    ) override;
    void on_creature_health_changed(
        std::uint64_t id_p,
        int health_p
    ) override;
    void on_creature_attack_performed(std::uint64_t id_p) override;
    void on_creature_walk_requested(std::uint64_t id_p) override;
    void on_creature_state_changed(
        std::uint64_t id_p,
        creature_state_t state_p
    ) override;
    void on_creature_spotted(
        std::uint64_t observer_id_p,
        std::uint64_t spotted_id_p
    ) override;
    void on_creature_removed(std::uint64_t id_p) override;

signals:
    void gameTick();
    void creatureCreated(
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
    );
    void creatureMoved(
        std::uint64_t id_p,
        position_t position_p,
        QString direction_p
    );
    void creatureHealthChanged(std::uint64_t id_p, int health_p);
    void creatureAttackPerformed(std::uint64_t id_p);
    void creatureWalkRequested(std::uint64_t id_p);
    void creatureStateChanged(std::uint64_t id_p, QString state_p);
    void creatureSpotted(
        std::uint64_t observer_id_p,
        std::uint64_t spotted_id_p
    );
    void creatureRemoved(std::uint64_t id_p);
};
