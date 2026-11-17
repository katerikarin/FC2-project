#include "HUDWidget.h"

// ========================================================================
//  CONSTRUCTION
// ========================================================================

HUDWidget::HUDWidget(QWidget *parent) : QWidget{parent} {
    setAttribute(Qt::WA_TransparentForMouseEvents);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 10, 16, 10);
    layout->setSpacing(12);

    hpBarA = makeHpBar("Explorer A", "#27ae60");
    hpBarB = makeHpBar("Explorer B", "#2980b9");

    levelLabel = new QLabel("Level 1: Mossy Cave", this);
    levelLabel->setFixedHeight(28);
    levelLabel->setAlignment(Qt::AlignCenter);
    levelLabel->setStyleSheet(
        "color: #ecf0f1; font-weight: bold; font-size: 13px;"
        "background-color: rgba(15, 20, 18, 210);"
        "border: 1px solid #4e6e58; border-radius: 5px; padding: 0 12px;"
        );

    layout->addWidget(hpBarA);
    layout->addWidget(hpBarB);
    layout->addStretch();
    layout->addWidget(levelLabel);

    // Five compact slots ("1".."5") so all clues fit next to the level label.
    for (int i = 1; i <= SigilCount; ++i) {
        auto* slot = new QLabel(QString::number(i), this);
        slot->setFixedSize(34, 28);
        slot->setAlignment(Qt::AlignCenter);
        slot->setToolTip(QString("Sigil %1").arg(i));
        sigilSlots.append(slot);
        styleSigilSlot(slot, i, false);
        layout->addWidget(slot);
    }
}

// ========================================================================
//  HEALTH
// ========================================================================

QProgressBar* HUDWidget::makeHpBar(const QString& label, const QString& chunkColor) {
    auto* bar = new QProgressBar(this);
    bar->setRange(0, 100);
    bar->setValue(100);
    bar->setFixedSize(195, 28);
    bar->setFormat(label + ": %v/%m");
    bar->setStyleSheet(
        "QProgressBar {"
        "  border: 2px solid #2c3e50; border-radius: 5px;"
        "  background-color: rgba(15, 20, 18, 210);"
        "  color: #ffffff; font-weight: bold; font-size: 12px; text-align: center;"
        "}"
        "QProgressBar::chunk { background-color: " + chunkColor + "; border-radius: 3px; }"
        );
    return bar;
}

void HUDWidget::setPlayerHealth(int playerId, int current, int max) {
    QProgressBar* bar = (playerId == 1) ? hpBarA : hpBarB;
    bar->setRange(0, max);
    bar->setValue(qBound(0, current, max));
}

// ========================================================================
//  SIGIL (CLUE) SLOTS
// ========================================================================

void HUDWidget::styleSigilSlot(QLabel* slot, int index, bool collected) {
    Q_UNUSED(index);
    if (collected) {
        slot->setStyleSheet(
            "color: #f1c40f; font-weight: bold; font-size: 13px;"
            "background-color: rgba(45, 38, 10, 230);"
            "border: 2px solid #f1c40f; border-radius: 5px;"
            );
    } else {
        slot->setStyleSheet(
            "color: #95a5a6; font-weight: bold; font-size: 13px;"
            "background-color: rgba(15, 20, 18, 210);"
            "border: 1px solid #4e6e58; border-radius: 5px;"
            );
    }
}

void HUDWidget::setSigilCollected(int sigilIndex, bool collected) {
    if (sigilIndex < 1 || sigilIndex > sigilSlots.size()) return;
    styleSigilSlot(sigilSlots[sigilIndex - 1], sigilIndex, collected);
}

// ========================================================================
//  LEVEL TEXT & RESET
// ========================================================================

void HUDWidget::setLevelText(const QString& text) {
    levelLabel->setText(text);
}

void HUDWidget::resetCurrentLevelHUD(int maxHpA, int maxHpB) {
    setPlayerHealth(1, maxHpA, maxHpA);
    setPlayerHealth(2, maxHpB, maxHpB);
    for (int i = 1; i <= sigilSlots.size(); ++i) {
        setSigilCollected(i, false);
    }
}