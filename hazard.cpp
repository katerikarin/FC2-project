#include "Hazard.h"
#include <QPainter>
#include <QPolygonF>

const int HAZARD_COOLDOWN_MS = 750;   // same length as Ahmed's protection window
const int FLASH_MS = 200;
const int HITBOX_INSET = 4;           // forgiving: must overlap the hazard by 4 px

Hazard::Hazard(int x, int y, int w, int h, int damage)
{
    setRect(0, 0, w, h);
    setPos(x, y);
    setZValue(3);                     // above floor and puzzle tiles (2), below explorers (10)
    this->damage = damage;
}

int Hazard::getDamage() const
{
    return damage;
}

bool Hazard::touches(Explorer* explorer) const
{
    QRectF box = sceneBoundingRect().adjusted(HITBOX_INSET, HITBOX_INSET, -HITBOX_INSET, -HITBOX_INSET);
    return box.intersects(explorer->sceneBoundingRect());
}

void Hazard::tryHurt(Explorer* explorer, QElapsedTimer& cooldown)
{
    if (explorer->isDead() || !touches(explorer))
        return;
    // still protected from this hazard -> no damage this tick
    if (cooldown.isValid() && cooldown.elapsed() < HAZARD_COOLDOWN_MS)
        return;

    explorer->takeDamage(damage);
    cooldown.start();
    flashTimer.start();
    QGraphicsItem::update();
}

void Hazard::update(Explorer* a, Explorer* b)
{
    tryHurt(a, cooldownA);
    tryHurt(b, cooldownB);

    // repaint once when the flash ends
    if (flashTimer.isValid() && flashTimer.elapsed() >= FLASH_MS) {
        flashTimer.invalidate();
        QGraphicsItem::update();
    }
}

void Hazard::reset()
{
    cooldownA.invalidate();
    cooldownB.invalidate();
    flashTimer.invalidate();
    QGraphicsItem::update();
}

bool Hazard::flashing() const
{
    return flashTimer.isValid() && flashTimer.elapsed() < FLASH_MS;
}

// ---------------------------------------------------------------- spikes
SpikeHazard::SpikeHazard(int x, int y, int w, int h, int damage) : Hazard(x, y, w, h, damage) {}

void SpikeHazard::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    QRectF r = rect();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->fillRect(r, QColor("#3a3a3a"));                       // dark base plate

    const int spikeW = 16;
    int count = qMax(1, int(r.width() / spikeW));
    for (int row = 0; row < int(r.height() / spikeW); row++) {
        for (int i = 0; i < count; i++) {
            QPolygonF tri;
            double x = r.left() + i * spikeW;
            double y = r.top() + row * spikeW;
            tri << QPointF(x + 1, y + spikeW - 1) << QPointF(x + spikeW - 1, y + spikeW - 1)
                << QPointF(x + spikeW / 2.0, y + 1);
            painter->setPen(QPen(QColor("#1d1d1d"), 1));
            painter->setBrush(flashing() ? QColor("#ff5a4a") : QColor("#c9ced3"));
            painter->drawPolygon(tri);
        }
    }
    painter->setPen(QPen(flashing() ? QColor("#ff5a4a") : QColor("#8c2f2f"), 2));   // red warning border
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(r.adjusted(1, 1, -1, -1));
}

// ---------------------------------------------------------------- poison
PoisonPool::PoisonPool(int x, int y, int w, int h, int damage) : Hazard(x, y, w, h, damage) {}

void PoisonPool::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    QRectF r = rect();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(QColor("#1f5a1a"), 3));
    painter->setBrush(flashing() ? QColor("#d6ff3a") : QColor("#7bd62b"));
    painter->drawRoundedRect(r.adjusted(1, 1, -1, -1), 18, 18);

    // a few bubbles so it reads as toxic, not as grass
    painter->setPen(QPen(QColor("#e6ffb0"), 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPointF(r.left() + r.width() * 0.25, r.top() + r.height() * 0.40), 5, 5);
    painter->drawEllipse(QPointF(r.left() + r.width() * 0.55, r.top() + r.height() * 0.65), 4, 4);
    painter->drawEllipse(QPointF(r.left() + r.width() * 0.78, r.top() + r.height() * 0.35), 6, 6);
}


// ---------------------------------------------------------------- v2 sprite spikes
TileSpikeHazard::TileSpikeHazard(int x, int y, int tileSize, int damage)
    : Hazard(x, y, tileSize, tileSize, damage)
    , sheet(QStringLiteral(":/sprites/spikes.png"))
{
}

void TileSpikeHazard::triggerBurst()
{
    burstTimer.start();
    QGraphicsItem::update();
}

void TileSpikeHazard::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (sheet.isNull())
        return;

    int frame = 2; // extended spikes
    if (burstTimer.isValid()) {
        const qint64 ms = burstTimer.elapsed();
        if (ms < 120)
            frame = 1; // rising
        else if (ms < 700)
            frame = 2; // extended burst
        else
            burstTimer.invalidate();
    }

    const int w = int(rect().width());
    const int h = int(rect().height());
    painter->drawPixmap(QRect(0, 0, w, h), sheet, QRect(frame * 24, 0, 24, 24));
}

// ---------------------------------------------------------------- v2 sprite poison
TilePoisonHazard::TilePoisonHazard(int x, int y, int variant, int tileSize, int damage)
    : Hazard(x, y, tileSize, tileSize, damage)
    , sheet(QStringLiteral(":/sprites/poison_spill.png"))
    , variantIndex(qBound(0, variant, 1))
{
    animTimer.start();
}

void TilePoisonHazard::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (sheet.isNull())
        return;

    const int animFrame = int((animTimer.elapsed() / 450) % 2);
    const int col = variantIndex * 2 + animFrame;
    const int w = int(rect().width());
    const int h = int(rect().height());
    painter->drawPixmap(QRect(0, 0, w, h), sheet, QRect(col * 24, 0, 24, 24));
}
