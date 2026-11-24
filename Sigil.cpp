#include "Sigil.h"
#include <QBrush>
#include <QPen>

Sigil::Sigil(QGraphicsScene* scene, int clueId, int x, int y, QObject* parent)
    : QObject(parent)
    , sheet(QStringLiteral(":/sprites/sigil_light.png"))
{
    this->clueId = clueId;
    taken = false;

    // Collision hitbox (24x24 centered at x, y)
    item = new QGraphicsRectItem(-12, -12, 24, 24);
    item->setPos(x, y);
    item->setZValue(5);
    item->hide();

    if (!sheet.isNull()) {
        item->setBrush(Qt::NoBrush);
        item->setPen(QPen(Qt::NoPen));

        spriteItem = new QGraphicsPixmapItem(item);
        spriteItem->setOffset(-12, -12);
        spriteItem->setPixmap(sheet.copy(0, 0, 24, 24));
    } else {
        item->setRotation(45);
        item->setBrush(QBrush(QColor("#e8c547")));
        item->setPen(QPen(QColor("#7a5c00"), 2));
    }

    scene->addItem(item);
}

void Sigil::updateAnimation()
{
    if (!spriteItem || sheet.isNull() || !animTimer.isValid())
        return;

    const int frame = int((animTimer.elapsed() / 160) % 4);
    spriteItem->setPixmap(sheet.copy(frame * 24, 0, 24, 24));
}

void Sigil::update(Explorer* a, Explorer* b)
{
    if (taken || !item->isVisible())
        return;

    updateAnimation();

    if (a->collidesWithItem(item) || b->collidesWithItem(item)) {
        taken = true;
        item->hide();
        emit collected(clueId);
    }
}

void Sigil::reset()
{
    taken = false;
    animTimer.invalidate();
    item->hide();
}

void Sigil::appear()
{
    if (!taken) {
        animTimer.start();
        updateAnimation();
        item->show();
    }
}