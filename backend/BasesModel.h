#pragma once

#include <QAbstractListModel>
#include <QColor>
#include <QString>

#include <vector>

#include "creature.h"

class BasesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum role_t
    {
        id_role = Qt::UserRole + 1,
        identifier_role,
        column_role,
        row_role,
        size_role,
        level_role,
        name_role,
        group_role,
        color_role,
        health_role,
        maximum_health_role,
        attack_role,
        range_role,
        damage_amount_role,
        damage_revision_role,
        attack_revision_role,
        attack_target_column_role,
        attack_target_row_role,
        removing_role,
        gold_role,
        food_role,
        gold_income_role,
        food_income_role,
        ai_difficulty_role,
        ai_strategy_role,
        human_controlled_role
    };

    explicit BasesModel(QObject* parent_p = nullptr);

    int rowCount(const QModelIndex& parent_p = QModelIndex()) const override;
    QVariant data(const QModelIndex& index_p, int role_p) const override;
    QHash<int, QByteArray> roleNames() const override;

    void insert_base(
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
    );
    void update_base(
        std::uint64_t id_p,
        int level_p,
        int health_p,
        int maximum_health_p,
        int attack_p,
        int range_p
    );
    void update_base_health(std::uint64_t id_p, int health_p);
    void update_base_controller(
        std::uint64_t id_p,
        int gold_p,
        int food_p,
        int gold_income_p,
        int food_income_p,
        QString ai_difficulty_p,
        QString ai_strategy_p,
        bool human_controlled_p
    );
    void publish_damage();
    void notify_base_attack(std::uint64_t id_p, position_t target_position_p);
    void remove_base(std::uint64_t id_p);

private:
    struct base_entry_t
    {
        std::uint64_t id_;
        position_t position_;
        int size_;
        int level_;
        QString identifier_;
        QString name_;
        QString group_;
        QColor color_;
        int health_;
        int maximum_health_;
        int attack_;
        int range_;
        int pending_damage_;
        int damage_amount_;
        int damage_revision_;
        int attack_revision_;
        position_t attack_target_;
        bool removing_;
        int gold_;
        int food_;
        int gold_income_;
        int food_income_;
        QString ai_difficulty_;
        QString ai_strategy_;
        bool human_controlled_;
    };

    int row_of(std::uint64_t id_p) const;
    void notify_row_changed(int row_p, const QList<int>& roles_p);

    std::vector<base_entry_t> bases_;
};
