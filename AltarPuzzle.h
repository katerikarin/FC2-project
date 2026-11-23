#ifndef ALTARPUZZLE_H
#define ALTARPUZZLE_H

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include "Puzzle.h"

// Puzzle 2 (cooperative): two altars far apart.
// Both explorers have to stand on them at the same time for 2 seconds.
// Whoever steps off early takes 10 damage and the timer starts over.
class AltarPuzzle : public Puzzle
{
    Q_OBJECT

public:
    AltarPuzzle(QGraphicsScene* scene, QObject* parent = nullptr);

    void update(Explorer* a, Explorer* b);
    void reset();

private:
    QGraphicsRectItem* makeAltar(int x, int y);
    Explorer* whoIsOn(QGraphicsRectItem* altar, Explorer* a, Explorer* b);

    QGraphicsRectItem* altar1;
    QGraphicsRectItem* altar2;

    // who was standing on each altar last tick (nullptr = nobody)
    Explorer* lastOn1;
    Explorer* lastOn2;

    int heldTicks;   // how many ticks both altars have been held together
};

#endif // ALTARPUZZLE_H
