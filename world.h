#ifndef WORLD_H
#define WORLD_H

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QList>
#include <QRectF>

// The tile grid of the mossy cave (tasks C1 and C4).
// 16 px tiles, 64 x 45 tiles, grid starts at y = 48 so it ends exactly at the
// bottom of the 1024 x 768 scene (the HUD strip covers the top 52 px).
// '#' = wall tile, '.' = floor tile. The whole cave is painted once into one pixmap
// at z = 0, so floor and walls are clearly different and explorers/hazards draw above.
class World
{
public:
    static const int TileSize = 16;
    static const int Cols = 64;
    static const int Rows = 45;
    static const int OriginY = 48;

    World(QGraphicsScene* scene);

    // solid rectangles for collisions: horizontal wall runs are merged so there
    // are only a few dozen rectangles instead of hundreds of tiles
    QList<QRectF> solidRects() const;

    // the walkable area inside the outer wall (used to keep explorers on screen)
    QRectF playArea() const;

    // gap in the divider wall that Level closes with the gate (scene pixels)
    QRectF gateRect() const;

private:
    void buildGrid();
    void fill(int col, int row, int w, int h, char tile);
    void paintPixmap();

    QGraphicsScene* scene;
    char grid[Rows][Cols];
};

#endif // WORLD_H