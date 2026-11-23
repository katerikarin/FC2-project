#include "LetterTrailPuzzle.h"
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QGraphicsSimpleTextItem>

const int TILE_SIZE = 56;
const int MISTAKE_DAMAGE = 10;

LetterTrailPuzzle::LetterTrailPuzzle(QGraphicsScene* scene, QObject* parent) : Puzzle(parent)
{
    this->scene = scene;

    // the trail spells U N O W N, each arrow points to the next tile
    trail.append(makeTile("U ↑", 100, 560));
    trail.append(makeTile("N →", 100, 440));
    trail.append(makeTile("O ↑", 220, 440));
    trail.append(makeTile("W →", 220, 320));
    trail.append(makeTile("N", 340, 320));

    // decoy tiles have letters but no arrow points at them
    decoys.append(makeTile("A", 220, 560));
    decoys.append(makeTile("E", 100, 320));
    decoys.append(makeTile("K", 340, 440));
    decoys.append(makeTile("T", 340, 200));
    decoys.append(makeTile("Z", 460, 320));

    // hint sign on the wall of the trail room
    QGraphicsSimpleTextItem* sign = new QGraphicsSimpleTextItem("Start on the glowing letter.\nFollow the arrows. Wrong letters hurt.");
    sign->setFont(QFont("Arial", 13));
    sign->setBrush(QBrush(QColor("#e6dfc8")));
    sign->setPos(40, 90);
    scene->addItem(sign);

    reset();
}

QGraphicsRectItem* LetterTrailPuzzle::makeTile(QString text, int x, int y)
{
    QGraphicsRectItem* tile = new QGraphicsRectItem(0, 0, TILE_SIZE, TILE_SIZE);
    tile->setPos(x, y);
    tile->setZValue(2);
    scene->addItem(tile);

    // the letter (and arrow) is a child of the tile so it moves with it
    QGraphicsSimpleTextItem* label = new QGraphicsSimpleTextItem(text, tile);
    label->setFont(QFont("Arial", 16, QFont::Bold));
    label->setBrush(QBrush(QColor("#2b2b2b")));
    label->setPos(8, 14);

    return tile;
}

void LetterTrailPuzzle::update(Explorer* a, Explorer* b)
{
    if (solvedFlag)
        return;

    // only react when an explorer moves onto a new tile,
    // so standing still on a tile does not count again every tick
    QGraphicsRectItem* tileA = tileUnder(a);
    if (tileA != lastTileA) {
        lastTileA = tileA;
        stepOn(a, tileA);
    }

    QGraphicsRectItem* tileB = tileUnder(b);
    if (tileB != lastTileB) {
        lastTileB = tileB;
        stepOn(b, tileB);
    }
}

QGraphicsRectItem* LetterTrailPuzzle::tileUnder(Explorer* explorer)
{
    // an explorer is on a tile when the middle of the explorer is inside it
    QPointF middle = explorer->sceneBoundingRect().center();

    for (int i = 0; i < trail.size(); i++) {
        if (trail[i]->sceneBoundingRect().contains(middle))
            return trail[i];
    }
    for (int i = 0; i < decoys.size(); i++) {
        if (decoys[i]->sceneBoundingRect().contains(middle))
            return decoys[i];
    }
    return nullptr;
}

void LetterTrailPuzzle::stepOn(Explorer* explorer, QGraphicsRectItem* tile)
{
    if (tile == nullptr || solvedFlag)
        return;

    int index = trail.indexOf(tile);   // -1 means it is a decoy

    if (index == nextTile) {
        // right tile: light it up and move on
        tile->setBrush(QBrush(QColor("#8fd18a")));
        nextTile++;
        if (nextTile == trail.size())
            markSolved();
    }
    else if (index == -1 || index > nextTile) {
        // wrong tile: lose health and start over
        explorer->takeDamage(MISTAKE_DAMAGE);
        clearTrail();
    }
    // index < nextTile means going back over a lit tile, that is fine
}

void LetterTrailPuzzle::clearTrail()
{
    nextTile = 0;

    for (int i = 0; i < trail.size(); i++) {
        trail[i]->setBrush(QBrush(QColor("#b7ad93")));
        trail[i]->setPen(QPen(QColor("#6b6350"), 2));
    }
    for (int i = 0; i < decoys.size(); i++) {
        decoys[i]->setBrush(QBrush(QColor("#b7ad93")));
        decoys[i]->setPen(QPen(QColor("#6b6350"), 2));
    }

    // the first tile glows so the players know where to start
    trail[0]->setPen(QPen(QColor("#f2c230"), 5));
}

void LetterTrailPuzzle::reset()
{
    solvedFlag = false;
    lastTileA = nullptr;
    lastTileB = nullptr;
    clearTrail();
}
