#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>

namespace {
bool isMovementKey(int key) {
    return key == Qt::Key_W || key == Qt::Key_A || key == Qt::Key_S || key == Qt::Key_D
        || key == Qt::Key_Up || key == Qt::Key_Left || key == Qt::Key_Down || key == Qt::Key_Right;
}

// M2 TODO: replace with names read from the level definition (data-driven bonus).
QString levelTitle(int n) {
    return n == 1 ? QStringLiteral("Level 1: Mossy Cave")
                  : QStringLiteral("Level %1").arg(n);
}
}

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
    // Reserve the HUD strip in screen pixels, then fit the playable cave below it.
    QVBoxLayout* layout = new QVBoxLayout(ui->centralwidget);
    layout->setContentsMargins(0, Config::HudHeight, 0, 0);
    layout->addWidget(view);
    view->setSceneRect(scene->sceneRect().adjusted(0, Config::HudHeight, 0, 0));
    view->viewport()->installEventFilter(this);

    level = new Level(scene, this);
    connect(level, &Level::clueCollected, this, &MainWindow::onClueCollected);
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

bool MainWindow::eventFilter(QObject* object, QEvent* event) {
    if (event->type() == QEvent::Resize && view && object == view->viewport())
        view->fitInView(view->sceneRect(), Qt::KeepAspectRatio);
    return QMainWindow::eventFilter(object, event);
}

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
    if (newState != GameState::Playing)
        level->clearHeldKeys();
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
    hud->setLevelText(levelTitle(1));
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

    showBanner(QString("LEVEL %1 COMPLETE!\nSigil %1 Collected.\n\nProceeding to next area...")
                   .arg(currentLevelNumber));
    transitionTimer->start(Config::CompleteBannerMs);   // L1-18
}

void MainWindow::enterNextLevelStub() {
    gameLoopTimer->stop();
    hud->setLevelText(levelTitle(currentLevelNumber + 1) + " (Milestone 2 Stub)");
    refreshClueHud();   // L1-16: earned clues stay visible
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
//     Keys: WASD for Explorer A, arrows for Explorer B.
//     M2 TODO: menu navigation moves to QPushButton signals.
// ========================================================================

void MainWindow::mousePressEvent(QMouseEvent* event) {
    if (state == GameState::Title && event->button() == Qt::LeftButton) {
        startLevel(1);   // M2: Level Select / Replay buttons call startLevel(n) instead
    }
    QMainWindow::mousePressEvent(event);
}

bool MainWindow::event(QEvent* event) {
    // A release may go to another window or widget after focus changes.
    if (level && (event->type() == QEvent::WindowDeactivate || event->type() == QEvent::FocusOut))
        level->clearHeldKeys();
    return QMainWindow::event(event);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (state == GameState::Playing && isMovementKey(event->key())) {
        if (!event->isAutoRepeat())
            level->keyPressed(event->key());
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent* event) {
    if (state == GameState::Playing && isMovementKey(event->key())) {
        if (!event->isAutoRepeat())
            level->keyReleased(event->key());
        event->accept();
        return;
    }
    QMainWindow::keyReleaseEvent(event);
}

// ========================================================================
//  5. GAME LOOP  (runs every Config::TickMs while Playing)
//     Integration order (do not reorder - spec criteria depend on it):
//       1) update world  2) death check (L1-10)  3) exit check (L1-17)
// ========================================================================

void MainWindow::onGameTick() {
    if (state != GameState::Playing) return;

    const double dt = frameClock.restart() / 1000.0;
    level->update(dt);   // movement, hazards, puzzles

    Explorer* a = level->getExplorerA();
    Explorer* b = level->getExplorerB();
    hud->setPlayerHealth(1, a->getHealth(), a->getMaxHealth());
    hud->setPlayerHealth(2, b->getHealth(), b->getMaxHealth());

    if (isEitherExplorerDead()) {            // death BEFORE exit
        restartCurrentLevel();
        return;
    }

    if (isLevelWon()) {
        setState(GameState::LevelComplete);
    }
}

// ---- Rule predicates (swap bodies for Explorer/Level queries in A5) ------

bool MainWindow::isEitherExplorerDead() const {
    return level->getExplorerA()->isDead() || level->getExplorerB()->isDead();
}

bool MainWindow::areBothExplorersAtExit() const {
    return level->bothAtExit();
}

bool MainWindow::isLevelWon() const {
    // L1-17: a clue AND both explorers on the exit. Clue id == level number for now.
    return hasClue(currentLevelNumber) && areBothExplorersAtExit();
}

// ========================================================================
//  6. LEVEL LIFECYCLE
//     M2 TODO: loadFromFile(path) for the data-driven bonus.
//     M2 TODO: Replay -> loadProgress() into persistentClues, then startLevel(n).
// ========================================================================

void MainWindow::startLevel(int levelNumber) {
    currentLevelNumber = levelNumber;
    setWindowTitle(QString("Ruins of the Unown - Level %1").arg(levelNumber));
    hud->setLevelText(levelTitle(levelNumber));

    level->loadLevel(levelNumber);
    restartCurrentLevel();          // fresh HP / exits / current-level clues, HUD re-synced
    setState(GameState::Playing);
}

void MainWindow::restartCurrentLevel() {
    currentLevelClues.clear();   // L1-19: only this level's clues are lost

    level->reset();   // restore the explorers before reading their HP for the HUD
    hud->resetCurrentLevelHUD(level->getExplorerA()->getMaxHealth(), level->getExplorerB()->getMaxHealth());
    refreshClueHud();   // re-light earlier clues, keep this level's slot empty

    frameClock.restart();
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

void MainWindow::refreshClueHud() {
    for (int id = 1; id <= HUDWidget::SigilCount; ++id) {
        hud->setSigilCollected(id, hasClue(id));
    }
}
