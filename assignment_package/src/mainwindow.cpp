#include "mainwindow.h"
#include <ui_mainwindow.h>
#include <QFileDialog>
#include <QDir>
#include "utils.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mygl->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionOpenOBJ_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open .obj File", getCurrentPath());
    ui->mygl->loadOBJ(filename);  // pass it off to mygl
}
