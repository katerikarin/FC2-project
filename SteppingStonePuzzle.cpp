#include "SteppingStonePuzzle.h"

SteppingStonePuzzle::SteppingStonePuzzle(QGraphicsScene* scene,
                                         const QList<QPoint>& orderedCells,
                                         const QList<QPoint>& decoyCells,
                                         int tileSize,
                                         QObject* parent)
    : Puzzle(parent)
    , m_scene(scene)
    , m_sheet(QStringLiteral(":/sprites/stepping_stones.png"))
    , m_tilePx(tileSize)
    , m_orderedCells(orderedCells)
    , m_decoyCells(decoyCells)
{
    for (const QPoint& cell : m_orderedCells) {
        auto* item = new QGraphicsPixmapItem();
        item->setPos(cell.x() * m_tilePx, cell.y() * m_tilePx);
        item->setZValue(2);
        setStoneFrame(item, 0);
        m_scene->addItem(item);
        m_orderedItems.append(item);
    }

    for (const QPoint& cell : m_decoyCells) {
        auto* item = new QGraphicsPixmapItem();
        item->setPos(cell.x() * m_tilePx, cell.y() * m_tilePx);
        item->setZValue(2);
        setStoneFrame(item, 0);
        m_scene->addItem(item);
        m_decoyItems.append(item);
    }
}

void SteppingStonePuzzle::setStoneFrame(QGraphicsPixmapItem* item, int frame)
{
    if (!item || m_sheet.isNull())
        return;
    const int f = qBound(0, frame, 2);
    item->setPixmap(m_sheet.copy(f * m_tilePx, 0, m_tilePx, m_tilePx));
}

void SteppingStonePuzzle::revealNextStone()
{
    if (isSolved() || m_nextIndex < 0 || m_nextIndex >= m_orderedItems.size())
        return;

    setStoneFrame(m_orderedItems[m_nextIndex], 1);
}

void SteppingStonePuzzle::resetSequenceOnly()
{
    m_nextIndex = 0;
    for (QGraphicsPixmapItem* item : m_orderedItems)
        setStoneFrame(item, 0);
    for (QGraphicsPixmapItem* item : m_decoyItems)
        setStoneFrame(item, 0);
}

void SteppingStonePuzzle::reset()
{
    solvedFlag = false;
    m_lastSteppedCell = QPoint(-1, -1);
    resetSequenceOnly();
}

void SteppingStonePuzzle::update(Explorer* a, Explorer* b)
{
    Q_UNUSED(a);
    if (isSolved() || b == nullptr)
        return;

    // Wait until Trainer 2 finishes stepping onto the tile
    if (b->isMoving())
        return;

    const QPoint cell = b->gridCell();
    if (cell == m_lastSteppedCell)
        return;

    m_lastSteppedCell = cell;

    // Check if Trainer 2 stepped on one of the 5 ordered stones
    const int orderedIdx = m_orderedCells.indexOf(cell);
    if (orderedIdx != -1) {
        if (orderedIdx == m_nextIndex) {
            setStoneFrame(m_orderedItems[orderedIdx], 2);
            ++m_nextIndex;
            if (m_nextIndex >= m_orderedCells.size()) {
                markSolved();
            }
        } else if (orderedIdx > m_nextIndex) {
            // Wrong order: damage Trainer 2 and reset sequence
            b->takeDamage(10);
            resetSequenceOnly();
        }
        // orderedIdx < m_nextIndex means already pressed; safe to walk across
        return;
    }

    // Check if Trainer 2 stepped on a decoy stone
    if (m_decoyCells.contains(cell)) {
        b->takeDamage(10);
        resetSequenceOnly();
    }
}