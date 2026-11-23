#ifndef SIGIL_H
#define SIGIL_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include "Explorer.h"

// The clue item. It stays hidden until appear() is called,
// then the first explorer to touch it collects it.
class Sigil : public QObject
{
    Q_OBJECT

public:
    Sigil(QGraphicsScene* scene, int clueId, int x, int y, QObject* parent = nullptr);

    void update(Explorer* a, Explorer* b);
    void reset();

public slots:
    void appear();

signals:
    void collected(int clueId);

private:
    QGraphicsRectItem* item;
    int clueId;
    bool taken;
};

#endif // SIGIL_H
