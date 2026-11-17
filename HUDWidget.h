#pragma once

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QHBoxLayout>
#include <QString>

class HUDWidget : public QWidget {
    Q_OBJECT

public:
    explicit HUDWidget(QWidget *parent = nullptr);

public slots:
    void setPlayerHealth(int playerID, int current, int max);
    void setSigilCollected(int sigilIndex, bool collected);
    void setLevelText(const QString& Text);
    void resetCurrentLevelHUD(int maxhpA = 100, int maxhpB = 100);

private:
    QProgressBar* hpBarA;
    QProgressBar* hpBarB;
    QLabel* levelLabel;
    QLabel* sigilSlot1;

signals:
};
