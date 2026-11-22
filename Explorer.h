#ifndef EXPLORER_H
#define EXPLORER_H

#include <QGraphicsRectItem>
#include <QColor>

// One of the two players. For now it is drawn as a coloured box
// (Ahmed can swap in sprites and animation later).
class Explorer : public QGraphicsRectItem
{
public:
    Explorer(QColor color, int startX, int startY, int maxHealth);

    // health
    int getHealth();
    int getMaxHealth();
    void takeDamage(int amount);
    bool isDead();

    // puts the explorer back at the start with full health
    void reset();

private:
    int health;
    int maxHealth;
    int startX;
    int startY;
};

#endif // EXPLORER_H
