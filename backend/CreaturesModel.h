#pragma once

#include <QAbstractListModel>

#include <vector>

#include "creature.h"

class CreaturesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum role_t
    {
        column_role = Qt::UserRole + 1,
        row_role
    };

    explicit CreaturesModel(QObject* parent_p = nullptr);

    int rowCount(const QModelIndex& parent_p = QModelIndex()) const override;
    QVariant data(const QModelIndex& index_p, int role_p) const override;
    QHash<int, QByteArray> roleNames() const override;

    void upsert_creature(std::uint64_t id_p, position_t position_p);

private:
    struct creature_entry_t
    {
        std::uint64_t id_;
        position_t position_;
    };

    std::vector<creature_entry_t> creatures_;
};
