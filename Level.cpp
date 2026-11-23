#include "Level.h"
#include "mainwindow.h"
#include "LetterTrailPuzzle.h"
#include <QBrush>
#include <QPen>
#include <cmath>

const double WalkSpeed = 187.5; // 3 scene pixels in a nominal 16 ms tick

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
    explorerA->setSpriteSheet(QStringLiteral(":/sprites/explorer_a.png"));
    explorerB->setSpriteSheet(QStringLiteral(":/sprites/explorer_b.png"));
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
    update(Config::TickMs / 1000.0);
}

void Level::update(double elapsedSeconds)
{
    if (explorerA == nullptr)
        return;

    // explorer A: W A S D
    int ax = 0;
    int ay = 0;
    if (heldKeys.contains(Qt::Key_A)) ax = ax - 1;
    if (heldKeys.contains(Qt::Key_D)) ax = ax + 1;
    if (heldKeys.contains(Qt::Key_W)) ay = ay - 1;
    if (heldKeys.contains(Qt::Key_S)) ay = ay + 1;

    // explorer B: arrow keys
    int bx = 0;
    int by = 0;
    if (heldKeys.contains(Qt::Key_Left)) bx = bx - 1;
    if (heldKeys.contains(Qt::Key_Right)) bx = bx + 1;
    if (heldKeys.contains(Qt::Key_Up)) by = by - 1;
    if (heldKeys.contains(Qt::Key_Down)) by = by + 1;

    QPointF oldA = explorerA->pos();
    QPointF oldB = explorerB->pos();
    moveExplorer(explorerA, ax, ay, elapsedSeconds);
    moveExplorer(explorerB, bx, by, elapsedSeconds);
    explorerA->animate(explorerA->pos() - oldA, elapsedSeconds);
    explorerB->animate(explorerB->pos() - oldB, elapsedSeconds);

    // puzzles check where the explorers are standing
    for (int i = 0; i < puzzles.size(); i++)
        puzzles[i]->update(explorerA, explorerB);
    sigil->update(explorerA, explorerB);
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

void Level::moveExplorer(Explorer* explorer, int dx, int dy, double elapsedSeconds)
{
    if (dx == 0 && dy == 0)
        return;

    // Limit catch-up after a long pause. A diagonal has length sqrt(2).
    double distance = WalkSpeed * qBound(0.0, elapsedSeconds, 0.1);
    if (dx != 0 && dy != 0)
        distance /= std::sqrt(2.0);

    // Small axis-by-axis steps reuse Malek's collision checks and allow wall sliding.
    while (distance > 0.0) {
        double step = qMin(3.0, distance);
        QPointF before = explorer->pos();
        explorer->moveBy(dx * step, 0);
        if (hitsWall(explorer))
            explorer->setX(before.x());
        explorer->moveBy(0, dy * step);
        if (hitsWall(explorer))
            explorer->setY(before.y());
        distance -= step;
    }
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
