#pragma once

#include <QAbstractListModel>
#include <QString>

#include <vector>

#include "creature.h"

class CorpsesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum role_t
    {
        id_role = Qt::UserRole + 1,
        identifier_role,
        column_role,
        row_role,
        removing_role
    };

    explicit CorpsesModel(QObject* parent_p = nullptr);

    int rowCount(const QModelIndex& parent_p = QModelIndex()) const override;
    QVariant data(const QModelIndex& index_p, int role_p) const override;
    QHash<int, QByteArray> roleNames() const override;

    void insert_corpse(
        std::uint64_t id_p,
        position_t position_p,
        QString identifier_p
    );
    void remove_corpse(std::uint64_t id_p);

private:
    struct corpse_entry_t
    {
        std::uint64_t id_;
        position_t position_;
        QString identifier_;
        bool removing_;
    };

    int row_of(std::uint64_t id_p) const;

    std::vector<corpse_entry_t> corpses_;
};
