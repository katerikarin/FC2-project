#ifndef PUZZLE_H
#define PUZZLE_H

#include <QObject>
#include "Explorer.h"

// Base class for every puzzle. Each puzzle checks the two explorers every tick
// in update() and goes back to the start in reset().
// When it is solved it emits solved() one time only.
class Puzzle : public QObject
{
    Q_OBJECT

public:
    Puzzle(QObject* parent = nullptr);

    virtual void update(Explorer* a, Explorer* b) = 0;
    virtual void reset() = 0;

    bool isSolved();

signals:
    void solved();

protected:
    void markSolved();

    bool solvedFlag;
};

#endif // PUZZLE_H
