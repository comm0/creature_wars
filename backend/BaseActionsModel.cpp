#include "BaseActionsModel.h"

#include <QByteArray>
#include <QColor>
#include <QHash>
#include <QString>
#include <QVariant>

#include <algorithm>
#include <utility>

namespace
{
QString kind_name(base_action_kind_t kind_p)
{
    switch (kind_p) {
    case base_action_kind_t::spawn:
        return QStringLiteral("spawn");
    case base_action_kind_t::training:
        return QStringLiteral("training");
    case base_action_kind_t::research:
        return QStringLiteral("research");
    case base_action_kind_t::upgrade:
        return QStringLiteral("upgrade");
    }

    return {};
}
}

BaseActionsModel::BaseActionsModel(QObject* parent_p)
    : QAbstractListModel(parent_p)
{
}

int BaseActionsModel::rowCount(const QModelIndex& parent_p) const
{
    if (parent_p.isValid()) {
        return 0;
    }

    return static_cast<int>(actions_.size());
}

QVariant BaseActionsModel::data(const QModelIndex& index_p, int role_p) const
{
    const auto row = index_p.row();

    if (!index_p.isValid() || row < 0 || row >= rowCount()) {
        return {};
    }

    const auto& action = actions_[static_cast<std::size_t>(row)];

    switch (role_p) {
    case key_role:
        return QString::fromStdString(action.key_);
    case kind_role:
        return kind_name(action.kind_);
    case name_role:
        return QString::fromStdString(action.name_);
    case subject_role:
        return QString::fromStdString(action.subject_);
    case color_role:
        return QColor::fromRgb(action.color_);
    case level_role:
        return action.level_;
    case gold_role:
        return action.cost_.cost_.gold_;
    case food_role:
        return action.cost_.cost_.food_;
    case duration_role:
        return static_cast<int>(action.cost_.duration_.count());
    case queued_role:
        return action.queued_;
    case remaining_role:
        return static_cast<int>(action.remaining_.count());
    case revision_role:
        return revision_;
    default:
        return {};
    }
}

QHash<int, QByteArray> BaseActionsModel::roleNames() const
{
    return {
        {key_role, "actionKey"},
        {kind_role, "actionKind"},
        {name_role, "actionName"},
        {subject_role, "subject"},
        {color_role, "subjectColor"},
        {level_role, "actionLevel"},
        {gold_role, "goldCost"},
        {food_role, "foodCost"},
        {duration_role, "duration"},
        {queued_role, "queued"},
        {remaining_role, "remaining"},
        {revision_role, "revision"}
    };
}

void BaseActionsModel::set_actions(std::vector<base_action_state_t> actions_p)
{
    ++revision_;
    const auto same_keys = actions_p.size() == actions_.size()
        && std::equal(
            actions_p.begin(),
            actions_p.end(),
            actions_.begin(),
            [](const auto& left_p, const auto& right_p) {
                return left_p.key_ == right_p.key_;
            }
        );

    if (!same_keys) {
        beginResetModel();
        actions_ = std::move(actions_p);
        endResetModel();
        return;
    }

    actions_ = std::move(actions_p);

    if (!actions_.empty()) {
        emit dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, 0));
    }
}
