#ifndef HAZARD_H
#define HAZARD_H

#include <QGraphicsRectItem>
#include <QElapsedTimer>
#include <QColor>
#include "Explorer.h"

// Base class for every stationary damage source (task C3).
// Level calls update(a, b) every tick and reset() on a level restart.
// Damage is applied in steps, not every frame: each hazard keeps a per-explorer
// cooldown (L1-08). Explorer::takeDamage has its own protection window too

class Hazard : public QGraphicsRectItem
{
public:
    Hazard(int x, int y, int w, int h, int damage);

    void update(Explorer* a, Explorer* b);
    void reset();

    int getDamage() const;

protected:
    // subclasses draw their own look; flashing() is true right after a hit
    bool flashing() const;

private:
    void tryHurt(Explorer* explorer, QElapsedTimer& cooldown);
    bool touches(Explorer* explorer) const;

    int damage;
    QElapsedTimer cooldownA;     // last time this hazard hurt explorer A
    QElapsedTimer cooldownB;     // last time this hazard hurt explorer B
    QElapsedTimer flashTimer;    // visual feedback on the hazard itself
};

// Metal spikes on the floor.
class SpikeHazard : public Hazard
{
public:
    SpikeHazard(int x, int y, int w, int h, int damage = 10);
protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
};

// Green poison pool. Weaker than spikes, so it reads as a different hazard type.
class PoisonPool : public Hazard
{
public:
    PoisonPool(int x, int y, int w, int h, int damage = 5);
protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
};

#endif // HAZARD_H
