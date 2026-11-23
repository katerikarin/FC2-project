#ifndef EXPLORER_H
#define EXPLORER_H

#include <QPixmap>
#include <QGraphicsRectItem>
#include <QColor>
#include <QElapsedTimer>

// The rectangle defines collisions. Painting a sprite leaves that shape intact.
class Explorer : public QGraphicsRectItem
{
public:
    Explorer(QColor color, int startX, int startY, int maxHealth);

    void setSpriteSheet(const QString& path);
    void animate(const QPointF& movement, double elapsedSeconds);

    // health
    int getHealth() const;
    int getMaxHealth() const;
    void takeDamage(int amount);
    bool isDead() const;
    void updateDamageFeedback();

    // puts the explorer back at the start with full health
    void reset();

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QElapsedTimer timeSinceHit;
    int health;
    int maxHealth;
    int startX;
    int startY;

    enum Facing { Down, Left, Right, Up }; // row order in the sprite sheet
    QPixmap spriteSheet;
    Facing facing = Down;
    bool walking = false;
    int frame = 0;
    double poseSeconds = 0.0;
};

#endif // EXPLORER_H
