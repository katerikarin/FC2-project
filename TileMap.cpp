#include "TileMap.h"
#include <QFile>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPainter>
#include <QTextStream>
#include <QTransform>

TileMap::TileMap(QGraphicsScene* scene, const QString& mapResourcePath)
    : m_scene(scene)
{
    loadTileCoords(QStringLiteral(":/tiles/tileset.json"));
    loadMap(mapResourcePath);
    parseObjects();
    renderStaticLayer();
}

void TileMap::loadTileCoords(const QString& jsonResourcePath)
{
    QFile file(jsonResourcePath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    const QJsonObject tilesObj = doc.object().value(QStringLiteral("tiles")).toObject();
    for (auto it = tilesObj.begin(); it != tilesObj.end(); ++it) {
        const QJsonArray arr = it.value().toArray();
        if (arr.size() >= 2)
            m_tileCoords.insert(it.key(), QPoint(arr[0].toInt(), arr[1].toInt()));
    }
}

void TileMap::loadMap(const QString& mapResourcePath)
{
    QFile file(mapResourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (!line.isEmpty())
            m_grid.append(line);
    }

    m_rows = m_grid.size();
    m_cols = m_rows > 0 ? m_grid[0].size() : 0;
}

char TileMap::cellAt(int col, int row) const
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols)
        return '#';
    return m_grid[row][col].toLatin1();
}

bool TileMap::isWallOrVoid(int col, int row) const
{
    const char ch = cellAt(col, row);
    return ch == '#' || ch == ':' || ch == 'G' || ch == 'R';
}

bool TileMap::isWalkableCell(int col, int row, bool entranceOpen, bool gateCleared) const
{
    const char ch = cellAt(col, row);
    if (ch == '#' || ch == ':')
        return false;
    if (ch == 'R')
        return entranceOpen;
    if (ch == 'G')
        return gateCleared;
    return true;
}


QList<QRectF> TileMap::solidRects(bool entranceOpen, bool gateCleared) const
{
    QList<QRectF> rects;
    for (int r = 0; r < m_rows; ++r) {
        int c = 0;
        while (c < m_cols) {
            if (isWalkableCell(c, r, entranceOpen, gateCleared)) {
                ++c;
                continue;
            }
            const int start = c;
            while (c < m_cols && !isWalkableCell(c, r, entranceOpen, gateCleared))
                ++c;
            rects.append(QRectF(start * TileSize, r * TileSize, (c - start) * TileSize, TileSize));
        }
    }
    return rects;
}

QRectF TileMap::playArea() const
{
    return QRectF(TileSize, TileSize, (m_cols - 2) * TileSize, (m_rows - 2) * TileSize);
}

QRectF TileMap::groupBounds(char target) const
{
    int minC = m_cols, maxC = -1;
    int minR = m_rows, maxR = -1;
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            if (cellAt(c, r) == target) {
                minC = qMin(minC, c);
                maxC = qMax(maxC, c);
                minR = qMin(minR, r);
                maxR = qMax(maxR, r);
            }
        }
    }
    if (maxC < minC || maxR < minR)
        return QRectF();
    return QRectF(minC * TileSize, minR * TileSize,
                  (maxC - minC + 1) * TileSize, (maxR - minR + 1) * TileSize);
}

void TileMap::parseObjects()
{
    QHash<char, QPoint> orderedStones;

    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            const char ch = cellAt(c, r);
            const QPoint cell(c, r);
            const QPointF topLeft(c * TileSize, r * TileSize);

            if (ch == 'A')
                m_startA = topLeft;
            else if (ch == 'B')
                m_startB = topLeft;
            else if (ch == 'L')
                m_sigilPos = topLeft;
            else if (ch >= '1' && ch <= '5')
                orderedStones.insert(ch, cell);
            else if (ch == 'o')
                m_decoyStones.append(cell);
            else if (ch == 'S')
                m_spikes.append(cell);
            else if (ch == 'p')
                m_poisonA.append(cell);
            else if (ch == 'q')
                m_poisonB.append(cell);
        }
    }

    for (char step = '1'; step <= '5'; ++step) {
        if (orderedStones.contains(step))
            m_steppingStones.append(orderedStones.value(step));
    }

    m_gateRect = groupBounds('G');
    m_entranceRect = groupBounds('R');
    m_finalPlateA = groupBounds('X');
    m_finalPlateB = groupBounds('Y');

    for (char id : {'a', 'b', 'c', 'd', 'e'})
        m_plates.insert(id, groupBounds(id));
}

QPixmap TileMap::extractTile(const QPixmap& sheet, const QString& name) const
{
    const QPoint gridPos = m_tileCoords.value(name, QPoint(0, 0));
    return sheet.copy(gridPos.x() * TileSize, gridPos.y() * TileSize, TileSize, TileSize);
}

