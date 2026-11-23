#ifndef LETTERTRAILPUZZLE_H
#define LETTERTRAILPUZZLE_H

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QList>
#include <QString>
#include "Puzzle.h"

// Puzzle 1 (solo): floor tiles with Unown letters and arrows.
// Start on the glowing tile and follow the arrows to the last tile.
// Stepping on a wrong tile costs 10 health and the trail starts over.
class LetterTrailPuzzle : public Puzzle
{
    Q_OBJECT

public:
    LetterTrailPuzzle(QGraphicsScene* scene, QObject* parent = nullptr);

    void update(Explorer* a, Explorer* b);
    void reset();

private:
    QGraphicsRectItem* makeTile(QString text, int x, int y);
    QGraphicsRectItem* tileUnder(Explorer* explorer);
    void stepOn(Explorer* explorer, QGraphicsRectItem* tile);
    void clearTrail();

    QGraphicsScene* scene;
    QList<QGraphicsRectItem*> trail;   // the correct tiles, in order
    QList<QGraphicsRectItem*> decoys;  // wrong tiles
    int nextTile;                      // index of the tile that has to be stepped on next

    // the tile each explorer was standing on last tick
    QGraphicsRectItem* lastTileA;
    QGraphicsRectItem* lastTileB;
};

#endif // LETTERTRAILPUZZLE_H
