#ifndef TILEMAP_H
#define TILEMAP_H

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QHash>
#include <QList>
#include <QPoint>
#include <QPointF>
#include <QRectF>
#include <QStringList>

class TileMap {
public:
    static constexpr int TileSize = 24;

    explicit TileMap(QGraphicsScene* scene, const QString& mapResourcePath = QStringLiteral(":/maps/level1.txt"));

    int cols() const {return m_cols; }
    int rows() const { return m_rows; }
    int pixelWidth() const {return m_cols * TileSize;}
    int pixelHeight() const {return m_rows * TileSize;}

    char cellAt(int col, int row) const;
    bool isWallOrVoid(int col, int row) const;
    bool isWalkableCell(int col, int row, bool entranceOpen = false, bool gateCleared = false) const;

    QList<QRectF> solidRects(bool entranceOpen = false, bool gateCleared = false) const;
    QRectF playArea() const;

    QPointF startPosA() const { return m_startA; }
    QPointF startPosB() const { return m_startB; }
    QRectF gateRect() const { return m_gateRect; }
    QRectF entranceRect() const { return m_entranceRect; }
    QPointF sigilPos() const { return m_sigilPos; }
    QRectF finalPlateRectA() const { return m_finalPlateA; }
    QRectF finalPlateRectB() const { return m_finalPlateB; }
    QRectF plateRect(char id) const { return m_plates.value(id); }

    QList<QPoint> steppingStoneCells() const { return m_steppingStones; }
    QList<QPoint> decoyStoneCells() const { return m_decoyStones; }
    QList<QPoint> spikeCells() const { return m_spikes; }
    QList<QPoint> poisonCellsA() const { return m_poisonA; }
    QList<QPoint> poisonCellsB() const { return m_poisonB; }

private:
    void loadMap(const QString& mapResourcePath);
    void loadTileCoords(const QString& jsonResourcePath);
    void parseObjects();
    void renderStaticLayer();

    QPixmap extractTile(const QPixmap& sheet, const QString& name) const;
    QPixmap orientDecoRock(const QPixmap& base, const QString& dirs, bool isCorner) const;
    QRectF groupBounds(char target) const;

    QGraphicsScene* m_scene = nullptr;
    QGraphicsPixmapItem* m_mapItem = nullptr;

    QStringList m_grid;
    int m_cols = 0;
    int m_rows = 0;

    QHash<QString, QPoint> m_tileCoords;

    QPointF m_startA;
    QPointF m_startB;
    QRectF m_gateRect;
    QRectF m_entranceRect;
    QPointF m_sigilPos;
    QRectF m_finalPlateA;
    QRectF m_finalPlateB;
    QHash<char, QRectF> m_plates;

    QList<QPoint> m_steppingStones;
    QList<QPoint> m_decoyStones;
    QList<QPoint> m_spikes;
    QList<QPoint> m_poisonA;
    QList<QPoint> m_poisonB;
};

#endif // TILEMAP_H
