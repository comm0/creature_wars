#include "CreaturesModel.h"

#include <QByteArray>
#include <QHash>
#include <QVariant>

#include <algorithm>

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
    if (!index_p.isValid() || index_p.row() >= rowCount()) {
        return {};
    }

    const auto& creature = creatures_[index_p.row()];

    switch (role_p) {
    case column_role:
        return creature.position_.column_;
    case row_role:
        return creature.position_.row_;
    default:
        return {};
    }
}

QHash<int, QByteArray> CreaturesModel::roleNames() const
{
    return {
        {column_role, "column"},
        {row_role, "row"}
    };
}

void CreaturesModel::upsert_creature(std::uint64_t id_p, position_t position_p)
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
        creatures_.push_back({id_p, position_p});
        endInsertRows();
        return;
    }

    if (creature->position_ == position_p) {
        return;
    }

    creature->position_ = position_p;
    const auto row = static_cast<int>(std::distance(creatures_.begin(), creature));
    const auto model_index = createIndex(row, 0);
    emit dataChanged(model_index, model_index, {column_role, row_role});
}
