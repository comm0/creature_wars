#pragma once

#include <QObject>

class GameBackend : public QObject
{
    Q_OBJECT

public:
    explicit GameBackend(QObject *parent = nullptr);
};