QPixmap TileMap::orientDecoRock(const QPixmap& base, const QString& dirs, bool isCorner) const
{
    QImage img = base.toImage();
    if (isCorner) {
        if (dirs.contains('N') && dirs.contains('W'))
            return base;
        if (dirs.contains('N') && dirs.contains('E'))
            return QPixmap::fromImage(img.flipped(Qt::Horizontal));
        if (dirs.contains('S') && dirs.contains('W'))
            return QPixmap::fromImage(img.flipped(Qt::Vertical));
        if (dirs.contains('S') && dirs.contains('E'))
            return QPixmap::fromImage(img.transformed(QTransform().rotate(180)));
        return base;
    }

    if (dirs.contains('N'))
        return base;
    if (dirs.contains('E'))
        return QPixmap::fromImage(img.transformed(QTransform().rotate(90)));
    if (dirs.contains('S'))
        return QPixmap::fromImage(img.transformed(QTransform().rotate(180)));
    if (dirs.contains('W'))
        return QPixmap::fromImage(img.transformed(QTransform().rotate(-90)));
    return base;
}

void TileMap::renderStaticLayer()
{
    if (!m_scene || m_cols <= 0 || m_rows <= 0)
        return;

    const QPixmap tileset(QStringLiteral(":/tiles/tileset.png"));
    const QPixmap decoSheet(QStringLiteral(":/tiles/deco_rocks.png"));

    QPixmap canvas(pixelWidth(), pixelHeight());
    canvas.fill(Qt::black);
    QPainter painter(&canvas);

    // Pass 1: Draw floor tiles under every cell (walls have irregular transparent edges)
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            const char ch = cellAt(c, r);
            QString floorName;
            if (ch == 'm')
                floorName = QStringLiteral("floor_moss_0");
            else if (ch == 'n')
                floorName = QStringLiteral("floor_moss_1");
            else
                floorName = QStringLiteral("floor_%1").arg((r * 7 + c * 3) % 4);

            painter.drawPixmap(c * TileSize, r * TileSize, extractTile(tileset, floorName));
        }
    }

    // Pass 2: Autotiled walls, bottom void fade, border overlays, and decorative rocks
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            const char ch = cellAt(c, r);
            const int px = c * TileSize;
            const int py = r * TileSize;
            const int variant = (r * 5 + c * 11) % 4;

            if (ch == '#' || ch == ':') {
                if (ch == ':') {
                    int fadeDepth = 1;
                    int rr = r - 1;
                    while (rr >= 0 && cellAt(c, rr) == ':') {
                        ++fadeDepth;
                        --rr;
                    }
                    painter.drawPixmap(px, py, extractTile(tileset, QStringLiteral("fade_%1").arg(qMin(fadeDepth, 3))));
                } else {
                    int mask = 0;
                    if (!isWallOrVoid(c, r - 1)) mask |= 1; // N
                    if (!isWallOrVoid(c + 1, r)) mask |= 2; // E
                    if (!isWallOrVoid(c, r + 1)) mask |= 4; // S
                    if (!isWallOrVoid(c - 1, r)) mask |= 8; // W

                    const QString wallName = QStringLiteral("wall_v%1_mask%2")
                                                 .arg(variant)
                                                 .arg(mask, 2, 10, QLatin1Char('0'));
                    painter.drawPixmap(px, py, extractTile(tileset, wallName));

                    if (mask == 0) {
                        if (!isWallOrVoid(c + 1, r - 1))
                            painter.drawPixmap(px, py, extractTile(tileset, QStringLiteral("wall_inner_ne")));
                        if (!isWallOrVoid(c + 1, r + 1))
                            painter.drawPixmap(px, py, extractTile(tileset, QStringLiteral("wall_inner_se")));
                        if (!isWallOrVoid(c - 1, r + 1))
                            painter.drawPixmap(px, py, extractTile(tileset, QStringLiteral("wall_inner_sw")));
                        if (!isWallOrVoid(c - 1, r - 1))
                            painter.drawPixmap(px, py, extractTile(tileset, QStringLiteral("wall_inner_nw")));
                    }
                }

                if (r == 0)
                    painter.drawPixmap(px, py, extractTile(tileset, QStringLiteral("void_overlay_N")));
                if (r == m_rows - 1)
                    painter.drawPixmap(px, py, extractTile(tileset, QStringLiteral("void_overlay_S")));
                if (c == 0)
                    painter.drawPixmap(px, py, extractTile(tileset, QStringLiteral("void_overlay_W")));
                if (c == m_cols - 1)
                    painter.drawPixmap(px, py, extractTile(tileset, QStringLiteral("void_overlay_E")));
            } else if (ch == 'f' || ch == 'u' || ch == 'v') {
                const int decoIdx = (ch == 'f') ? 0 : (ch == 'u' ? 1 : 2);
                QPixmap rock = decoSheet.copy(decoIdx * TileSize, 0, TileSize, TileSize);
                if (ch != 'f') {
                    QString dirs;
                    if (isWallOrVoid(c, r - 1)) dirs += 'N';
                    if (isWallOrVoid(c + 1, r)) dirs += 'E';
                    if (isWallOrVoid(c, r + 1)) dirs += 'S';
                    if (isWallOrVoid(c - 1, r)) dirs += 'W';
                    rock = orientDecoRock(rock, dirs, ch == 'u');
                }
                painter.drawPixmap(px, py, rock);
            }
        }
    }

    painter.end();

    m_mapItem = new QGraphicsPixmapItem(canvas);
    m_mapItem->setPos(0, 0);
    m_mapItem->setZValue(0);
    m_scene->addItem(m_mapItem);
}