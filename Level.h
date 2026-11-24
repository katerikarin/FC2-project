#ifndef LEVEL_H
#define LEVEL_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QList>
#include <QSet>
#include <QRectF>
#include <QStringList>
#include "Explorer.h"
#include "Puzzle.h"
#include "Sigil.h"
#include "Hazard.h"
#include "World.h"
#include "TileMap.h"
#include <QGraphicsPixmapItem>
#include "SteppingStonePuzzle.h"
#include "PressurePlatePuzzle.h"

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
    void triggerFlash();
    void triggerWaterCannon();
    void requestClue();

    // keys are remembered while they are held so both players can move together
    void keyPressed(int key);
    void keyReleased(int key);
    void clearHeldKeys();

    Explorer* getExplorerA();
    Explorer* getExplorerB();
    bool bothAtExit();
    int mapPixelWidth() const;
    int mapPixelHeight() const;

signals:
    void clueCollected(int clueId);
    void dialogueRequested(const QStringList& pages, const QString& speaker = QString());

private slots:
    void openGate();
    void openEntrance();
    void weakenGate();
    void onWrongPlateTriggered();

private:
    QGraphicsRectItem* addWall(int x, int y, int w, int h);
    void addSolid(const QRectF& r);       // invisible collision rectangle; World draws the wall art
    void keepInside(Explorer* explorer);  // helper safety net: never let an explorer leave the play area
    void moveExplorer(Explorer* explorer, int dx, int dy);
    void moveExplorer(Explorer* explorer, int dx, int dy, double elapsedSeconds);
    bool hitsWall(Explorer* explorer);
    void updateExplorerGrid(Explorer* explorer, int dx, int dy, double elapsedSeconds);
    bool canStepToCell(int col, int row) const;

    void updateEntranceVisual();
    void updateFlashEffect(double elapsedSeconds);
    PressurePlatePuzzle* platePuzzle = nullptr;
    QList<TileSpikeHazard*> tileSpikes;

    QGraphicsPixmapItem* entranceItem = nullptr;
    QPixmap entranceSheet;
    QGraphicsPixmapItem* flashFxItem = nullptr;
    QPixmap flashFxSheet;
    double flashFxRemaining = 0.0;
    SteppingStonePuzzle* steppingPuzzle = nullptr;

    QGraphicsScene* scene;
    Explorer* explorerA;
    Explorer* explorerB;
    QList<QGraphicsRectItem*> walls;
    QGraphicsRectItem* exitZone;
    QGraphicsRectItem* gate;
    QSet<int> heldKeys;

    World* world;
    TileMap* tileMap;
    bool useLegacyWorld = false;
    QList<Hazard*> hazards;

    void updateGateVisual();
    void updateWaterCannonEffect(double elapsedSeconds);

    bool entranceOpen = false;
    bool gateWeakened = false;
    bool gateCleared = false;


    QGraphicsPixmapItem* gateItem = nullptr;
    QPixmap gateSheet;
    QList<QGraphicsPixmapItem*> cannonFxItems;
    QPixmap cannonFxSheet;
    double cannonFxRemaining = 0.0;
    Explorer::Facing cannonFxDir = Explorer::Up;

    QList<Puzzle*> puzzles;
    Sigil* sigil;

    void checkDialogueTriggers();

    bool area1IntroShown = false;
    bool area2IntroShown = false;
    bool gateIntroShown = false;
};

#endif // LEVEL_H