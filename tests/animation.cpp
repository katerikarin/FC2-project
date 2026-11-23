#include "Level.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QtTest>

// Read exactly the pixels the scene would paint inside the explorer's collision box.
static QImage paintedCell(QGraphicsItem* explorer)
{
    QImage image(28, 28, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    QStyleOptionGraphicsItem option;
    explorer->paint(&painter, &option, nullptr);
    painter.end();
    return image.copy(2, 0, 24, 28);
}

static QImage cell(const QImage& sheet, int row, int col)
{
    return sheet.copy(col * 24, row * 28, 24, 28).convertToFormat(QImage::Format_ARGB32_Premultiplied);
}

class AnimationTest : public QObject
{
    Q_OBJECT
private slots:
    void bothExplorers();
};

void AnimationTest::bothExplorers()
{
    QGraphicsScene scene;
    Level level(&scene);
    level.loadLevel(1);
    Explorer* a = level.getExplorerA();
    Explorer* b = level.getExplorerB();
    QImage sheetA(":/sprites/explorer_a.png");
    QImage sheetB(":/sprites/explorer_b.png");
    QVERIFY(!sheetA.isNull() && !sheetB.isNull());
    QCOMPARE(paintedCell(a), cell(sheetA, 0, 0));
    QCOMPARE(paintedCell(b), cell(sheetB, 0, 0));
    const int keys[] = {Qt::Key_S, Qt::Key_A, Qt::Key_D, Qt::Key_W};
    for (int row = 0; row < 4; row++) {
        level.reset();
        level.keyPressed(keys[row]);
        level.update(0.016);
        QCOMPARE(paintedCell(a), cell(sheetA, row, 2));
        QCOMPARE(paintedCell(b), cell(sheetB, 0, 0));
        level.update(0.12);
        QCOMPARE(paintedCell(a), cell(sheetA, row, 0));
        level.update(0.12);
        QCOMPARE(paintedCell(a), cell(sheetA, row, 3));
        level.keyReleased(keys[row]);
        level.keyPressed(Qt::Key_Left);
        level.update(0.016);
        QCOMPARE(paintedCell(a), cell(sheetA, row, 0));
        QCOMPARE(paintedCell(b), cell(sheetB, 1, 2));
        level.update(1.25);
        QCOMPARE(paintedCell(a), cell(sheetA, row, 1));
    }

    level.reset();
    level.keyPressed(Qt::Key_A);
    for (int i = 0; i < 100; i++) level.update(0.016);
    QPointF againstWall = a->pos();
    level.update(0.016);
    QCOMPARE(a->pos(), againstWall);
    QVERIFY(paintedCell(a) == cell(sheetA, 1, 0) || paintedCell(a) == cell(sheetA, 1, 1));
    level.reset();
    QCOMPARE(paintedCell(a), cell(sheetA, 0, 0));
    QCOMPARE(paintedCell(b), cell(sheetB, 0, 0));
}

QTEST_MAIN(AnimationTest)
#include "animation.moc"
