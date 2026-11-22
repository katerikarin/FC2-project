#include "Explorer.h"
#include <QBrush>
#include <QPen>

Explorer::Explorer(QColor color, int startX, int startY, int maxHealth)
{
    this->startX = startX;
    this->startY = startY;
    this->maxHealth = maxHealth;
    health = maxHealth;

    // 28x28 box, position is the top left corner
    setRect(0, 0, 28, 28);
    setBrush(QBrush(color));
    setPen(QPen(Qt::black, 2));
    setZValue(10); // explorers are drawn above the floor and puzzle tiles
    setPos(startX, startY);
}

int Explorer::getHealth()
{
    return health;
}

int Explorer::getMaxHealth()
{
    return maxHealth;
}

void Explorer::takeDamage(int amount)
{
    health = health - amount;
    if (health < 0)
        health = 0;
}

bool Explorer::isDead()
{
    return health <= 0;
}

void Explorer::reset()
{
    health = maxHealth;
    setPos(startX, startY);
}
