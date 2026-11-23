#include "mainwindow.h"
#include <QtTest>
#include <limits>

class HealthTest : public QObject
{
    Q_OBJECT
private slots:
    void boundedDamageAndProtection();
    void puzzleHudAndRestart();
};

void HealthTest::boundedDamageAndProtection()
{
    Explorer a(Qt::green, 10, 20, 80);
    Explorer b(Qt::blue, 30, 40, 60);
    QCOMPARE(a.getHealth(), 80);
    QCOMPARE(b.getMaxHealth(), 60);
    a.takeDamage(-12);
    a.takeDamage(0);
    QCOMPARE(a.getHealth(), 80);
    a.takeDamage(10);
    a.takeDamage(10);
    b.takeDamage(10);
    QCOMPARE(a.getHealth(), 70);
    QCOMPARE(b.getHealth(), 50);
    QVERIFY(a.opacity() < 1.0 && a.opacity() > 0.0);
    QTest::qWait(780);
    a.updateDamageFeedback();
    QCOMPARE(a.opacity(), 1.0);
    a.takeDamage(std::numeric_limits<int>::max());
    QCOMPARE(a.getHealth(), 0);
    QVERIFY(a.isDead());
    a.setPos(0, 0);
    a.reset();
    QCOMPARE(a.getHealth(), 80);
    QCOMPARE(a.pos(), QPointF(10, 20));
    QCOMPARE(a.opacity(), 1.0);
    a.takeDamage(1);
    QCOMPARE(a.getHealth(), 79);
}

void HealthTest::puzzleHudAndRestart()
{
    MainWindow window;
    window.show();
    QTest::mouseClick(&window, Qt::LeftButton);
    Level* level = window.findChild<Level*>();
    QVERIFY(level);
    Explorer* a = level->getExplorerA();
    Explorer* b = level->getExplorerB();
    QList<QProgressBar*> bars = window.findChildren<QProgressBar*>();
    QCOMPARE(bars.size(), 2);
    a->setPos(220, 560); // Ali's real decoy calls takeDamage(10)
    QVERIFY(QMetaObject::invokeMethod(&window, "onGameTick"));
    QCOMPARE(bars[0]->value(), 90);
    QCOMPARE(bars[1]->value(), 100);
    window.onClueCollected(1);
    a->setPos(920, 100);
    b->setPos(925, 105);
    QVERIFY(level->bothAtExit());
    b->takeDamage(100);
    QVERIFY(QMetaObject::invokeMethod(&window, "onGameTick"));
    QCOMPARE(window.currentState(), GameState::Playing); // death wins over a valid exit
    QVERIFY(!window.hasClue(1));
    QCOMPARE(a->pos(), QPointF(80, 690));
    QCOMPARE(b->pos(), QPointF(140, 690));
    QCOMPARE(a->getHealth(), 100);
    QCOMPARE(b->getHealth(), 100);
    QCOMPARE(bars[0]->value(), a->getHealth());
    QCOMPARE(bars[1]->value(), b->getHealth());
}

QTEST_MAIN(HealthTest)
#include "health.moc"
