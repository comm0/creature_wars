#include "GameBackend.h"

#include "game_constants.h"

#include <QMetaObject>

GameBackend::GameBackend(QObject* parent_p)
    : QObject(parent_p)
{
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

bool GameBackend::running() const
{
    return game_running_;
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

    game_.start(
        [this]() {
            QMetaObject::invokeMethod(
                this,
                [this]() { emit heartbeat(); },
                Qt::QueuedConnection
            );
        },
        [this](std::uint64_t id_p, position_t position_p) {
            QMetaObject::invokeMethod(
                this,
                [this, id_p, position_p]() {
                    receive_creature_position(id_p, position_p);
                },
                Qt::QueuedConnection
            );
        }
    );

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

void GameBackend::receive_creature_position(std::uint64_t id_p, position_t position_p)
{
    creatures_model_.upsert_creature(id_p, position_p);
}
