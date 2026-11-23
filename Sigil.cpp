#include "Sigil.h"
#include <QBrush>
#include <QPen>

Sigil::Sigil(QGraphicsScene* scene, int clueId, int x, int y, QObject* parent) : QObject(parent)
{
    this->clueId = clueId;
    taken = false;

    // gold square turned 45 degrees so it looks like a gem
    item = new QGraphicsRectItem(-12, -12, 24, 24);
    item->setPos(x, y);
    item->setRotation(45);
    item->setBrush(QBrush(QColor("#e8c547")));
    item->setPen(QPen(QColor("#7a5c00"), 2));
    item->setZValue(5);
    item->hide();
    scene->addItem(item);
}

void Sigil::update(Explorer* a, Explorer* b)
{
    if (taken || !item->isVisible())
        return;

    if (a->collidesWithItem(item) || b->collidesWithItem(item)) {
        taken = true;
        item->hide();
        emit collected(clueId);
    }
}

void Sigil::reset()
{
    taken = false;
    item->hide();
}

void Sigil::appear()
{
    if (!taken)
        item->show();
}
