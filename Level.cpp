#include "Level.h"
#include "mainwindow.h"
#include "LetterTrailPuzzle.h"
#include <QBrush>
#include <QPen>

const int SPEED = 3; // pixels per tick

Level::Level(QGraphicsScene* scene, QObject* parent) : QObject(parent)
{
    this->scene = scene;
    explorerA = nullptr;
    explorerB = nullptr;
    exitZone = nullptr;
    gate = nullptr;
    sigil = nullptr;
}

void Level::loadLevel(int levelNumber)
{
    // Milestone 1 only has level 1 (mossy cave)
    if (levelNumber != 1)
        return;

    // outer walls (the HUD covers the top 52 pixels)
    addWall(0, 52, 1024, 16);
    addWall(0, 752, 1024, 16);
    addWall(0, 52, 16, 716);
    addWall(1008, 52, 16, 716);

    // wall between the trail room (left) and the altar room (right)
    // the gap from y = 360 to 440 is closed by the gate until the letter trail is solved
    addWall(600, 68, 24, 292);
    addWall(600, 440, 24, 312);
    gate = addWall(600, 360, 24, 80);
    gate->setBrush(QBrush(QColor("#8a6d3b")));

    // exit in the top right corner
    exitZone = new QGraphicsRectItem(0, 0, 90, 90);
    exitZone->setPos(900, 84);
    exitZone->setBrush(QBrush(QColor("#c9b458")));
    exitZone->setPen(QPen(Qt::NoPen));
    exitZone->setZValue(1);
    scene->addItem(exitZone);

    // the two explorers start in the bottom left
    explorerA = new Explorer(QColor("#3fae5a"), 80, 690, Config::MaxHp);
    explorerB = new Explorer(QColor("#3f7fd6"), 140, 690, Config::MaxHp);
    scene->addItem(explorerA);
    scene->addItem(explorerB);

    // puzzle 1: the letter trail in the left room
    LetterTrailPuzzle* trailPuzzle = new LetterTrailPuzzle(scene, this);
    puzzles.append(trailPuzzle);
    connect(trailPuzzle, &Puzzle::solved, this, &Level::openGate);

    // sigil 1 sits between the two altars and shows up when the altar puzzle is solved
    sigil = new Sigil(scene, 1, 816, 400, this);
    connect(sigil, &Sigil::collected, this, &Level::clueCollected);
}

void Level::update()
{
    if (explorerA == nullptr)
        return;

    // explorer A: W A S D
    int ax = 0;
    int ay = 0;
    if (heldKeys.contains(Qt::Key_A)) ax = ax - SPEED;
    if (heldKeys.contains(Qt::Key_D)) ax = ax + SPEED;
    if (heldKeys.contains(Qt::Key_W)) ay = ay - SPEED;
    if (heldKeys.contains(Qt::Key_S)) ay = ay + SPEED;

    // explorer B: arrow keys
    int bx = 0;
    int by = 0;
    if (heldKeys.contains(Qt::Key_Left)) bx = bx - SPEED;
    if (heldKeys.contains(Qt::Key_Right)) bx = bx + SPEED;
    if (heldKeys.contains(Qt::Key_Up)) by = by - SPEED;
    if (heldKeys.contains(Qt::Key_Down)) by = by + SPEED;

    moveExplorer(explorerA, ax, ay);
    moveExplorer(explorerB, bx, by);

    // puzzles check where the explorers are standing
    for (int i = 0; i < puzzles.size(); i++)
        puzzles[i]->update(explorerA, explorerB);
    sigil->update(explorerA, explorerB);
    explorerA->updateDamageFeedback();
    explorerB->updateDamageFeedback();
}

void Level::reset()
{
    if (explorerA == nullptr)
        return;

    explorerA->reset();
    explorerB->reset();
    heldKeys.clear();

    for (int i = 0; i < puzzles.size(); i++)
        puzzles[i]->reset();
    sigil->reset();
    gate->show();   // the gate closes again on a restart
}

void Level::keyPressed(int key)
{
    heldKeys.insert(key);
}

void Level::keyReleased(int key)
{
    heldKeys.remove(key);
}

Explorer* Level::getExplorerA()
{
    return explorerA;
}

Explorer* Level::getExplorerB()
{
    return explorerB;
}

bool Level::bothAtExit()
{
    if (exitZone == nullptr)
        return false;
    return explorerA->collidesWithItem(exitZone) && explorerB->collidesWithItem(exitZone);
}

QGraphicsRectItem* Level::addWall(int x, int y, int w, int h)
{
    QGraphicsRectItem* wall = new QGraphicsRectItem(0, 0, w, h);
    wall->setPos(x, y);
    wall->setBrush(QBrush(QColor("#55604f")));
    wall->setPen(QPen(Qt::NoPen));
    scene->addItem(wall);
    walls.append(wall);
    return wall;
}

void Level::openGate()
{
    // a hidden wall does not block (see hitsWall)
    gate->hide();
}

void Level::moveExplorer(Explorer* explorer, int dx, int dy)
{
    // move one direction at a time so the explorer slides along a wall instead of sticking
    explorer->moveBy(dx, 0);
    if (hitsWall(explorer))
        explorer->moveBy(-dx, 0);

    explorer->moveBy(0, dy);
    if (hitsWall(explorer))
        explorer->moveBy(0, -dy);
}

bool Level::hitsWall(Explorer* explorer)
{
    for (int i = 0; i < walls.size(); i++) {
        // a hidden wall (like an open gate) does not block
        if (walls[i]->isVisible() && explorer->collidesWithItem(walls[i]))
            return true;
    }
    return false;
}
