#include "CreaturesModel.h"

#include <QByteArray>
#include <QHash>
#include <QVariant>

#include <algorithm>
#include <utility>

CreaturesModel::CreaturesModel(QObject* parent_p)
    : QAbstractListModel(parent_p)
{
}

int CreaturesModel::rowCount(const QModelIndex& parent_p) const
{
    if (parent_p.isValid()) {
        return 0;
    }

    return static_cast<int>(creatures_.size());
}

QVariant CreaturesModel::data(const QModelIndex& index_p, int role_p) const
{
    const auto row = index_p.row();

    if (!index_p.isValid() || row < 0 || row >= rowCount()) {
        return {};
    }

    const auto& creature = creatures_[static_cast<std::size_t>(row)];

    switch (role_p) {
    case column_role:
        return creature.position_.column_;
    case row_role:
        return creature.position_.row_;
    case name_role:
        return creature.name_;
    case group_role:
        return creature.group_;
    case color_role:
        return creature.color_;
    case marker_color_role:
        return creature.marker_color_;
    case health_role:
        return creature.health_;
    case attack_role:
        return creature.attack_;
    case attack_range_role:
        return creature.attack_range_;
    case vision_range_role:
        return creature.vision_range_;
    default:
        return {};
    }
}

QHash<int, QByteArray> CreaturesModel::roleNames() const
{
    return {
        {column_role, "column"},
        {row_role, "row"},
        {name_role, "creatureName"},
        {group_role, "creatureGroup"},
        {color_role, "creatureColor"},
        {marker_color_role, "markerColor"},
        {health_role, "health"},
        {attack_role, "attack"},
        {attack_range_role, "attackRange"},
        {vision_range_role, "visionRange"}
    };
}

void CreaturesModel::update_or_insert_creature(
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
)
{
    const auto creature = std::find_if(
        creatures_.begin(),
        creatures_.end(),
        [id_p](const auto& entry_p) {
            return entry_p.id_ == id_p;
        }
    );

    if (creature == creatures_.end()) {
        const auto row = rowCount();
        beginInsertRows({}, row, row);
        creatures_.push_back({
            id_p,
            position_p,
            std::move(name_p),
            std::move(group_p),
            std::move(color_p),
            std::move(marker_color_p),
            health_p,
            attack_p,
            attack_range_p,
            vision_range_p
        });
        endInsertRows();
        return;
    }

    creature->position_ = position_p;
    creature->name_ = std::move(name_p);
    creature->group_ = std::move(group_p);
    creature->color_ = std::move(color_p);
    creature->marker_color_ = std::move(marker_color_p);
    creature->health_ = health_p;
    creature->attack_ = attack_p;
    creature->attack_range_ = attack_range_p;
    creature->vision_range_ = vision_range_p;
    const auto row = static_cast<int>(std::distance(creatures_.begin(), creature));
    const auto model_index = createIndex(row, 0);
    emit dataChanged(model_index, model_index, {
        column_role,
        row_role,
        name_role,
        group_role,
        color_role,
        marker_color_role,
        health_role,
        attack_role,
        attack_range_role,
        vision_range_role
    });
}

void CreaturesModel::update_creature_position(
    std::uint64_t id_p,
    position_t position_p
)
{
    const auto creature = std::find_if(
        creatures_.begin(),
        creatures_.end(),
        [id_p](const auto& entry_p) {
            return entry_p.id_ == id_p;
        }
    );

    if (creature == creatures_.end() || creature->position_ == position_p) {
        return;
    }

    creature->position_ = position_p;
    const auto row = static_cast<int>(std::distance(creatures_.begin(), creature));
    const auto model_index = createIndex(row, 0);
    emit dataChanged(model_index, model_index, {column_role, row_role});
}

void CreaturesModel::update_creature_health(std::uint64_t id_p, int health_p)
{
    const auto creature = std::find_if(
        creatures_.begin(),
        creatures_.end(),
        [id_p](const auto& entry_p) {
            return entry_p.id_ == id_p;
        }
    );

    if (creature == creatures_.end() || creature->health_ == health_p) {
        return;
    }

    creature->health_ = health_p;
    const auto row = static_cast<int>(std::distance(creatures_.begin(), creature));
    const auto model_index = createIndex(row, 0);
    emit dataChanged(model_index, model_index, {health_role});
}

void CreaturesModel::remove_creature(std::uint64_t id_p)
{
    const auto creature = std::find_if(
        creatures_.begin(),
        creatures_.end(),
        [id_p](const auto& entry_p) {
            return entry_p.id_ == id_p;
        }
    );

    if (creature == creatures_.end()) {
        return;
    }

    const auto row = static_cast<int>(std::distance(creatures_.begin(), creature));
    beginRemoveRows({}, row, row);
    creatures_.erase(creature);
    endRemoveRows();
}
