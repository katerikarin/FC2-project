#include "mainwindow.h"
#include <QtTest>

class InputTest : public QObject
{
    Q_OBJECT
private slots:
    void sharedKeyboard();
};

void InputTest::sharedKeyboard()
{
    MainWindow window;
    window.show();
    QApplication::processEvents();
    Level* level = window.findChild<Level*>();
    QVERIFY(level);
    QTest::keyPress(&window, Qt::Key_A);
    QTest::mouseClick(&window, Qt::LeftButton);
    QCOMPARE(window.currentState(), GameState::Playing);
    QTest::keyClick(&window, Qt::Key_3);
    QVERIFY(!window.hasClue(1));
    Explorer* a = level->getExplorerA();
    Explorer* b = level->getExplorerB();
    const QPointF startA = a->pos();
    const QPointF startB = b->pos();
    level->update();
    QCOMPARE(a->pos(), startA); // key pressed on title

    QTest::keyPress(&window, Qt::Key_W);
    QTest::keyPress(&window, Qt::Key_Right);
    level->update();
    QVERIFY(a->y() < startA.y() && b->x() > startB.x());
    QTest::keyPress(&window, Qt::Key_S);
    QKeyEvent repeatRelease(QEvent::KeyRelease, Qt::Key_Right, Qt::NoModifier, "", true);
    QApplication::sendEvent(&window, &repeatRelease);
    QPointF firstA = a->pos();
    QPointF firstB = b->pos();
    level->update();
    QCOMPARE(a->pos(), firstA); // opposite keys cancel
    QVERIFY(b->x() > firstB.x()); // repeat release did not stop B

    QTest::keyRelease(&window, Qt::Key_S);
    QTest::keyRelease(&window, Qt::Key_Right);
    firstB = b->pos();
    level->update();
    QVERIFY(a->y() < firstA.y());
    QCOMPARE(b->pos(), firstB);

    QEvent deactivate(QEvent::WindowDeactivate);
    QApplication::sendEvent(&window, &deactivate);
    firstA = a->pos();
    QKeyEvent repeatPress(QEvent::KeyPress, Qt::Key_W, Qt::NoModifier, "", true);
    QApplication::sendEvent(&window, &repeatPress);
    level->update();
    QCOMPARE(a->pos(), firstA);

    QTest::keyPress(&window, Qt::Key_Left);
    QFocusEvent focusOut(QEvent::FocusOut);
    QApplication::sendEvent(&window, &focusOut);
    level->update();
    QCOMPARE(b->pos(), firstB);
}

QTEST_MAIN(InputTest)
#include "input.moc"
