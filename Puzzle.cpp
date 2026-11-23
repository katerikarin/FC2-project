#include "Puzzle.h"

Puzzle::Puzzle(QObject* parent) : QObject(parent)
{
    solvedFlag = false;
}

bool Puzzle::isSolved()
{
    return solvedFlag;
}

void Puzzle::markSolved()
{
    // only tell the level once, even though update() keeps running
    if (!solvedFlag) {
        solvedFlag = true;
        emit solved();
    }
}
