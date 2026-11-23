#include "World.h"
#include <QPainter>
#include <QPixmap>
#include <QRandomGenerator>

World::World(QGraphicsScene* scene)
{
    this->scene = scene;
    buildGrid();
    paintPixmap();
}

void World::fill(int col, int row, int w, int h, char tile)
{
    for (int r = row; r < row + h && r < Rows; r++)
        for (int c = col; c < col + w && c < Cols; c++)
            grid[r][c] = tile;
}

void World::buildGrid()
{
    fill(0, 0, Cols, Rows, '.');

    // outer wall (1 tile thick): keeps both explorers inside the screen (L1-05)
    fill(0, 0, Cols, 1, '#');
    fill(0, Rows - 1, Cols, 1, '#');
    fill(0, 0, 1, Rows, '#');
    fill(Cols - 1, 0, 1, Rows, '#');

    // divider between the trail room (left) and altar room (right): columns 37-38
    // = x 592..624. Rows 19-23 (y 352..432) stay floor; Level puts the gate there.
    fill(37, 1, 2, Rows - 2, '#');
    fill(37, 19, 2, 5, '.');

    // solid rock clusters (solid objects, L1-06). All sit clear of puzzle tiles,
    // altars, the sigil, the exit and the start points.
    fill(20, 36, 3, 3, '#');     // trail room, near the bottom
    fill(30, 8, 2, 3, '#');      // trail room, top middle
    fill(52, 22, 3, 2, '#');     // altar room, right side
}

QList<QRectF> World::solidRects() const
{
    QList<QRectF> rects;
    for (int r = 0; r < Rows; r++) {
        int c = 0;
        while (c < Cols) {
            if (grid[r][c] != '#') { c++; continue; }
            int start = c;
            while (c < Cols && grid[r][c] == '#') c++;
            rects.append(QRectF(start * TileSize, OriginY + r * TileSize, (c - start) * TileSize, TileSize));
        }
    }
    // merge identical vertical neighbours (same x and width) to cut the count further
    for (int i = 0; i < rects.size(); i++) {
        for (int j = i + 1; j < rects.size(); j++) {
            if (rects[j].x() == rects[i].x() && rects[j].width() == rects[i].width()
                && rects[j].top() == rects[i].bottom()) {
                rects[i].setBottom(rects[j].bottom());
                rects.removeAt(j);
                j = i;   // rescan: the grown rectangle may touch another
            }
        }
    }
    return rects;
}

QRectF World::playArea() const
{
    return QRectF(TileSize, OriginY + TileSize, (Cols - 2) * TileSize, (Rows - 2) * TileSize);
}

QRectF World::gateRect() const
{
    return QRectF(37 * TileSize, OriginY + 19 * TileSize, 2 * TileSize, 5 * TileSize);
}

void World::paintPixmap()
{
    QPixmap pix(Cols * TileSize, Rows * TileSize);
    QPainter p(&pix);
    QRandomGenerator rng(1101);   // fixed seed: same cave on every machine

    for (int r = 0; r < Rows; r++) {
        for (int c = 0; c < Cols; c++) {
            QRect t(c * TileSize, r * TileSize, TileSize, TileSize);
            if (grid[r][c] == '#') {
                // stone block with a bright top-left edge and dark bottom-right edge
                int shade = rng.bounded(-8, 9);
                p.fillRect(t, QColor(88 + shade, 98 + shade, 84 + shade));
                p.setPen(QColor("#c0cab5")); p.drawLine(t.topLeft(), t.topRight());
                p.drawLine(t.topLeft(), t.bottomLeft());
                p.setPen(QColor("#2c3328")); p.drawLine(t.bottomLeft(), t.bottomRight());
                p.drawLine(t.topRight(), t.bottomRight());
            } else {
                // dark mossy floor (dark enough that the cream hint text stays readable)
                int shade = rng.bounded(-6, 7);
                p.fillRect(t, QColor(38 + shade, 62 + shade, 48 + shade));
                if (rng.bounded(5) == 0) {          // moss speck
                    p.setPen(QColor(70, 110, 60));
                    p.drawPoint(t.x() + rng.bounded(TileSize), t.y() + rng.bounded(TileSize));
                    p.drawPoint(t.x() + rng.bounded(TileSize), t.y() + rng.bounded(TileSize));
                }
                p.setPen(QColor(30, 50, 38));       // faint grid line
                p.drawRect(t.adjusted(0, 0, -1, -1));
            }
        }
    }
    p.end();

    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(pix);
    item->setPos(0, OriginY);
    item->setZValue(0);
    scene->addItem(item);
}