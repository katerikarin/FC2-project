#include "Explorer.h"
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
}
