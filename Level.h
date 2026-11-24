#ifndef LEVEL_H
#define LEVEL_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QList>
#include <QSet>
#include <QRectF>
#include "Explorer.h"
#include "Puzzle.h"
#include "Sigil.h"
#include "Hazard.h"
#include "World.h"
#include "TileMap.h"

// Holds everything inside one level: the two explorers, the walls, the exit and the puzzles.
// MainWindow calls update() every game tick and reset() when the level restarts.
class Level : public QObject
{
    Q_OBJECT

public:
    Level(QGraphicsScene* scene, QObject* parent = nullptr);

    void loadLevel(int levelNumber);
    void update();
    void update(double elapsedSeconds);
    void reset();

    // keys are remembered while they are held so both players can move together
    void keyPressed(int key);
    void keyReleased(int key);
    void clearHeldKeys();

    Explorer* getExplorerA();
    Explorer* getExplorerB();
    bool bothAtExit();

signals:
    void clueCollected(int clueId);

private slots:
    void openGate();

private:
    QGraphicsRectItem* addWall(int x, int y, int w, int h);
    void addSolid(const QRectF& r);       // invisible collision rectangle; World draws the wall art
    void keepInside(Explorer* explorer);  // helper safety net: never let an explorer leave the play area
    void moveExplorer(Explorer* explorer, int dx, int dy);
    void moveExplorer(Explorer* explorer, int dx, int dy, double elapsedSeconds);
    bool hitsWall(Explorer* explorer);

    QGraphicsScene* scene;
    Explorer* explorerA;
    Explorer* explorerB;
    QList<QGraphicsRectItem*> walls;
    QGraphicsRectItem* exitZone;
    QGraphicsRectItem* gate;
    QSet<int> heldKeys;

    World* world;
    TileMap* tileMap;
    bool useLegacyWorld = true;
    QList<Hazard*> hazards;

    QList<Puzzle*> puzzles;
    Sigil* sigil;
};

#endif // LEVEL_H