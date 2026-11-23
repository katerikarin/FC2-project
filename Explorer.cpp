#include "Explorer.h"
#include <QPainter>
#include <cmath>
#include <QBrush>
#include <QPen>

const int HitProtectionMs = 750;

Explorer::Explorer(QColor color, int startX, int startY, int maxHealth)
{
    this->startX = startX;
    this->startY = startY;
    this->maxHealth = qMax(1, maxHealth);
    health = this->maxHealth;

    // 28x28 box, position is the top left corner
    setRect(0, 0, 28, 28);
    setBrush(QBrush(color));
    setPen(QPen(Qt::black, 2));
    setZValue(10); // explorers are drawn above the floor and puzzle tiles
    setPos(startX, startY);
}

int Explorer::getHealth() const
{
    return health;
}

int Explorer::getMaxHealth() const
{
    return maxHealth;
}

void Explorer::takeDamage(int amount)
{
    if (amount <= 0 || isDead())
        return;
    if (timeSinceHit.isValid() && timeSinceHit.elapsed() < HitProtectionMs)
        return;

    health = qMax(0, health - amount);
    timeSinceHit.start();
    updateDamageFeedback();
}

void Explorer::updateDamageFeedback()
{
    if (!timeSinceHit.isValid())
        return;
    qint64 elapsed = timeSinceHit.elapsed();
    if (elapsed >= HitProtectionMs) {
        timeSinceHit.invalidate();
        setOpacity(1.0);
    } else {
        // Alternate between dim and bright while the explorer is protected.
        if ((elapsed / 100) % 2 == 0)
            setOpacity(0.55);
        else
            setOpacity(1.0);
    }
}

bool Explorer::isDead() const
{
    return health <= 0;
}

void Explorer::reset()
{
    timeSinceHit.invalidate();
    setOpacity(1.0);
    health = maxHealth;
    setPos(startX, startY);
    facing = Down;
    walking = false;
    poseSeconds = 0.0;
    frame = 0;
    update();
}

void Explorer::setSpriteSheet(const QString& path)
{
    spriteSheet.load(path);
    update();
}

void Explorer::animate(const QPointF& movement, double elapsedSeconds)
{
    Facing nextFacing = facing;
    bool moving = !movement.isNull();
    if (moving) {
        // Favor the vertical sprite on a diagonal. Wall sliding uses the axis that moved.
        if (qAbs(movement.x()) > qAbs(movement.y()) + 0.000001)
            nextFacing = movement.x() < 0 ? Left : Right;
        else
            nextFacing = movement.y() < 0 ? Up : Down;
    }

    if (moving != walking || nextFacing != facing)
        poseSeconds = 0.0;
    else
        poseSeconds += qMax(0.0, elapsedSeconds);

    int nextFrame = 0;
    if (moving) {
        const int steps[] = {2, 0, 3, 0};
        poseSeconds = std::fmod(poseSeconds, 0.48);
        nextFrame = steps[int(poseSeconds / 0.12)];
    } else {
        // Brief idle pose once every 1.4 seconds, still facing the last travel direction.
        poseSeconds = std::fmod(poseSeconds, 1.4);
        if (poseSeconds >= 1.2)
            nextFrame = 1;
    }

    walking = moving;
    if (nextFacing != facing || nextFrame != frame) {
        facing = nextFacing;
        frame = nextFrame;
        update();
    }
}

void Explorer::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (spriteSheet.isNull()) {
        QGraphicsRectItem::paint(painter, option, widget);
        return;
    }
    const int cellWidth = 24;
    const int cellHeight = 28;
    QRect source(frame * cellWidth, facing * cellHeight, cellWidth, cellHeight);
    painter->drawPixmap(QRect(2, 0, cellWidth, cellHeight), spriteSheet, source);
}
