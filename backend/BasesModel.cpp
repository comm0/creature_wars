#include "BasesModel.h"

#include <QByteArray>
#include <QHash>
#include <QTimer>
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
    case level_role:
        return base.level_;
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
    case range_role:
        return base.range_;
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
    case removing_role:
        return base.removing_;
    case gold_role:
        return base.gold_;
    case food_role:
        return base.food_;
    case gold_income_role:
        return base.gold_income_;
    case food_income_role:
        return base.food_income_;
    case ai_difficulty_role:
        return base.ai_difficulty_;
    case ai_strategy_role:
        return base.ai_strategy_;
    case human_controlled_role:
        return base.human_controlled_;
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
        {level_role, "baseLevel"},
        {name_role, "baseName"},
        {group_role, "baseGroup"},
        {color_role, "baseColor"},
        {health_role, "health"},
        {maximum_health_role, "maximumHealth"},
        {attack_role, "attack"},
        {range_role, "baseRange"},
        {damage_amount_role, "damageAmount"},
        {damage_revision_role, "damageRevision"},
        {attack_revision_role, "attackRevision"},
        {attack_target_column_role, "attackTargetColumn"},
        {attack_target_row_role, "attackTargetRow"},
        {removing_role, "removing"},
        {gold_role, "baseGold"},
        {food_role, "baseFood"},
        {gold_income_role, "baseGoldIncome"},
        {food_income_role, "baseFoodIncome"},
        {ai_difficulty_role, "aiDifficulty"},
        {ai_strategy_role, "aiStrategy"},
        {human_controlled_role, "humanControlled"}
    };
}

void BasesModel::insert_base(
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
        level_p,
        std::move(identifier_p),
        std::move(name_p),
        std::move(group_p),
        std::move(color_p),
        health_p,
        maximum_health_p,
        attack_p,
        range_p,
        0,
        0,
        0,
        0,
        position_p,
        false,
        0,
        0,
        0,
        0,
        QStringLiteral("normal"),
        QStringLiteral("recover"),
        false
    });
    endInsertRows();
}

void BasesModel::update_base(
    std::uint64_t id_p,
    int level_p,
    int health_p,
    int maximum_health_p,
    int attack_p,
    int range_p
)
{
    const auto row = row_of(id_p);

    if (row < 0) {
        return;
    }

    auto& base = bases_[static_cast<std::size_t>(row)];
    base.level_ = level_p;
    base.health_ = health_p;
    base.maximum_health_ = maximum_health_p;
    base.attack_ = attack_p;
    base.range_ = range_p;
    notify_row_changed(row, {
        level_role,
        health_role,
        maximum_health_role,
        attack_role,
        range_role
    });
}

void BasesModel::update_base_health(std::uint64_t id_p, int health_p)
{
    const auto row = row_of(id_p);

    if (row < 0) {
        return;
    }

    auto& base = bases_[static_cast<std::size_t>(row)];

    if (health_p < base.health_) {
        base.pending_damage_ += base.health_ - health_p;
    }

    base.health_ = health_p;
    notify_row_changed(row, {health_role});
}

void BasesModel::publish_damage()
{
    for (std::size_t index = 0; index < bases_.size(); ++index) {
        auto& base = bases_[index];

        if (base.pending_damage_ <= 0) {
            continue;
        }

        base.damage_amount_ = base.pending_damage_;
        base.pending_damage_ = 0;
        ++base.damage_revision_;
        notify_row_changed(static_cast<int>(index), {
            damage_amount_role,
            damage_revision_role
        });
    }
}

void BasesModel::update_base_controller(
    std::uint64_t id_p,
    int gold_p,
    int food_p,
    int gold_income_p,
    int food_income_p,
    QString ai_difficulty_p,
    QString ai_strategy_p,
    bool human_controlled_p
)
{
    const auto row = row_of(id_p);

    if (row < 0) {
        return;
    }

    auto& base = bases_[static_cast<std::size_t>(row)];
    base.gold_ = gold_p;
    base.food_ = food_p;
    base.gold_income_ = gold_income_p;
    base.food_income_ = food_income_p;
    base.ai_difficulty_ = std::move(ai_difficulty_p);
    base.ai_strategy_ = std::move(ai_strategy_p);
    base.human_controlled_ = human_controlled_p;
    notify_row_changed(row, {
        gold_role,
        food_role,
        gold_income_role,
        food_income_role,
        ai_difficulty_role,
        ai_strategy_role,
        human_controlled_role
    });
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

    auto& base = bases_[static_cast<std::size_t>(row)];

    if (base.removing_) {
        return;
    }

    base.removing_ = true;
    notify_row_changed(row, {removing_role});

    QTimer::singleShot(300, this, [this, id_p]() {
        const auto removed_row = row_of(id_p);

        if (removed_row < 0
            || !bases_[static_cast<std::size_t>(removed_row)].removing_) {
            return;
        }

        beginRemoveRows({}, removed_row, removed_row);
        bases_.erase(bases_.begin() + removed_row);
        endRemoveRows();
    });
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
