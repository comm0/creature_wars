#pragma once

#include <QObject>

#include "BaseActionsModel.h"
#include "BasesModel.h"
#include "CreaturesModel.h"
#include "GameObserver.h"
#include "game.h"

class GameBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(CreaturesModel* creaturesModel READ creaturesModel CONSTANT)
    Q_PROPERTY(BasesModel* basesModel READ basesModel CONSTANT)
    Q_PROPERTY(BaseActionsModel* baseActionsModel READ baseActionsModel CONSTANT)
    Q_PROPERTY(qulonglong playerBaseId READ playerBaseId NOTIFY playerStateChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(double timeScale READ timeScale NOTIFY timeScaleChanged)
    Q_PROPERTY(bool aggressive READ aggressive WRITE setAggressive NOTIFY aggressiveChanged)
    Q_PROPERTY(int mapColumnCount READ mapColumnCount CONSTANT)
    Q_PROPERTY(int mapRowCount READ mapRowCount CONSTANT)
    Q_PROPERTY(QString playerGroup READ playerGroup NOTIFY playerStateChanged)
    Q_PROPERTY(QString playerBaseName READ playerBaseName NOTIFY playerStateChanged)
    Q_PROPERTY(int playerBaseLevel READ playerBaseLevel NOTIFY playerStateChanged)
    Q_PROPERTY(int playerGold READ playerGold NOTIFY playerStateChanged)
    Q_PROPERTY(int playerFood READ playerFood NOTIFY playerStateChanged)
    Q_PROPERTY(int goldIncome READ goldIncome NOTIFY playerStateChanged)
    Q_PROPERTY(int goldIncomeInterval READ goldIncomeInterval NOTIFY playerStateChanged)
    Q_PROPERTY(int goldIncomeCycle READ goldIncomeCycle NOTIFY playerStateChanged)
    Q_PROPERTY(int foodIncome READ foodIncome NOTIFY playerStateChanged)
    Q_PROPERTY(int foodIncomeInterval READ foodIncomeInterval NOTIFY playerStateChanged)
    Q_PROPERTY(int foodIncomeCycle READ foodIncomeCycle NOTIFY playerStateChanged)

public:
    explicit GameBackend(QObject* parent_p = nullptr);
    ~GameBackend() override;

    CreaturesModel* creaturesModel();
    BasesModel* basesModel();
    BaseActionsModel* baseActionsModel();
    qulonglong playerBaseId() const;
    bool running() const;
    double timeScale() const;
    bool aggressive() const;
    void setAggressive(bool aggressive_p);
    int mapColumnCount() const;
    int mapRowCount() const;
    QString playerGroup() const;
    QString playerBaseName() const;
    int playerBaseLevel() const;
    int playerGold() const;
    int playerFood() const;
    int goldIncome() const;
    int goldIncomeInterval() const;
    int goldIncomeCycle() const;
    int foodIncome() const;
    int foodIncomeInterval() const;
    int foodIncomeCycle() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void cycleTimeScale();
    Q_INVOKABLE void spawnBase(
        const QString& identifier_p,
        int column_p,
        int row_p
    );
    Q_INVOKABLE void spawnCreature(
        const QString& identifier_p,
        int column_p,
        int row_p
    );
    Q_INVOKABLE void orderBaseAction(const QString& key_p);
    Q_INVOKABLE void startMatch(const QString& player_base_identifier_p);
    Q_INVOKABLE void attackTarget(
        std::uint64_t creature_id_p,
        std::uint64_t target_id_p
    );
    Q_INVOKABLE void walkCreature(
        std::uint64_t creature_id_p,
        int column_p,
        int row_p
    );

signals:
    void runningChanged();
    void timeScaleChanged();
    void aggressiveChanged();
    void playerStateChanged();
    void areaAttack(int column_p, int row_p, int radius_p, QColor color_p);
    void heartbeat();
    void creatureRemoved(std::uint64_t id_p);

private:
    void receive_creature(
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
    void receive_creature_position(
        std::uint64_t id_p,
        position_t position_p,
        QString direction_p
    );

    GameObserver game_observer_;
    game_t game_;
    CreaturesModel creatures_model_;
    BasesModel bases_model_;
    BaseActionsModel base_actions_model_;
    bool game_running_ = false;
    int time_scale_index_ = 0;
    bool aggressive_ = true;
    player_state_t player_state_;
};
