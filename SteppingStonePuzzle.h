#ifndef STEPPINGSTONEPUZZLE_H
#define STEPPINGSTONEPUZZLE_H

#include "Puzzle.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QList>
#include <QPixmap>
#include <QPoint>

class SteppingStonePuzzle : public Puzzle
{
    Q_OBJECT

public:
    SteppingStonePuzzle(QGraphicsScene* scene,
                        const QList<QPoint>& orderedCells,
                        const QList<QPoint>& decoyCells,
                        int tileSize = 24,
                        QObject* parent = nullptr);

    void update(Explorer* a, Explorer* b) override;
    void reset() override;

    // Called when Player 1 activates Flash (E)
    void revealNextStone();

private:
    void setStoneFrame(QGraphicsPixmapItem* item, int frame);
    void resetSequenceOnly();

    QGraphicsScene* m_scene = nullptr;
    QPixmap m_sheet;
    int m_tilePx = 24;

    QList<QPoint> m_orderedCells;
    QList<QPoint> m_decoyCells;
    QList<QGraphicsPixmapItem*> m_orderedItems;
    QList<QGraphicsPixmapItem*> m_decoyItems;

    int m_nextIndex = 0;
    QPoint m_lastSteppedCell = QPoint(-1, -1);
};

#endif // STEPPINGSTONEPUZZLE_H