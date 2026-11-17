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

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void onGameTick();
    void onTransitionTimerTimeout();

private:

    void setState(GameState newState);
    void updateOverlayLayout();

    Ui::MainWindow *ui;
    GameState state = GameState::Title;

    QGraphicsScene* scene;
    QGraphicsView* view;
    HUDWidget* hud;
    QLabel* overlayBanner;

    QTimer* gameLoopTimer;
    QTimer* transitionTimer;
    QElapsedTimer frameClock;
};

#endif // MAINWINDOW_H