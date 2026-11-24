#ifndef EXPLORER_H
#define EXPLORER_H

#include <QPixmap>
#include <QGraphicsRectItem>
#include <QColor>
#include <QElapsedTimer>
#include <QPoint>
#include <QPointF>

// The rectangle defines collisions. Painting a sprite leaves that shape intact.
class Explorer : public QGraphicsRectItem
{
public:
    enum Facing { Down = 0, Left = 1, Right = 2, Up = 3 };
    Explorer(QColor color, int startX, int startY, int maxHealth, int tileSize = 24);

    void setSpriteSheet(const QString& path);
    void animate(const QPointF& movement, double elapsedSeconds);

    // Grid-based movement with smooth visual interpolation
    void setGridPosition(int col, int row);
    QPoint gridCell() const { return currentCell; }
    Facing facingDirection() const { return facing; }
    void setFacing(Facing dir);

    bool isMoving() const { return movingStep; }
    void startStepTo(int targetCol, int targetRow);
    bool updateStep(double elapsedSeconds); // returns true when a step finishes this tick

    // health
    int getHealth() const;
    int getMaxHealth() const;
    void takeDamage(int amount);
    bool isDead() const;
    void updateDamageFeedback();

    // puts the explorer back at the start with full health
    void reset();

    QRectF boundingRect() const override;

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QElapsedTimer timeSinceHit;
    int health;
    int maxHealth;
    int tilePx;
    int startX;
    int startY;

    QPoint startCell;
    QPoint currentCell;
    QPoint targetCell;
    QPointF stepFromPos;
    QPointF stepToPos;
    bool movingStep = false;
    double stepProgress = 0.0;

    QPixmap spriteSheet;
    Facing facing = Down;
    int frame = 0;
    double walkAnimSeconds = 0.0;
};

#endif // EXPLORER_H
