#include "PressurePlatePuzzle.h"

namespace {
constexpr double RequiredDualHoldSec = 3.0;
constexpr qint64 WrongFlashDurationMs = 500;
}

PressurePlatePuzzle::PressurePlatePuzzle(QGraphicsScene* scene,
                                         const QHash<char, QRectF>& plateRects,
                                         const QRectF& finalRectA,
                                         const QRectF& finalRectB,
                                         QObject* parent)
    : Puzzle(parent)
    , m_scene(scene)
    , m_platesSheet(QStringLiteral(":/sprites/plates.png"))
    , m_finalSheet(QStringLiteral(":/sprites/final_plate.png"))
    , m_plateRects(plateRects)
    , m_sequence({'c', 'a', 'e'})
    , m_finalRectA(finalRectA)
    , m_finalRectB(finalRectB)
{
    for (char id : {'a', 'b', 'c', 'd', 'e'}) {
        auto* item = new QGraphicsPixmapItem();
        item->setPos(m_plateRects.value(id).topLeft());
        item->setZValue(2);
        m_scene->addItem(item);
        m_plateItems.insert(id, item);
        setRunePlateFrame(id, 0);
    }

    m_finalItemA = new QGraphicsPixmapItem();
    m_finalItemA->setPos(m_finalRectA.topLeft());
    m_finalItemA->setZValue(2);
    m_scene->addItem(m_finalItemA);
    setFinalPlateFrame(m_finalItemA, 0);

    m_finalItemB = new QGraphicsPixmapItem();
    m_finalItemB->setPos(m_finalRectB.topLeft());
    m_finalItemB->setZValue(2);
    m_scene->addItem(m_finalItemB);
    setFinalPlateFrame(m_finalItemB, 0);
}

void PressurePlatePuzzle::setRunePlateFrame(char id, int frame)
{
    QGraphicsPixmapItem* item = m_plateItems.value(id, nullptr);
    if (!item || m_platesSheet.isNull())
        return;

    const int row = qBound(0, id - 'a', 4);
    const int col = qBound(0, frame, 3);
    item->setPixmap(m_platesSheet.copy(col * 48, row * 48, 48, 48));
}

void PressurePlatePuzzle::setFinalPlateFrame(QGraphicsPixmapItem* item, int frame)
{
    if (!item || m_finalSheet.isNull())
        return;

    const int col = qBound(0, frame, 3);
    item->setPixmap(m_finalSheet.copy(col * 48, 0, 48, 48));
}

void PressurePlatePuzzle::revealNextPlate()
{
    if (m_platesSolved || m_nextIndex < 0 || m_nextIndex >= m_sequence.size())
        return;

    setRunePlateFrame(m_sequence[m_nextIndex], 1);
}

void PressurePlatePuzzle::resetRuneSequenceOnly()
{
    m_nextIndex = 0;
    for (char id : {'a', 'b', 'c', 'd', 'e'}) {
        if (id != m_wrongPlateId)
            setRunePlateFrame(id, 0);
    }
}

void PressurePlatePuzzle::reset()
{
    solvedFlag = false;
    m_platesSolved = false;
    m_nextIndex = 0;
    m_lastSteppedPlate = 0;
    m_wrongPlateId = 0;
    m_wrongFlashTimer.invalidate();
    m_dualHoldSeconds = 0.0;
    m_holdTickTimer.invalidate();

    for (char id : {'a', 'b', 'c', 'd', 'e'})
        setRunePlateFrame(id, 0);

    setFinalPlateFrame(m_finalItemA, 0);
    setFinalPlateFrame(m_finalItemB, 0);
}

void PressurePlatePuzzle::update(Explorer* a, Explorer* b)
{
    if (isSolved() || a == nullptr || b == nullptr)
        return;

    // Clear red error highlight after 0.5 s
    if (m_wrongPlateId != 0 && m_wrongFlashTimer.isValid()
        && m_wrongFlashTimer.elapsed() >= WrongFlashDurationMs) {
        const char id = m_wrongPlateId;
        m_wrongPlateId = 0;
        m_wrongFlashTimer.invalidate();
        if (!m_platesSolved)
            setRunePlateFrame(id, 0);
    }

    // Phase 1: Fixed sequence c -> a -> e solved by Trainer 2
    if (!m_platesSolved) {
        if (b->isMoving())
            return;

        const QPointF centerB = b->sceneBoundingRect().center();
        char currentPlate = 0;
        for (char id : {'a', 'b', 'c', 'd', 'e'}) {
            if (m_plateRects.value(id).contains(centerB)) {
                currentPlate = id;
                break;
            }
        }

        if (currentPlate == 0) {
            m_lastSteppedPlate = 0;
            return;
        }

        if (currentPlate == m_lastSteppedPlate)
            return;

        m_lastSteppedPlate = currentPlate;

        const char expected = m_sequence[m_nextIndex];
        if (currentPlate == expected) {
            setRunePlateFrame(currentPlate, 2);
            ++m_nextIndex;
            if (m_nextIndex >= m_sequence.size()) {
                m_platesSolved = true;
                setFinalPlateFrame(m_finalItemA, 2);
                setFinalPlateFrame(m_finalItemB, 2);
            }
            return;
        }

        // Ignore stepping back onto an already-solved earlier plate in this run
        bool alreadySolvedInRun = false;
        for (int i = 0; i < m_nextIndex; ++i) {
            if (m_sequence[i] == currentPlate) {
                alreadySolvedInRun = true;
                break;
            }
        }
        if (alreadySolvedInRun)
            return;

        // Wrong plate: flash red, trigger spikes + damage, reset sequence
        m_wrongPlateId = currentPlate;
        m_wrongFlashTimer.start();
        setRunePlateFrame(currentPlate, 3);
        resetRuneSequenceOnly();
        emit wrongPlateTriggered();
        return;
    }

    // Phase 2: Two large final pressure plates (both players hold simultaneously for ~3 s)
    const QPointF centerA = a->sceneBoundingRect().center();
    const QPointF centerB = b->sceneBoundingRect().center();

    const bool onFinalA = m_finalRectA.contains(centerA) || m_finalRectA.contains(centerB);
    const bool onFinalB = m_finalRectB.contains(centerA) || m_finalRectB.contains(centerB);

    setFinalPlateFrame(m_finalItemA, onFinalA ? 3 : 2);
    setFinalPlateFrame(m_finalItemB, onFinalB ? 3 : 2);

    if (onFinalA && onFinalB) {
        if (!m_holdTickTimer.isValid()) {
            m_holdTickTimer.start();
        } else {
            m_dualHoldSeconds += m_holdTickTimer.restart() / 1000.0;
        }

        if (m_dualHoldSeconds >= RequiredDualHoldSec) {
            setFinalPlateFrame(m_finalItemA, 3);
            setFinalPlateFrame(m_finalItemB, 3);
            markSolved();
        }
    } else {
        m_dualHoldSeconds = 0.0;
        m_holdTickTimer.invalidate();
    }
}