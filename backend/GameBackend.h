#pragma once

#include <QObject>

#include "CreaturesModel.h"
#include "game.h"

class GameBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(CreaturesModel* creaturesModel READ creaturesModel CONSTANT)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(int mapColumnCount READ mapColumnCount CONSTANT)
    Q_PROPERTY(int mapRowCount READ mapRowCount CONSTANT)

public:
    explicit GameBackend(QObject* parent_p = nullptr);
    ~GameBackend() override;

    CreaturesModel* creaturesModel();
    bool running() const;
    int mapColumnCount() const;
    int mapRowCount() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void spawnMinotaur();
    Q_INVOKABLE void spawnOrc();

signals:
    void runningChanged();
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
        int attack_p,
        int attack_range_p,
        int vision_range_p
    );

    game_t game_;
    CreaturesModel creatures_model_;
    bool game_running_ = false;
};
