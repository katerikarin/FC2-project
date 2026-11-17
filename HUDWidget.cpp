#include "HUDWidget.h"

HUDWidget::HUDWidget(QWidget *parent) : QWidget{parent} {
    setAttribute(Qt::WA_TransparentForMouseEvents);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16,10,16,10);
    layout->setSpacing(12);

    hpBarA = new QProgressBar(this);
    hpBarA->setRange(0,100);
    hpBarA->setValue(100);
    hpBarA->setFixedSize(195,28);
    hpBarA->setFormat("Explorer A: %v/%m");
    hpBarA->setStyleSheet(
        "QProgressBar {"
        "  border: 2px solid #2c3e50; border-radius: 5px;"
        "  background-color: rgba(15, 20, 18, 210);"
        "  color: #ffffff; font-weight: bold; font-size: 12px; text-align: center;"
        "}"
        "QProgressBar::chunk { background-color: #27ae60; border-radius: 3px; }"
    );

    hpBarB = new QProgressBar(this);
    hpBarB->setRange(0, 100);
    hpBarB->setValue(100);
    hpBarB->setFixedSize(195, 28);
    hpBarB->setFormat("Explorer B: %v/%m");
    hpBarB->setStyleSheet(
        "QProgressBar {"
        "  border: 2px solid #2c3e50; border-radius: 5px;"
        "  background-color: rgba(15, 20, 18, 210);"
        "  color: #ffffff; font-weight: bold; font-size: 12px; text-align: center;"
        "}"
        "QProgressBar::chunk { background-color: #2980b9; border-radius: 3px; }"
        );

    levelLabel = new QLabel("Level 1: Mossy Cave", this);
    levelLabel->setFixedHeight(28);
    levelLabel->setAlignment(Qt::AlignCenter);
    levelLabel->setStyleSheet(
        "color: #ecf0f1; font-weight: bold; font-size: 13px;"
        "background-color: rgba(15, 20, 18, 210);"
        "border: 1px solid #4e6e58; border-radius: 5px; padding: 0 12px;"
        );

    sigilSlot1 = new QLabel(this);
    sigilSlot1->setFixedHeight(28);
    sigilSlot1->setAlignment(Qt::AlignCenter);
    setSigilCollected(1, false); // Initialize as Empty

    layout->addWidget(hpBarA);
    layout->addWidget(hpBarB);
    layout->addStretch();
    layout->addWidget(levelLabel);
    layout->addWidget(sigilSlot1);

}


void HUDWidget::setPlayerHealth(int playerId, int current, int max) {
    QProgressBar* bar = (playerId == 1) ? hpBarA : hpBarB;
    bar->setRange(0, max);
    bar->setValue(qBound(0, current, max));
}

void HUDWidget::setSigilCollected(int sigilIndex, bool collected) {
    if (sigilIndex != 1) return;

    if (collected) {
        sigilSlot1->setText("Sigil 1: Collected");
        sigilSlot1->setStyleSheet(
            "color: #f1c40f; font-weight: bold; font-size: 13px;"
            "background-color: rgba(45, 38, 10, 230);"
            "border: 2px solid #f1c40f; border-radius: 5px; padding: 0 12px;"
        );
    } else {
        sigilSlot1->setText("Sigil 1: Empty");
        sigilSlot1->setStyleSheet(
            "color: #95a5a6; font-weight: bold; font-size: 13px;"
            "background-color: rgba(15, 20, 18, 210);"
            "border: 1px solid #4e6e58; border-radius: 5px; padding: 0 12px;"
        );
    }
}

void HUDWidget::setLevelText(const QString& text) {
    levelLabel->setText(text);
}

void HUDWidget::resetCurrentLevelHUD(int maxHpA, int maxHpB) {
    setPlayerHealth(1, maxHpA, maxHpA);
    setPlayerHealth(2, maxHpB, maxHpB);
    setSigilCollected(1, false);
}
























