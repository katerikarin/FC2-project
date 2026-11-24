#include "Level.h"
#include "mainwindow.h"
#include "LetterTrailPuzzle.h"
#include "AltarPuzzle.h"
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
    world = nullptr;
    tileMap = nullptr;
}

void Level::loadLevel(int levelNumber)
{
    if (useLegacyWorld) {
        world = new World(scene);
        QList<QRectF> solids = world->solidRects();
        for (int i = 0; i < solids.size(); i++)
            addSolid(solids[i]);

        // the gap in the divider wall (between the trail room and the altar room)
        // is closed by the gate until the letter trail is solved
        QRectF g = world->gateRect();
        gate = addWall(int(g.x()), int(g.y()), int(g.width()), int(g.height()));
        gate->setBrush(QBrush(QColor("#8a6d3b")));
        gate->setZValue(4);

        hazards.append(new SpikeHazard(448, 544, 96, 48));    // trail room, bottom
        hazards.append(new SpikeHazard(470, 180, 96, 48));    // trail room, top
        hazards.append(new PoisonPool(700, 520, 112, 64));    // altar room, bottom
        hazards.append(new PoisonPool(800, 200, 96, 64));     // altar room, top
        for (int i = 0; i < hazards.size(); i++)
            scene->addItem(hazards[i]);

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

        // puzzle 2: the twin altars in the right room (needs both explorers)
        AltarPuzzle* altarPuzzle = new AltarPuzzle(scene, this);
        puzzles.append(altarPuzzle);
        connect(altarPuzzle, &Puzzle::solved, sigil, &Sigil::appear);
    } else {
        tileMap = new TileMap(scene);
        QList<QRectF> solids = tileMap->solidRects(entranceOpen, gateCleared);
        for (int i = 0; i < solids.size(); i++)
            addSolid(solids[i]);

        const QPointF startA = tileMap->startPosA();
        const QPointF startB = tileMap->startPosB();
        explorerA = new Explorer(QColor("#3fae5a"), int(startA.x()), int(startA.y()), Config::MaxHp, TileMap::TileSize);
        explorerB = new Explorer(QColor("#3f7fd6"), int(startB.x()), int(startB.y()), Config::MaxHp, TileMap::TileSize);
        explorerA->setSpriteSheet(QStringLiteral(":/sprites/trainer1.png"));
        explorerB->setSpriteSheet(QStringLiteral(":/sprites/trainer2.png"));
        explorerA->setFacing(Explorer::Up);
        explorerB->setFacing(Explorer::Up);
        scene->addItem(explorerA);
        scene->addItem(explorerB);
        // Hidden rock entrance (48x24, frame 0 = closed, frame 1 = open)
        entranceSheet.load(QStringLiteral(":/sprites/rock_entrance.png"));
        entranceItem = new QGraphicsPixmapItem();
        entranceItem->setPos(tileMap->entranceRect().topLeft());
        entranceItem->setZValue(3);
        scene->addItem(entranceItem);
        updateEntranceVisual();

        // Flash ability VFX (48x24 -> 2 frames of 24x24)
        flashFxSheet.load(QStringLiteral(":/fx/flash_glow.png"));
        flashFxItem = new QGraphicsPixmapItem();
        flashFxItem->setZValue(12);
        flashFxItem->hide();
        scene->addItem(flashFxItem);

        // Area 1: Stepping stones (1..5) + decoy stones (o)
        steppingPuzzle = new SteppingStonePuzzle(
            scene,
            tileMap->steppingStoneCells(),
            tileMap->decoyStoneCells(),
            TileMap::TileSize,
            this);
        puzzles.append(steppingPuzzle);
        connect(steppingPuzzle, &Puzzle::solved, this, &Level::openEntrance);
    }

    // Milestone 1 only has level 1 (mossy cave)
    if (levelNumber != 1)
        return;


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

    if (tileMap != nullptr) {
        updateExplorerGrid(explorerA, ax, ay, elapsedSeconds);
        updateExplorerGrid(explorerB, bx, by, elapsedSeconds);
    } else {
        QPointF oldA = explorerA->pos();
        QPointF oldB = explorerB->pos();
        moveExplorer(explorerA, ax, ay, elapsedSeconds);
        moveExplorer(explorerB, bx, by, elapsedSeconds);
        explorerA->animate(explorerA->pos() - oldA, elapsedSeconds);
        explorerB->animate(explorerB->pos() - oldB, elapsedSeconds);
    }

    for (int i = 0; i < hazards.size(); i++)
        hazards[i]->update(explorerA, explorerB);

    keepInside(explorerA);
    keepInside(explorerB);

    // puzzles check where the explorers are standing
    for (int i = 0; i < puzzles.size(); i++)
        puzzles[i]->update(explorerA, explorerB);
    if (sigil)
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

    for (int i = 0; i < hazards.size(); i++)
        hazards[i]->reset();

    for (int i = 0; i < puzzles.size(); i++)
        puzzles[i]->reset();
    if (sigil)
        sigil->reset();
    if (gate)
        gate->show();
    entranceOpen = false;
    gateCleared = false;
    updateEntranceVisual();
    flashFxRemaining = 0.0;
    if (flashFxItem)
        flashFxItem->hide();
    explorerA->reset();
    explorerB->reset();
    if (tileMap != nullptr) {
        explorerA->setFacing(Explorer::Up);
        explorerB->setFacing(Explorer::Up);
    }
}

