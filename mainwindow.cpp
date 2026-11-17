#include "mainwindow.h"
#include "ui_mainwindow.h"

// ========================================================================
//  1. CONSTRUCTION & UI SETUP
//     Everything that is built exactly once lives here.
//     M2 TODO: setupMenus() (Title menu, Level Select, Replay dialog)
// ========================================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    resize(Config::WindowW, Config::WindowH);
    setWindowTitle("Ruins of the Unown - Level 1");

    setupScene();
    setupHud();
    setupOverlayBanner();
    setupTimers();

    updateOverlayLayout();
    setState(GameState::Title);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupScene() {
    // Malek (C1): the Level adds its tiles/hazards/explorers to this scene.
    scene = new QGraphicsScene(0, 0, Config::WindowW, Config::WindowH, this);
    scene->setBackgroundBrush(QColor("#1e2d24"));

    view = new QGraphicsView(scene, this);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFrameShape(QFrame::NoFrame);
    view->setFocusPolicy(Qt::NoFocus);                      // keys go to MainWindow, not the view
    view->setAttribute(Qt::WA_TransparentForMouseEvents);   // clicks go to MainWindow (click-to-start)
    setCentralWidget(view);
}

void MainWindow::setupHud() {
    hud = new HUDWidget(this);
}

void MainWindow::setupOverlayBanner() {
    overlayBanner = new QLabel(this);
    overlayBanner->setAlignment(Qt::AlignCenter);
    overlayBanner->setWordWrap(true);
    // Mouse-transparent on purpose: the click passes through to MainWindow.
    // M2: real QPushButton menus must NOT live in this label.
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
}

void MainWindow::setupTimers() {
    gameLoopTimer = new QTimer(this);
    connect(gameLoopTimer, &QTimer::timeout, this, &MainWindow::onGameTick);

    transitionTimer = new QTimer(this);
    transitionTimer->setSingleShot(true);
    connect(transitionTimer, &QTimer::timeout, this, &MainWindow::onTransitionTimerTimeout);
}

// ========================================================================
//  2. OVERLAY LAYOUT & BANNER HELPERS
//     HUD (top strip) + centred banner. Re-run on every resize.
// ========================================================================

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    updateOverlayLayout();   // was duplicated here and in updateOverlayLayout()
}

void MainWindow::updateOverlayLayout() {
    hud->setGeometry(0, 0, width(), Config::HudHeight);
    hud->raise();

    const int bannerW = (width() * 3) / 5;
    const int bannerH = height() / 3;
    overlayBanner->setGeometry((width() - bannerW) / 2, (height() - bannerH) / 2, bannerW, bannerH);
    overlayBanner->raise();
}

void MainWindow::showBanner(const QString& text) {
    overlayBanner->setText(text);
    overlayBanner->show();
    overlayBanner->raise();
}

// ========================================================================
//  3. GAME STATE MACHINE
//     Title -> Playing -> LevelComplete -> NextLevelStub
//     One enterX() per state keeps setState() a readable switch.
//     M2 TODO: add states for menus / level select / replay / game won.
// ========================================================================

void MainWindow::setState(GameState newState) {
    state = newState;

    switch (state) {
    case GameState::Title:         enterTitle();         break;
    case GameState::Playing:       enterPlaying();       break;
    case GameState::LevelComplete: enterLevelComplete(); break;
    case GameState::NextLevelStub: enterNextLevelStub(); break;
    }
}

void MainWindow::enterTitle() {
    gameLoopTimer->stop();
    transitionTimer->stop();
    hud->setLevelText("Level 1: Mossy Cave");
    showBanner("RUINS OF THE UNOWN\n\nClick Anywhere to Start Level 1");
}

void MainWindow::enterPlaying() {
    transitionTimer->stop();
    overlayBanner->hide();
    frameClock.restart();
    gameLoopTimer->start(Config::TickMs);
}

void MainWindow::enterLevelComplete() {
    gameLoopTimer->stop();

    // L1-19: clues earned this level become permanent.
    persistentClues.unite(currentLevelClues);
    currentLevelClues.clear();

    showBanner("LEVEL 1 COMPLETE!\nSigil 1 Collected.\n\nProceeding to next area...");
    transitionTimer->start(Config::CompleteBannerMs);   // L1-18
}

