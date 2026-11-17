#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    resize(1024,768);
    setWindowTitle("Ruins of the Unown - Level 1");

    setStyleSheet("QMainWindow { background-color: #1e2d24; }");

    hud = new HUDWidget(this);
    hud->setGeometry(0,0,width(), 52);
    hud->raise();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    hud->setGeometry(0,0,width(), 52);
    hud->raise();
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_1) {
        testHpA = qMax(0, testHpA - 20);
        hud->setPlayerHealth(1, testHpA, 100);
    } else if (event->key() == Qt::Key_2) {
        testHpB = qMax(0, testHpB - 20);
        hud->setPlayerHealth(2, testHpB, 100);
    } else if (event->key() == Qt::Key_3) {
        testSigil1 = !testSigil1;
        hud->setSigilCollected(1, testSigil1);
    } else if (event->key() == Qt::Key_R) {
        testHpA = 100;
        testHpB = 100;
        testSigil1 = false;
        hud->resetCurrentLevelHUD(100,100);
    }

    QMainWindow::keyPressEvent(event);
}