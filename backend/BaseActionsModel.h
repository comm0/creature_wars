#pragma once

#include <QAbstractListModel>

#include <vector>

#include "base_action.h"

class BaseActionsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum role_t
    {
        key_role = Qt::UserRole + 1,
        kind_role,
        name_role,
        subject_role,
        color_role,
        level_role,
        gold_role,
        food_role,
        duration_role,
        queued_role,
        remaining_role,
        revision_role
    };

    explicit BaseActionsModel(QObject* parent_p = nullptr);

    int rowCount(const QModelIndex& parent_p = QModelIndex()) const override;
    QVariant data(const QModelIndex& index_p, int role_p) const override;
    QHash<int, QByteArray> roleNames() const override;

    void set_actions(std::vector<base_action_state_t> actions_p);

private:
    std::vector<base_action_state_t> actions_;
    int revision_ = 0;
};
