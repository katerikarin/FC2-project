#pragma once

#include <QMainWindow>
#include <QResizeEvent>
#include <QKeyEvent>
#include "HUDWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    Ui::MainWindow *ui;
    HUDWidget* hud;

    int testHpA = 100;
    int testHpB = 100;
    bool testSigil1 = false;
};
