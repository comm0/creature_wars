#pragma once

#include <QAbstractListModel>
#include <QColor>
#include <QString>

#include <vector>

#include "creature.h"

class CreaturesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum role_t
    {
        column_role = Qt::UserRole + 1,
        row_role,
        name_role,
        group_role,
        color_role,
        marker_color_role,
        health_role,
        attack_role,
        attack_range_role,
        vision_range_role
    };

    explicit CreaturesModel(QObject* parent_p = nullptr);

    int rowCount(const QModelIndex& parent_p = QModelIndex()) const override;
    QVariant data(const QModelIndex& index_p, int role_p) const override;
    QHash<int, QByteArray> roleNames() const override;

    void upsert_creature(
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
    );

private:
    struct creature_entry_t
    {
        std::uint64_t id_;
        position_t position_;
        QString name_;
        QString group_;
        QColor color_;
        QColor marker_color_;
        int health_;
        int attack_;
        int attack_range_;
        int vision_range_;
    };

    std::vector<creature_entry_t> creatures_;
};
