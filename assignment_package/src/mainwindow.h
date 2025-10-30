#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <mesh.h>


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionQuit_triggered();
    void on_actionOpenOBJ_triggered();

    // this connects to a signal in mygl after every build or rebuild
    void slot_rebuildLists(const Mesh* mesh);
    void slot_onVertexPicked(QListWidgetItem* vertItem);
    void slot_onFacePicked(QListWidgetItem* faceItem);
    void slot_onEdgePicked(QListWidgetItem* edgeItem);

private:
    Ui::MainWindow *ui;
    void resetAllWidgetValues();
};


#endif // MAINWINDOW_H
