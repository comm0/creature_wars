#include "BasesModel.h"

#include <QByteArray>
#include <QHash>
#include <QVariant>

#include <algorithm>
#include <utility>

BasesModel::BasesModel(QObject* parent_p)
    : QAbstractListModel(parent_p)
{
}

int BasesModel::rowCount(const QModelIndex& parent_p) const
{
    if (parent_p.isValid()) {
        return 0;
    }

    return static_cast<int>(bases_.size());
}

QVariant BasesModel::data(const QModelIndex& index_p, int role_p) const
{
    const auto row = index_p.row();

    if (!index_p.isValid() || row < 0 || row >= rowCount()) {
        return {};
    }

    const auto& base = bases_[static_cast<std::size_t>(row)];

    switch (role_p) {
    case id_role:
        return QVariant::fromValue(base.id_);
    case identifier_role:
        return base.identifier_;
    case column_role:
        return base.position_.column_;
    case row_role:
        return base.position_.row_;
    case size_role:
        return base.size_;
    case name_role:
        return base.name_;
    case group_role:
        return base.group_;
    case color_role:
        return base.color_;
    case health_role:
        return base.health_;
    case maximum_health_role:
        return base.maximum_health_;
    case attack_role:
        return base.attack_;
    case attack_range_role:
        return base.attack_range_;
    case spawn_creature_name_role:
        return base.spawn_creature_name_;
    case damage_amount_role:
        return base.damage_amount_;
    case damage_revision_role:
        return base.damage_revision_;
    case attack_revision_role:
        return base.attack_revision_;
    case attack_target_column_role:
        return base.attack_target_.column_;
    case attack_target_row_role:
        return base.attack_target_.row_;
    default:
        return {};
    }
}

QHash<int, QByteArray> BasesModel::roleNames() const
{
    return {
        {id_role, "baseId"},
        {identifier_role, "baseTypeIdentifier"},
        {column_role, "column"},
        {row_role, "row"},
        {size_role, "baseSize"},
        {name_role, "baseName"},
        {group_role, "baseGroup"},
        {color_role, "baseColor"},
        {health_role, "health"},
        {maximum_health_role, "maximumHealth"},
        {attack_role, "attack"},
        {attack_range_role, "attackRange"},
        {spawn_creature_name_role, "spawnCreatureName"},
        {damage_amount_role, "damageAmount"},
        {damage_revision_role, "damageRevision"},
        {attack_revision_role, "attackRevision"},
        {attack_target_column_role, "attackTargetColumn"},
        {attack_target_row_role, "attackTargetRow"}
    };
}

void BasesModel::insert_base(
    std::uint64_t id_p,
    position_t position_p,
    int size_p,
    QString identifier_p,
    QString name_p,
    QString group_p,
    QColor color_p,
    int health_p,
    int maximum_health_p,
    int attack_p,
    int attack_range_p,
    QString spawn_creature_name_p
)
{
    if (row_of(id_p) >= 0) {
        return;
    }

    const auto row = rowCount();
    beginInsertRows({}, row, row);
    bases_.push_back({
        id_p,
        position_p,
        size_p,
        std::move(identifier_p),
        std::move(name_p),
        std::move(group_p),
        std::move(color_p),
        health_p,
        maximum_health_p,
        attack_p,
        attack_range_p,
        std::move(spawn_creature_name_p),
        0,
        0,
        0,
        position_p
    });
    endInsertRows();
}

void BasesModel::update_base_health(std::uint64_t id_p, int health_p)
{
    const auto row = row_of(id_p);

    if (row < 0) {
        return;
    }

    auto& base = bases_[static_cast<std::size_t>(row)];

    if (health_p < base.health_) {
        base.damage_amount_ = base.health_ - health_p;
        ++base.damage_revision_;
    }

    base.health_ = health_p;
    notify_row_changed(row, {health_role, damage_amount_role, damage_revision_role});
}

void BasesModel::notify_base_attack(
    std::uint64_t id_p,
    position_t target_position_p
)
{
    const auto row = row_of(id_p);

    if (row < 0) {
        return;
    }

    auto& base = bases_[static_cast<std::size_t>(row)];
    base.attack_target_ = target_position_p;
    ++base.attack_revision_;
    notify_row_changed(row, {
        attack_target_column_role,
        attack_target_row_role,
        attack_revision_role
    });
}

void BasesModel::remove_base(std::uint64_t id_p)
{
    const auto row = row_of(id_p);

    if (row < 0) {
        return;
    }

    beginRemoveRows({}, row, row);
    bases_.erase(bases_.begin() + row);
    endRemoveRows();
}

int BasesModel::row_of(std::uint64_t id_p) const
{
    const auto base = std::find_if(
        bases_.begin(),
        bases_.end(),
        [id_p](const auto& entry_p) {
            return entry_p.id_ == id_p;
        }
    );

    return base == bases_.end()
        ? -1
        : static_cast<int>(std::distance(bases_.begin(), base));
}

void BasesModel::notify_row_changed(int row_p, const QList<int>& roles_p)
{
    const auto model_index = createIndex(row_p, 0);
    emit dataChanged(model_index, model_index, roles_p);
}
