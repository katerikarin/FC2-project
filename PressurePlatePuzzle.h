#ifndef PRESSUREPLATEPUZZLE_H
#define PRESSUREPLATEPUZZLE_H

#include "Puzzle.h"
#include <QElapsedTimer>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QHash>
#include <QList>
#include <QPixmap>
#include <QRectF>

class PressurePlatePuzzle : public Puzzle
{
    Q_OBJECT

public:
    PressurePlatePuzzle(QGraphicsScene* scene,
                        const QHash<char, QRectF>& plateRects,
                        const QRectF& finalRectA,
                        const QRectF& finalRectB,
                        QObject* parent = nullptr);

    void update(Explorer* a, Explorer* b) override;
    void reset() override;

    // Called when Player 1 activates Flash (E) in Area 2
    void revealNextPlate();
    bool areRunePlatesSolved() const { return m_platesSolved; }

signals:
    void wrongPlateTriggered();

private:
    void setRunePlateFrame(char id, int frame);
    void setFinalPlateFrame(QGraphicsPixmapItem* item, int frame);
    void resetRuneSequenceOnly();

    QGraphicsScene* m_scene = nullptr;
    QPixmap m_platesSheet;
    QPixmap m_finalSheet;

    QHash<char, QRectF> m_plateRects;
    QHash<char, QGraphicsPixmapItem*> m_plateItems;
    QList<char> m_sequence;

    int m_nextIndex = 0;
    char m_lastSteppedPlate = 0;
    char m_wrongPlateId = 0;
    QElapsedTimer m_wrongFlashTimer;
    bool m_platesSolved = false;

    QRectF m_finalRectA;
    QRectF m_finalRectB;
    QGraphicsPixmapItem* m_finalItemA = nullptr;
    QGraphicsPixmapItem* m_finalItemB = nullptr;
    double m_dualHoldSeconds = 0.0;
    QElapsedTimer m_holdTickTimer;
};

#endif // PRESSUREPLATEPUZZLE_H