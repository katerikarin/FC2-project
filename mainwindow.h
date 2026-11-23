#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QElapsedTimer>   // fixed: was <QElapsedTimer.h> (breaks on case-sensitive systems, L-12 "fresh clone")
#include <QLabel>
#include <QSet>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include "HUDWidget.h"
#include "Level.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// ============================= GAME STATE ===============================
// M2 TODO: add MainMenu, LevelSelect, Settings, Credits, ReplayConfirm, GameWon
//          (Title can be renamed MainMenu once the menu exists).
enum class GameState {
    Title,
    Playing,
    LevelComplete,
    NextLevelStub      // M2 TODO: delete once Level 2 exists
};

// ============================= GAME CONSTANTS ===========================
// Single source of truth for "magic numbers". Fill from the team design freeze (A5).
namespace Config {
constexpr int WindowW          = 1024;
constexpr int WindowH          = 768;
constexpr int HudHeight        = 52;
constexpr int TickMs           = 16;     // ~60 FPS
constexpr int CompleteBannerMs = 2500;   // L1-18
constexpr int MaxHp            = 100;    // TODO(A5): confirm with team HP table
constexpr int TotalLevels      = 5;      // M2
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    // ===================== PUBLIC API (used by teammates) ===============
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    GameState currentState() const { return state; }
    bool hasClue(int clueId) const;

public slots:
    // Ali's Puzzle/Clue system should emit a signal connected to this.
    void onClueCollected(int clueId);

protected:
    // ===================== QT EVENT OVERRIDES ===========================
    bool event(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;     // Ahmed (B1): replace debug keys with InputHandler
    void keyReleaseEvent(QKeyEvent* event) override;   // Ahmed (B1): needed for no-stuck-keys (checklist #2/#3)
    void mousePressEvent(QMouseEvent* event) override; // M2: stays for click-to-start only

private slots:
    void onGameTick();
    void onTransitionTimerTimeout();

private:
    // ===================== 1. UI SETUP (called once from ctor) ==========
    void setupScene();
    void setupHud();
    void setupOverlayBanner();
    void setupTimers();
    void updateOverlayLayout();
    void showBanner(const QString& text);
    // M2 TODO: setupMenus() -> own QWidget container WITHOUT WA_TransparentForMouseEvents

    // ===================== 2. STATE MACHINE =============================
    void setState(GameState newState);
    void enterTitle();
    void enterPlaying();
    void enterLevelComplete();
    void enterNextLevelStub();

    // ===================== 3. LEVEL LIFECYCLE ===========================
    // Malek (Level.cpp) plugs in here.
    void startLevel(int levelNumber);   // the ONLY way a level begins (M1: always 1)
    void restartCurrentLevel();
    // M2 TODO: loadFromFile(path) for the data-driven bonus; replay -> startLevel(n)
    //          with persistentClues pre-filled from save.txt.

    // ===================== 4. GAME LOOP (per-tick rules) ================
    // Order matters: death BEFORE exit (L1-10), exit needs clue AND both (L1-17).
    bool isEitherExplorerDead() const;
    bool areBothExplorersAtExit() const;
    bool isLevelWon() const;            // clue AND both at exit (L1-17)

    // ===================== 5. CLUE STORAGE (L1-19) ======================
    void refreshClueHud();              // repaint all 5 sigil slots from the two clue sets
    // M2 TODO: saveProgress() / loadProgress() with QFile + QTextStream (save.txt)

    // ===================== MEMBERS ======================================
    // ---- Qt scaffolding
    Ui::MainWindow* ui;
    QGraphicsScene* scene;
    QGraphicsView*  view;
    HUDWidget*      hud;
    QLabel*         overlayBanner;   // mouse-transparent; M2 menus need a separate container

    // ---- Timing
    QTimer*       gameLoopTimer;
    QTimer*       transitionTimer;
    QElapsedTimer frameClock;

    // ---- Game progress
    GameState state = GameState::Title;
    int currentLevelNumber = 1;
    QSet<int> persistentClues;     // survive a restart
    QSet<int> currentLevelClues;   // wiped on restart

    // ---- the current level (explorers, walls, exit)
    Level* level = nullptr;
};

#endif // MAINWINDOW_H
