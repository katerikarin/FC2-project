#pragma once

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QHBoxLayout>
#include <QList>
#include <QString>

// Mouse-transparent overlay drawn on top of the game view.
// It only DISPLAYS state; MainWindow/Level own the truth.
class HUDWidget : public QWidget {
    Q_OBJECT

public:
    static constexpr int SigilCount = 5;   // one clue per level (spec: all clues stay viewable)

    explicit HUDWidget(QWidget *parent = nullptr);

public slots:
    // playerId: 1 = Explorer A, 2 = Explorer B
    void setPlayerHealth(int playerId, int current, int max);
    // sigilIndex: 1..SigilCount
    void setSigilCollected(int sigilIndex, bool collected);
    void setLevelText(const QString& text);
    // Resets health bars AND empties every sigil slot; caller re-applies persistent clues.
    void resetCurrentLevelHUD(int maxHpA = 100, int maxHpB = 100);

private:
    QProgressBar* makeHpBar(const QString& label, const QString& chunkColor);
    void styleSigilSlot(QLabel* slot, int index, bool collected);

    QProgressBar*   hpBarA;
    QProgressBar*   hpBarB;
    QLabel*         levelLabel;
    QList<QLabel*>  sigilSlots;   // index 0 = Sigil 1
};