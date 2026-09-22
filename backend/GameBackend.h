#pragma once

#include <QObject>

#include "CreaturesModel.h"
#include "GameObserver.h"
#include "game.h"

class GameBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(CreaturesModel* creaturesModel READ creaturesModel CONSTANT)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(bool aggressive READ aggressive WRITE setAggressive NOTIFY aggressiveChanged)
    Q_PROPERTY(int mapColumnCount READ mapColumnCount CONSTANT)
    Q_PROPERTY(int mapRowCount READ mapRowCount CONSTANT)

public:
    explicit GameBackend(QObject* parent_p = nullptr);
    ~GameBackend() override;

    CreaturesModel* creaturesModel();
    bool running() const;
    bool aggressive() const;
    void setAggressive(bool aggressive_p);
    int mapColumnCount() const;
    int mapRowCount() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void spawnCreature(
        const QString& identifier_p,
        int column_p,
        int row_p
    );
    Q_INVOKABLE void walkCreature(
        std::uint64_t creature_id_p,
        int column_p,
        int row_p
    );

signals:
    void runningChanged();
    void aggressiveChanged();
    void heartbeat();

private:
    void receive_creature(
        std::uint64_t id_p,
        position_t position_p,
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
    void receive_creature_position(
        std::uint64_t id_p,
        position_t position_p
    );

    GameObserver game_observer_;
    game_t game_;
    CreaturesModel creatures_model_;
    bool game_running_ = false;
    bool aggressive_ = true;
};
