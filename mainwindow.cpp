#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    resize(1024,768);
    setWindowTitle("Ruins of the Unown - Level 1");

    scene = new QGraphicsScene(0,0,1024,768, this);
    scene->setBackgroundBrush(QColor("#1e2d24"));

    view = new QGraphicsView(scene, this);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    view->setFrameShape(QFrame::NoFrame);
    view->setFocusPolicy(Qt::NoFocus);
    view->setAttribute(Qt::WA_TransparentForMouseEvents);
    setCentralWidget(view);

    hud = new HUDWidget(this);

    overlayBanner = new QLabel(this);
    overlayBanner->setAlignment(Qt::AlignCenter);
    overlayBanner->setWordWrap(true);
    overlayBanner->setAttribute(Qt::WA_TransparentForMouseEvents);
    overlayBanner->setStyleSheet(
        "background-color: rgba(12, 18, 15, 220);"
        "color: #ecf0f1;"
        "font-size: 24px;"
        "font-weight: bold;"
        "border: 2px solid #4e6e58;"
        "border-radius: 10px;"
        "padding: 24px;"
    );

    gameLoopTimer = new QTimer(this);
    connect(gameLoopTimer, &QTimer::timeout, this, &MainWindow::onGameTick);

    transitionTimer = new QTimer(this);
    transitionTimer->setSingleShot(true);
    connect(transitionTimer, &QTimer::timeout, this, &MainWindow::onTransitionTimerTimeout);

    updateOverlayLayout();
    setState(GameState::Title);
}

MainWindow::~MainWindow() {
    delete ui;
}

bool MainWindow::hasClue(int clueId) const {
    return persistentClues.contains(clueId) || currentLevelClues.contains(clueId);
}

void MainWindow::onClueCollected(int clueId) {
    if (hasClue(clueId)) return;

    currentLevelClues.insert(clueId);
    hud->setSigilCollected(clueId, true);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    hud->setGeometry(0,0,width(), 52);
    hud->raise();
}

void MainWindow::setState(GameState newState) {
    state = newState;

    switch (state) {
    case GameState::Title:
        gameLoopTimer->stop();
        transitionTimer->stop();
        hud->setLevelText("Level 1: Mossy Cave");
        overlayBanner->setText("RUINS OF THE UNOWN\n\nClick Anywhere to Start Level 1");
        overlayBanner->show();
        overlayBanner->raise();
        break;

    case GameState::Playing:
        transitionTimer->stop();
        overlayBanner->hide();
        frameClock.restart();
        gameLoopTimer->start(16);
        break;

    case GameState::LevelComplete:
        gameLoopTimer->stop();

        persistentClues.unite(currentLevelClues);
        currentLevelClues.clear();

        overlayBanner->setText("LEVEL 1 COMPLETE!\nSigil 1 Collected.\n\nProceeding to next area...");
        overlayBanner->show();
        overlayBanner->raise();

        transitionTimer->start(2500);
        break;
    case GameState::NextLevelStub:
        gameLoopTimer->stop();
        hud->setLevelText("Level 2 (Milestone 2 Stub)");

        hud->setSigilCollected(1, persistentClues.contains(1));

        overlayBanner->setText("LEVEL 2 — COMING SOON\n\n(Milestone 1 Complete)");
        overlayBanner->show();
        overlayBanner->raise();
        break;
    }
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
    if (state == GameState::Title && event->button() == Qt::LeftButton) {
        setState(GameState::Playing);
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (state != GameState::Playing) {
        return;
    }
    // TEMPORARY DEBUG HOTKEYS (REMOVE LATER)
    if (event->key() == Qt::Key_1) {
        // Damage Explorer A by 50 HP
        debugHpA = qMax(0, debugHpA - 50);
        hud->setPlayerHealth(1, debugHpA, 100);
    } else if (event->key() == Qt::Key_2) {
        // Damage Explorer B by 50 HP
        debugHpB = qMax(0, debugHpB - 50);
        hud->setPlayerHealth(2, debugHpB, 100);
    } else if (event->key() == Qt::Key_3) {
        // Collect Sigil 1
        onClueCollected(1);
    } else if (event->key() == Qt::Key_4) {
        // Toggle Explorer A standing on Exit
        debugExplorerAAtExit = !debugExplorerAAtExit;
    } else if (event->key() == Qt::Key_5) {
        // Toggle Explorer B standing on Exit
        debugExplorerBAtExit = !debugExplorerBAtExit;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::onGameTick() {
    if (state != GameState::Playing) return;

    double dt = frameClock.restart() / 1000.0;
    Q_UNUSED(dt);

    // future integration hooks for the actual gameplay (malek, ali, ahmed)
    // currentLevel->update(dt)

    // update movement, hazards, and puzzles (integration hook)

    if (isEitherExplorerDead()) {
        restartCurrentLevel();
        return;
    }

    if (hasClue(currentLevelNumber) && areBothExplorersAtExit()) {
        setState(GameState::LevelComplete);
        return;
    }

}

bool MainWindow::isEitherExplorerDead() const {
    return (debugHpA <= 0) || (debugHpB <= 0);
}

bool MainWindow::areBothExplorersAtExit() const {
    return debugExplorerAAtExit && debugExplorerBAtExit;
}

void MainWindow::restartCurrentLevel() {
    currentLevelClues.clear();

    hud->resetCurrentLevelHUD(100,100);
    if (persistentClues.contains(1)) {
        hud->setSigilCollected(1, true);
    }

    debugHpA = 100;
    debugHpB = 100;
    debugExplorerAAtExit = false;
    debugExplorerBAtExit = false;

    frameClock.restart();
}

void MainWindow::onTransitionTimerTimeout() {
    if (state == GameState::LevelComplete) {
        setState(GameState::NextLevelStub);
    }
}

void MainWindow::updateOverlayLayout() {
    hud->setGeometry(0,0, width(), 52);
    hud->raise();

    int bannerW = (width() * 3) / 5;
    int bannerH = height() / 3;

    overlayBanner->setGeometry((width() - bannerW) / 2, (height() - bannerH) / 2, bannerW, bannerH);
    overlayBanner->raise();
}






















