void MainWindow::enterNextLevelStub() {
    gameLoopTimer->stop();
    hud->setLevelText("Level 2 (Milestone 2 Stub)");
    hud->setSigilCollected(1, persistentClues.contains(1));   // L1-16: clue stays visible
    showBanner("LEVEL 2 — COMING SOON\n\n(Milestone 1 Complete)");
}

void MainWindow::onTransitionTimerTimeout() {
    if (state == GameState::LevelComplete) {
        setState(GameState::NextLevelStub);
    }
}

// ========================================================================
//  4. INPUT HANDLING
//     Mouse: only the Title click is meaningful in M1 (L1-01).
//     Keys : TEMP debug hotkeys until Ahmed's input layer is merged.
//     M2 TODO: menu navigation moves to QPushButton signals.
// ========================================================================

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

    // ---- TEMPORARY DEBUG HOTKEYS (REMOVE IN A5 - replace with real Level calls)
    if (event->key() == Qt::Key_1) {                 // damage Explorer A
        debugHpA = qMax(0, debugHpA - 50);
        hud->setPlayerHealth(1, debugHpA, Config::MaxHp);
    } else if (event->key() == Qt::Key_2) {          // damage Explorer B
        debugHpB = qMax(0, debugHpB - 50);
        hud->setPlayerHealth(2, debugHpB, Config::MaxHp);
    } else if (event->key() == Qt::Key_3) {          // collect Sigil 1
        onClueCollected(1);
    } else if (event->key() == Qt::Key_4) {          // toggle A at exit
        debugExplorerAAtExit = !debugExplorerAAtExit;
    } else if (event->key() == Qt::Key_5) {          // toggle B at exit
        debugExplorerBAtExit = !debugExplorerBAtExit;
    }

    QMainWindow::keyPressEvent(event);
}

// ========================================================================
//  5. GAME LOOP  (runs every Config::TickMs while Playing)
//     Integration order (do not reorder - spec criteria depend on it):
//       1) update world  2) death check (L1-10)  3) exit check (L1-17)
// ========================================================================

void MainWindow::onGameTick() {
    if (state != GameState::Playing) return;

    const double dt = frameClock.restart() / 1000.0;
    Q_UNUSED(dt);

    // TODO(A5): currentLevel->update(dt);   // movement, hazards, puzzles (Ahmed/Malek/Ali)

    if (isEitherExplorerDead()) {            // death BEFORE exit
        restartCurrentLevel();
        return;
    }

    if (hasClue(currentLevelNumber) && areBothExplorersAtExit()) {
        setState(GameState::LevelComplete);
    }
}

// ---- Rule predicates (swap bodies for Explorer/Level queries in A5) ------

bool MainWindow::isEitherExplorerDead() const {
    return (debugHpA <= 0) || (debugHpB <= 0);
}

bool MainWindow::areBothExplorersAtExit() const {
    return debugExplorerAAtExit && debugExplorerBAtExit;
}

// ========================================================================
//  6. LEVEL LIFECYCLE
//     M2 TODO: startLevel(n) / loadFromFile(path) (see commit 2).
// ========================================================================

void MainWindow::restartCurrentLevel() {
    currentLevelClues.clear();   // L1-19: only this level's clues are lost

    hud->resetCurrentLevelHUD(Config::MaxHp, Config::MaxHp);
    if (persistentClues.contains(1)) {
        hud->setSigilCollected(1, true);
    }

    debugHpA = Config::MaxHp;
    debugHpB = Config::MaxHp;
    debugExplorerAAtExit = false;
    debugExplorerBAtExit = false;

    frameClock.restart();
    // TODO(A5/Malek C5): currentLevel->reset();
}

// ========================================================================
//  7. CLUE STORAGE  (two-tier: persistent vs current level)
//     M2 TODO: saveProgress()/loadProgress() -> save.txt
//              (unlocked_level=3 / clues=1,2) via QFile + QTextStream.
// ========================================================================

bool MainWindow::hasClue(int clueId) const {
    return persistentClues.contains(clueId) || currentLevelClues.contains(clueId);
}

void MainWindow::onClueCollected(int clueId) {
    if (hasClue(clueId)) return;

    currentLevelClues.insert(clueId);
    hud->setSigilCollected(clueId, true);
}