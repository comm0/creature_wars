#include "CreaturesModel.h"

#include <QByteArray>
#include <QHash>
#include <QTimer>
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
    case id_role:
        return QVariant::fromValue(creature.id_);
    case identifier_role:
        return creature.identifier_;
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
    case maximum_health_role:
        return creature.maximum_health_;
    case attack_role:
        return creature.attack_;
    case attack_range_role:
        return creature.attack_range_;
    case vision_range_role:
        return creature.vision_range_;
    case speed_role:
        return creature.speed_;
    case direction_role:
        return creature.direction_;
    case state_role:
        return creature.state_;
    case alert_revision_role:
        return creature.alert_revision_;
    case damage_amount_role:
        return creature.damage_amount_;
    case damage_revision_role:
        return creature.damage_revision_;
    case attack_revision_role:
        return creature.attack_revision_;
    case attack_target_column_role:
        return creature.attack_target_.column_;
    case attack_target_row_role:
        return creature.attack_target_.row_;
    case walk_command_revision_role:
        return creature.walk_command_revision_;
    case target_id_role:
        return QVariant::fromValue(creature.target_id_);
    case removing_role:
        return creature.removing_;
    default:
        return {};
    }
}

QHash<int, QByteArray> CreaturesModel::roleNames() const
{
    return {
        {id_role, "creatureId"},
        {identifier_role, "creatureTypeIdentifier"},
        {column_role, "column"},
        {row_role, "row"},
        {name_role, "creatureName"},
        {group_role, "creatureGroup"},
        {color_role, "creatureColor"},
        {marker_color_role, "markerColor"},
        {health_role, "health"},
        {maximum_health_role, "maximumHealth"},
        {attack_role, "attack"},
        {attack_range_role, "attackRange"},
        {vision_range_role, "visionRange"},
        {speed_role, "movementSpeed"},
        {direction_role, "creatureDirection"},
        {state_role, "creatureState"},
        {alert_revision_role, "alertRevision"},
        {damage_amount_role, "damageAmount"},
        {damage_revision_role, "damageRevision"},
        {attack_revision_role, "attackRevision"},
        {attack_target_column_role, "attackTargetColumn"},
        {attack_target_row_role, "attackTargetRow"},
        {walk_command_revision_role, "walkCommandRevision"},
        {target_id_role, "targetId"},
        {removing_role, "removing"}
    };
}

void CreaturesModel::update_or_insert_creature(
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
            std::move(identifier_p),
            std::move(name_p),
            std::move(group_p),
            std::move(color_p),
            std::move(marker_color_p),
            health_p,
            maximum_health_p,
            attack_p,
            attack_range_p,
            vision_range_p,
            speed_p,
            QStringLiteral("south"),
            QStringLiteral("idle"),
            0,
            0,
            0,
            0,
            0,
            position_p,
            0,
            0,
            false
        });
        endInsertRows();
        return;
    }

    creature->position_ = position_p;
    creature->identifier_ = std::move(identifier_p);
    creature->name_ = std::move(name_p);
    creature->group_ = std::move(group_p);
    creature->color_ = std::move(color_p);
    creature->marker_color_ = std::move(marker_color_p);
    creature->health_ = health_p;
    creature->maximum_health_ = maximum_health_p;
    creature->attack_ = attack_p;
    creature->attack_range_ = attack_range_p;
    creature->vision_range_ = vision_range_p;
    creature->speed_ = speed_p;
    const auto row = static_cast<int>(std::distance(creatures_.begin(), creature));
    const auto model_index = createIndex(row, 0);
    emit dataChanged(model_index, model_index, {
        column_role,
        row_role,
        identifier_role,
        name_role,
        group_role,
        color_role,
        marker_color_role,
        health_role,
        maximum_health_role,
        attack_role,
        attack_range_role,
        vision_range_role,
        speed_role
    });
}

