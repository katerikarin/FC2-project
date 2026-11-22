#ifndef LEVEL_H
#define LEVEL_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QList>
#include <QSet>
#include "Explorer.h"

// Holds everything inside one level: the two explorers, the walls and the exit.
// MainWindow calls update() every game tick and reset() when the level restarts.
class Level : public QObject
{
    Q_OBJECT

public:
    Level(QGraphicsScene* scene, QObject* parent = nullptr);

    void loadLevel(int levelNumber);
    void update();
    void reset();

    // keys are remembered while they are held so both players can move together
    void keyPressed(int key);
    void keyReleased(int key);

    Explorer* getExplorerA();
    Explorer* getExplorerB();
    bool bothAtExit();

private:
    void addWall(int x, int y, int w, int h);
    void moveExplorer(Explorer* explorer, int dx, int dy);
    bool hitsWall(Explorer* explorer);

    QGraphicsScene* scene;
    Explorer* explorerA;
    Explorer* explorerB;
    QList<QGraphicsRectItem*> walls;
    QGraphicsRectItem* exitZone;
    QSet<int> heldKeys;
};

#endif // LEVEL_H
