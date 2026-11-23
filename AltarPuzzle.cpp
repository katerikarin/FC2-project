#include "AltarPuzzle.h"
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsEllipseItem>

const int ALTAR_SIZE = 64;
const int HOLD_TICKS = 125;      // 125 ticks * 16 ms = 2 seconds
const int STEP_OFF_DAMAGE = 10;

AltarPuzzle::AltarPuzzle(QGraphicsScene* scene, QObject* parent) : Puzzle(parent)
{
    // one altar in the top left of the altar room, one in the bottom right,
    // too far apart for one explorer to stand on both
    altar1 = makeAltar(660, 110);
    altar2 = makeAltar(920, 640);
    scene->addItem(altar1);
    scene->addItem(altar2);

    // hint: a carving between the altars
    QGraphicsSimpleTextItem* carving = new QGraphicsSimpleTextItem("Two must stand as one.\nNeither may leave until the light awakens.");
    carving->setFont(QFont("Arial", 13, -1, true));
    carving->setBrush(QBrush(QColor("#e6dfc8")));
    carving->setPos(690, 300);
    scene->addItem(carving);

    reset();
}

QGraphicsRectItem* AltarPuzzle::makeAltar(int x, int y)
{
    QGraphicsRectItem* altar = new QGraphicsRectItem(0, 0, ALTAR_SIZE, ALTAR_SIZE);
    altar->setPos(x, y);
    altar->setBrush(QBrush(QColor("#9a9486")));
    altar->setZValue(2);

    // hint: two footprints on every altar
    QGraphicsEllipseItem* leftFoot = new QGraphicsEllipseItem(18, 16, 11, 28, altar);
    QGraphicsEllipseItem* rightFoot = new QGraphicsEllipseItem(35, 20, 11, 28, altar);
    leftFoot->setBrush(QBrush(QColor("#5c574c")));
    rightFoot->setBrush(QBrush(QColor("#5c574c")));
    leftFoot->setPen(QPen(Qt::NoPen));
    rightFoot->setPen(QPen(Qt::NoPen));

    return altar;
}

Explorer* AltarPuzzle::whoIsOn(QGraphicsRectItem* altar, Explorer* a, Explorer* b)
{
    // an explorer is on the altar when its middle is inside the altar
    if (altar->sceneBoundingRect().contains(a->sceneBoundingRect().center()))
        return a;
    if (altar->sceneBoundingRect().contains(b->sceneBoundingRect().center()))
        return b;
    return nullptr;
}

void AltarPuzzle::update(Explorer* a, Explorer* b)
{
    if (solvedFlag)
        return;

    Explorer* on1 = whoIsOn(altar1, a, b);
    Explorer* on2 = whoIsOn(altar2, a, b);

    // an altar glows while someone stands on it
    if (on1 != nullptr)
        altar1->setPen(QPen(QColor("#f2c230"), 5));
    else
        altar1->setPen(QPen(QColor("#5c574c"), 2));

    if (on2 != nullptr)
        altar2->setPen(QPen(QColor("#f2c230"), 5));
    else
        altar2->setPen(QPen(QColor("#5c574c"), 2));

    if (on1 != nullptr && on2 != nullptr) {
        // both altars held: count up to 2 seconds
        heldTicks++;
        if (heldTicks >= HOLD_TICKS)
            markSolved();
    }
    else if (heldTicks > 0) {
        // the timer was running and someone stepped off too early
        if (lastOn1 != nullptr && on1 != lastOn1)
            lastOn1->takeDamage(STEP_OFF_DAMAGE);
        if (lastOn2 != nullptr && on2 != lastOn2)
            lastOn2->takeDamage(STEP_OFF_DAMAGE);
        heldTicks = 0;
    }

    lastOn1 = on1;
    lastOn2 = on2;
}

void AltarPuzzle::reset()
{
    solvedFlag = false;
    heldTicks = 0;
    lastOn1 = nullptr;
    lastOn2 = nullptr;
    altar1->setPen(QPen(QColor("#5c574c"), 2));
    altar2->setPen(QPen(QColor("#5c574c"), 2));
}
