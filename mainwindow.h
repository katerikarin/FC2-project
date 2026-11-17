#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QElapsedTimer.h>
#include <QLabel>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include "HUDWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum class GameState {
    Title,
    Playing,
    LevelComplete,
    NextLevelStub
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    GameState currentState() const { return state; }
    bool hasClue(int clueId) const;

public slots:
    void onClueCollected(int clueId);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void onGameTick();
    void onTransitionTimerTimeout();

private:

    void setState(GameState newState);
    void restartCurrentLevel();
    void updateOverlayLayout();

    bool isEitherExplorerDead() const;
    bool areBothExplorersAtExit() const;

    Ui::MainWindow *ui;
    GameState state = GameState::Title;
    int currentLevelNumber = 1;

    QGraphicsScene* scene;
    QGraphicsView* view;
    HUDWidget* hud;
    QLabel* overlayBanner;

    QTimer* gameLoopTimer;
    QTimer* transitionTimer;
    QElapsedTimer frameClock;

    QSet<int> persistentClues;
    QSet<int> currentLevelClues;

    int debugHpA = 100;
    int debugHpB = 100;
    bool debugExplorerAAtExit = false;
    bool debugExplorerBAtExit = false;
};

#endif // MAINWINDOW_H