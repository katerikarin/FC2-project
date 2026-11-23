#include "mainwindow.h"
#include <QtTest>
#include <cmath>

class MovementTest : public QObject
{
    Q_OBJECT
private slots:
    void eightDirections();
    void timingAndWalls();
};

void MovementTest::eightDirections()
{
    QGraphicsScene scene;
    Level level(&scene);
    level.loadLevel(1);
    Explorer* a = level.getExplorerA();
    Explorer* b = level.getExplorerB();
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            level.reset();
            const QPointF startA = a->pos();
            const QPointF startB = b->pos();
            if (dx < 0) { level.keyPressed(Qt::Key_A); level.keyPressed(Qt::Key_Left); }
            if (dx > 0) { level.keyPressed(Qt::Key_D); level.keyPressed(Qt::Key_Right); }
            if (dy < 0) { level.keyPressed(Qt::Key_W); level.keyPressed(Qt::Key_Up); }
            if (dy > 0) { level.keyPressed(Qt::Key_S); level.keyPressed(Qt::Key_Down); }
            level.update(0.016);
            double length = std::hypot(dx, dy);
            QPointF expected;
            if (length > 0)
                expected = QPointF(dx * 3.0 / length, dy * 3.0 / length);
            QVERIFY(QLineF(a->pos() - startA, expected).length() < 0.00001);
            QVERIFY(QLineF(b->pos() - startB, expected).length() < 0.00001);
        }
    }
}

void MovementTest::timingAndWalls()
{
    QGraphicsScene scene;
    Level level(&scene);
    level.loadLevel(1);
    Explorer* a = level.getExplorerA();
    level.keyPressed(Qt::Key_D);
    for (int i = 0; i < 6; i++) level.update(0.016);
    const QPointF sixTicks = a->pos();
    level.reset();
    level.keyPressed(Qt::Key_D);
    level.update(0.096);
    QCOMPARE(a->pos(), sixTicks);
    for (int i = 0; i < 100; i++) level.update(0.1);
    QVERIFY(a->x() > 560 && a->sceneBoundingRect().right() <= 600);
    level.keyPressed(Qt::Key_W);
    const double oldY = a->y();
    level.update(0.1);
    QVERIFY(a->y() < oldY); // slides along the divider
    level.reset();
    level.update();
    QCOMPARE(a->pos(), QPointF(80, 690)); // held keys cleared by restart

    level.keyPressed(Qt::Key_A);
    level.keyPressed(Qt::Key_S);
    for (int i = 0; i < 100; i++) level.update(0.1);
    QVERIFY(a->sceneBoundingRect().left() >= 16 && a->x() < 22);
    QVERIFY(a->sceneBoundingRect().bottom() <= 752 && a->y() > 716);
    level.reset();
    a->setPos(900, 150);
    level.keyPressed(Qt::Key_D);
    level.keyPressed(Qt::Key_W);
    for (int i = 0; i < 100; i++) level.update(0.1);
    QVERIFY(a->sceneBoundingRect().right() <= 1008 && a->x() > 970);
    QVERIFY(a->sceneBoundingRect().top() >= 68 && a->y() < 75);
}

QTEST_MAIN(MovementTest)
#include "movement.moc"