void CreaturesModel::update_creature_position(
    std::uint64_t id_p,
    position_t position_p,
    QString direction_p
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
    creature->direction_ = std::move(direction_p);
    const auto row = static_cast<int>(std::distance(creatures_.begin(), creature));
    const auto model_index = createIndex(row, 0);
    emit dataChanged(model_index, model_index, {
        column_role,
        row_role,
        direction_role
    });
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

    const auto previous_health = creature->health_;
    creature->health_ = health_p;

    if (health_p < previous_health) {
        creature->pending_damage_ += previous_health - health_p;
    }

    const auto row = static_cast<int>(std::distance(creatures_.begin(), creature));
    const auto model_index = createIndex(row, 0);
    emit dataChanged(model_index, model_index, {health_role});
}

void CreaturesModel::publish_damage()
{
    for (std::size_t index = 0; index < creatures_.size(); ++index) {
        auto& creature = creatures_[index];

        if (creature.pending_damage_ <= 0) {
            continue;
        }

        creature.damage_amount_ = creature.pending_damage_;
        creature.pending_damage_ = 0;
        ++creature.damage_revision_;
        const auto model_index = createIndex(static_cast<int>(index), 0);
        emit dataChanged(model_index, model_index, {
            damage_amount_role,
            damage_revision_role
        });
    }
}

void CreaturesModel::update_creature_state(
    std::uint64_t id_p,
    QString state_p
)
{
    const auto creature = std::find_if(
        creatures_.begin(),
        creatures_.end(),
        [id_p](const auto& entry_p) {
            return entry_p.id_ == id_p;
        }
    );

    if (creature == creatures_.end() || creature->state_ == state_p) {
        return;
    }

    creature->state_ = std::move(state_p);
    const auto row = static_cast<int>(std::distance(creatures_.begin(), creature));
    const auto model_index = createIndex(row, 0);
    emit dataChanged(model_index, model_index, {state_role});
}

void CreaturesModel::notify_creature_spotted(std::uint64_t id_p)
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

    ++creature->alert_revision_;
    const auto row = static_cast<int>(std::distance(creatures_.begin(), creature));
    const auto model_index = createIndex(row, 0);
    emit dataChanged(model_index, model_index, {alert_revision_role});
}

void CreaturesModel::notify_creature_attack(
    std::uint64_t id_p,
    position_t target_position_p
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
        return;
    }

    creature->attack_target_ = target_position_p;
    ++creature->attack_revision_;
    const auto row = static_cast<int>(std::distance(creatures_.begin(), creature));
    const auto model_index = createIndex(row, 0);
    emit dataChanged(model_index, model_index, {
        attack_revision_role,
        attack_target_column_role,
        attack_target_row_role
    });
}

void CreaturesModel::notify_creature_walk_command(std::uint64_t id_p)
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

    ++creature->walk_command_revision_;
    const auto row = static_cast<int>(std::distance(creatures_.begin(), creature));
    const auto model_index = createIndex(row, 0);
    emit dataChanged(model_index, model_index, {walk_command_revision_role});
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

    if (creature->removing_) {
        return;
    }

    creature->removing_ = true;
    const auto row = static_cast<int>(std::distance(creatures_.begin(), creature));
    const auto model_index = createIndex(row, 0);
    emit dataChanged(model_index, model_index, {removing_role});

    QTimer::singleShot(300, this, [this, id_p]() {
        const auto removed_creature = std::find_if(
            creatures_.begin(),
            creatures_.end(),
            [id_p](const auto& entry_p) {
                return entry_p.id_ == id_p;
            }
        );

        if (removed_creature == creatures_.end() || !removed_creature->removing_) {
            return;
        }

        const auto removed_row = static_cast<int>(
            std::distance(creatures_.begin(), removed_creature)
        );
        beginRemoveRows({}, removed_row, removed_row);
        creatures_.erase(removed_creature);
        endRemoveRows();
    });
}

void CreaturesModel::update_creature_target(
    std::uint64_t id_p,
    std::uint64_t target_id_p
)
{
    const auto creature = std::find_if(
        creatures_.begin(),
        creatures_.end(),
        [id_p](const auto& entry_p) {
            return entry_p.id_ == id_p;
        }
    );

    if (creature == creatures_.end() || creature->target_id_ == target_id_p) {
        return;
    }

    creature->target_id_ = target_id_p;
    const auto row = static_cast<int>(std::distance(creatures_.begin(), creature));
    const auto model_index = createIndex(row, 0);
    emit dataChanged(model_index, model_index, {target_id_role});
}