void Level::clearHeldKeys()
{
    heldKeys.clear();
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

int Level::mapPixelWidth() const
{
    return tileMap ? tileMap->pixelWidth() : World::Cols * World::TileSize;
}

int Level::mapPixelHeight() const
{
    return tileMap ? tileMap->pixelHeight() : World::Rows * World::TileSize;
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

// invisible collision rectangle: World already paints the stone art, so this
// item has no brush and no pen. It must stay "visible" because hitsWall()
// ignores hidden walls (that is how the open gate stops blocking).
void Level::addSolid(const QRectF& r)
{
    QGraphicsRectItem* wall = new QGraphicsRectItem(0, 0, r.width(), r.height());
    wall->setPos(r.x(), r.y());
    wall->setBrush(Qt::NoBrush);
    wall->setPen(QPen(Qt::NoPen));
    scene->addItem(wall);
    walls.append(wall);
}

// Safety net for L1-05: if anything ever pushes an explorer past the outer wall,
// put it back inside the walkable area.
void Level::keepInside(Explorer* explorer)
{
    if (tileMap == nullptr && world == nullptr)
        return;

    QRectF area = tileMap ? tileMap->playArea() : world->playArea();
    QRectF box = explorer->sceneBoundingRect();

    double dx = 0;
    double dy = 0;
    if (box.left() < area.left())     dx = area.left() - box.left();
    if (box.right() > area.right())   dx = area.right() - box.right();
    if (box.top() < area.top())       dy = area.top() - box.top();
    if (box.bottom() > area.bottom()) dy = area.bottom() - box.bottom();

    if (dx != 0 || dy != 0)
        explorer->moveBy(dx, dy);
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

bool Level::canStepToCell(int col, int row) const
{
    if (tileMap == nullptr)
        return false;
    return tileMap->isWalkableCell(col, row, entranceOpen, gateCleared);
}

void Level::updateExplorerGrid(Explorer* explorer, int dx, int dy, double elapsedSeconds)
{
    if (explorer == nullptr)
        return;

    if (explorer->isMoving()) {
        explorer->updateStep(elapsedSeconds);
    }

    // Start a new grid step immediately when idle (seamless repeat while key is held)
    if (!explorer->isMoving() && (dx != 0 || dy != 0)) {
        int stepX = 0;
        int stepY = 0;
        if (dy != 0) {
            stepY = (dy < 0) ? -1 : 1;
            explorer->setFacing(stepY < 0 ? Explorer::Up : Explorer::Down);
        } else {
            stepX = (dx < 0) ? -1 : 1;
            explorer->setFacing(stepX < 0 ? Explorer::Left : Explorer::Right);
        }

        const QPoint cur = explorer->gridCell();
        const int nextCol = cur.x() + stepX;
        const int nextRow = cur.y() + stepY;
        if (canStepToCell(nextCol, nextRow)) {
            explorer->startStepTo(nextCol, nextRow);
        }
    }
}

void Level::openEntrance()
{
    entranceOpen = true;
    updateEntranceVisual();
}

void Level::updateEntranceVisual()
{
    if (!entranceItem || entranceSheet.isNull())
        return;
    const int frame = entranceOpen ? 1 : 0;
    entranceItem->setPixmap(entranceSheet.copy(frame * 48, 0, 48, 24));
}

void Level::triggerFlash()
{
    if (explorerA == nullptr)
        return;

    flashFxRemaining = 0.6;
    if (flashFxItem && !flashFxSheet.isNull()) {
        flashFxItem->setPos(explorerA->pos());
        flashFxItem->setPixmap(flashFxSheet.copy(0, 0, 24, 24));
        flashFxItem->show();
    }

    if (steppingPuzzle != nullptr)
        steppingPuzzle->revealNextStone();
}

void Level::updateFlashEffect(double elapsedSeconds)
{
    if (flashFxRemaining <= 0.0 || !flashFxItem)
        return;

    flashFxRemaining -= elapsedSeconds;
    if (flashFxRemaining <= 0.0) {
        flashFxRemaining = 0.0;
        flashFxItem->hide();
        return;
    }

    if (explorerA)
        flashFxItem->setPos(explorerA->pos());
    const int frame = (int(flashFxRemaining * 10.0) % 2 == 0) ? 0 : 1;
    flashFxItem->setPixmap(flashFxSheet.copy(frame * 24, 0, 24, 24));
}