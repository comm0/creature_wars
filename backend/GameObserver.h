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
    void on_corpse_created(
        std::uint64_t id_p,
        position_t position_p,
        const std::string& identifier_p
    ) override;
    void on_corpse_removed(std::uint64_t id_p) override;
    void on_resource_rewarded(
        position_t position_p,
        int gold_p,
        int food_p
    ) override;
    void on_base_created(const base_t& base_p) override;
    void on_base_health_changed(std::uint64_t id_p, int health_p) override;
    void on_base_attack_performed(
        std::uint64_t id_p,
        position_t target_position_p
    ) override;
    void on_base_removed(std::uint64_t id_p) override;
    void on_player_state_changed(const player_state_t& state_p) override;
    void on_base_controller_changed(
        const base_controller_state_t& state_p
    ) override;
    void on_base_changed(const base_t& base_p) override;
    void on_base_actions_changed(
        std::uint64_t base_id_p,
        const std::vector<base_action_state_t>& actions_p
    ) override;
    void on_area_attack(
        position_t center_p,
        int radius_p,
        std::uint32_t color_p
    ) override;
    void on_match_ended(bool victory_p, std::chrono::milliseconds duration_p) override;
    void on_creature_target_changed(
        std::uint64_t id_p,
        std::uint64_t target_id_p
    ) override;

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
    void corpseCreated(
        std::uint64_t id_p,
        position_t position_p,
        QString identifier_p
    );
    void corpseRemoved(std::uint64_t id_p);
    void resourceRewarded(position_t position_p, int gold_p, int food_p);
    void baseCreated(
        std::uint64_t id_p,
        position_t position_p,
        int size_p,
        int level_p,
        QString identifier_p,
        QString name_p,
        QString group_p,
        QColor color_p,
        int health_p,
        int maximum_health_p,
        int attack_p,
        int range_p
    );
    void baseChanged(
        std::uint64_t id_p,
        int level_p,
        int health_p,
        int maximum_health_p,
        int attack_p,
        int range_p
    );
    void baseActionsChanged(std::vector<base_action_state_t> actions_p);
    void areaAttack(position_t center_p, int radius_p, QColor color_p);
    void baseHealthChanged(std::uint64_t id_p, int health_p);
    void baseAttackPerformed(std::uint64_t id_p, position_t target_position_p);
    void baseRemoved(std::uint64_t id_p);
    void playerStateChanged(player_state_t state_p);
    void baseControllerChanged(
        std::uint64_t base_id_p,
        int gold_p,
        int food_p,
        int gold_income_p,
        int food_income_p,
        QString difficulty_p,
        QString strategy_p,
        bool human_controlled_p
    );
    void matchEnded(bool victory_p, int duration_ms_p);
    void creatureTargetChanged(std::uint64_t id_p, std::uint64_t target_id_p);
};
