#include "Explorer.h"
#include <QPainter>
#include <cmath>
#include <QBrush>
#include <QPen>

namespace {
constexpr int HitProtectionMs = 750;
constexpr double StepDurationSec = 0.15;
}

Explorer::Explorer(QColor color, int startX, int startY, int maxHealth, int tileSize)
{
    this->tilePx = qMax(1, tileSize);
    this->startX = startX;
    this->startY = startY;
    this->maxHealth = qMax(1, maxHealth);
    health = this->maxHealth;

    startCell = QPoint(startX / tilePx, startY / tilePx);
    currentCell = startCell;
    targetCell = startCell;

    // 28x28 box, position is the top left corner
    setRect(0, 0, tilePx, tilePx);
    setBrush(QBrush(color));
    setPen(QPen(Qt::black, 2));
    setZValue(10); // explorers are drawn above the floor and puzzle tiles
    setPos(startX, startY);
}

QRectF Explorer::boundingRect() const
{
    // Include the 32x32 sprite overflow (-4 left/right, -8 top) so Qt never clips hats/ears
    return QRectF(-4, -8, 32, 32);
}

void Explorer::setGridPosition(int col, int row)
{
    startCell = QPoint(col, row);
    currentCell = startCell;
    targetCell = startCell;
    startX = col * tilePx;
    startY = row * tilePx;
    movingStep = false;
    stepProgress = 0.0;
    setPos(startX, startY);
}

void Explorer::setFacing(Facing dir)
{
    if (facing != dir) {
        facing = dir;
        update();
    }
}

void Explorer::startStepTo(int targetCol, int targetRow)
{
    const int dc = targetCol - currentCell.x();
    const int dr = targetRow - currentCell.y();
    if (dc < 0) facing = Left;
    else if (dc > 0) facing = Right;
    else if (dr < 0) facing = Up;
    else if (dr > 0) facing = Down;

    targetCell = QPoint(targetCol, targetRow);
    stepFromPos = pos();
    stepToPos = QPointF(targetCol * tilePx, targetRow * tilePx);
    movingStep = true;
    stepProgress = 0.0;
    update();
}

bool Explorer::updateStep(double elapsedSeconds)
{
    if (!movingStep)
        return false;

    const double dt = qBound(0.0, elapsedSeconds, 0.1);
    stepProgress += dt / StepDurationSec;

    // 4-phase walk cycle across the 3 columns: step 1 -> idle -> step 2 -> idle
    walkAnimSeconds = std::fmod(walkAnimSeconds + dt, StepDurationSec * 2.0);
    const int cycle[] = {1, 0, 2, 0};
    const int nextFrame = cycle[int((walkAnimSeconds / (StepDurationSec * 2.0)) * 4.0) % 4];

    if (stepProgress >= 1.0) {
        currentCell = targetCell;
        setPos(stepToPos);
        movingStep = false;
        stepProgress = 0.0;
        frame = 0;
        update();
        return true;
    }

    const QPointF interp = stepFromPos + (stepToPos - stepFromPos) * stepProgress;
    setPos(interp);
    if (frame != nextFrame) {
        frame = nextFrame;
        update();
    }
    return false;
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
    currentCell = startCell;
    targetCell = startCell;
    movingStep = false;
    stepProgress = 0.0;
    setPos(startX, startY);
    facing = Down;
    walkAnimSeconds = 0.0;
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
    Q_UNUSED(elapsedSeconds);
    if (movingStep)
        return;

    if (!movement.isNull()) {
        if (qAbs(movement.x()) > qAbs(movement.y()) + 0.0001)
            facing = movement.x() < 0 ? Left : Right;
        else
            facing = movement.y() < 0 ? Up : Down;
    }

    if (frame != 0) {
        frame = 0;
        walkAnimSeconds = 0.0;
        update();
    }
}

void Explorer::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (spriteSheet.isNull()) {
        QGraphicsRectItem::paint(painter, option, widget);
        return;
    }

    // V2 trainer sheets: 96x128 (3 cols x 4 rows of 32x32 cells)
    if (spriteSheet.width() == 96 && spriteSheet.height() == 128) {
        const int cellSize = 32;
        const int col = qBound(0, frame, 2);
        const QRect source(col * cellSize, int(facing) * cellSize, cellSize, cellSize);
        painter->drawPixmap(QRect(-4, -8, cellSize, cellSize), spriteSheet, source);
        return;
    }

    // Legacy 96x112 fallback (24x28 cells)
    const int cellWidth = 24;
    const int cellHeight = 28;
    const QRect source(frame * cellWidth, int(facing) * cellHeight, cellWidth, cellHeight);
    painter->drawPixmap(QRect(2, 0, cellWidth, cellHeight), spriteSheet, source);
}
