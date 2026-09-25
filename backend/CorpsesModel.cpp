#include "CorpsesModel.h"

#include <QByteArray>
#include <QHash>
#include <QTimer>
#include <QVariant>

#include <algorithm>
#include <utility>

namespace
{
constexpr int corpse_fade_duration = 1500;
}

CorpsesModel::CorpsesModel(QObject* parent_p)
    : QAbstractListModel(parent_p)
{
}

int CorpsesModel::rowCount(const QModelIndex& parent_p) const
{
    return parent_p.isValid() ? 0 : static_cast<int>(corpses_.size());
}

QVariant CorpsesModel::data(const QModelIndex& index_p, int role_p) const
{
    const auto row = index_p.row();

    if (!index_p.isValid() || row < 0 || row >= rowCount()) {
        return {};
    }

    const auto& corpse = corpses_[static_cast<std::size_t>(row)];

    switch (role_p) {
    case id_role:
        return QVariant::fromValue(corpse.id_);
    case identifier_role:
        return corpse.identifier_;
    case column_role:
        return corpse.position_.column_;
    case row_role:
        return corpse.position_.row_;
    case removing_role:
        return corpse.removing_;
    default:
        return {};
    }
}

QHash<int, QByteArray> CorpsesModel::roleNames() const
{
    return {
        {id_role, "corpseId"},
        {identifier_role, "creatureTypeIdentifier"},
        {column_role, "column"},
        {row_role, "row"},
        {removing_role, "removing"}
    };
}

void CorpsesModel::insert_corpse(
    std::uint64_t id_p,
    position_t position_p,
    QString identifier_p
)
{
    if (row_of(id_p) >= 0) {
        return;
    }

    const auto row = rowCount();
    beginInsertRows({}, row, row);
    corpses_.push_back({id_p, position_p, std::move(identifier_p), false});
    endInsertRows();
}

void CorpsesModel::remove_corpse(std::uint64_t id_p)
{
    const auto row = row_of(id_p);

    if (row < 0) {
        return;
    }

    auto& corpse = corpses_[static_cast<std::size_t>(row)];

    if (corpse.removing_) {
        return;
    }

    corpse.removing_ = true;
    const auto model_index = createIndex(row, 0);
    emit dataChanged(model_index, model_index, {removing_role});

    QTimer::singleShot(corpse_fade_duration, this, [this, id_p]() {
        const auto removed_row = row_of(id_p);

        if (removed_row < 0
            || !corpses_[static_cast<std::size_t>(removed_row)].removing_) {
            return;
        }

        beginRemoveRows({}, removed_row, removed_row);
        corpses_.erase(corpses_.begin() + removed_row);
        endRemoveRows();
    });
}

int CorpsesModel::row_of(std::uint64_t id_p) const
{
    const auto corpse = std::find_if(
        corpses_.begin(),
        corpses_.end(),
        [id_p](const auto& entry_p) {
            return entry_p.id_ == id_p;
        }
    );

    return corpse == corpses_.end()
        ? -1
        : static_cast<int>(std::distance(corpses_.begin(), corpse));
}
