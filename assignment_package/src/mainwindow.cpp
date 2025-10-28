#include "mainwindow.h"
#include <ui_mainwindow.h>
#include <QFileDialog>
#include <QDir>
#include "utils.h"
#include <debug.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mygl->setFocus();

    // we should only be able to select one vertex/face/edge at a time
    ui->vertsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->facesListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->halfEdgesListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

        // Widget that emits the signal
    connect(ui->mygl,
        // Signal name
        SIGNAL(sig_meshWasBuiltOrRebuilt(const Mesh*)),
        // Widget with the slot that receives the signal
        this,
        // Slot name
        SLOT(slot_rebuildLists(const Mesh*)));

    connect(ui->vertsListWidget,
            SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),  // better than itemClicked()
            this,
            SLOT(slot_onVertexPicked(QListWidgetItem*, QListWidgetItem*)));

    connect(ui->facesListWidget,
            SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),  // better than itemClicked()
            this,
            SLOT(slot_onFacePicked(QListWidgetItem*, QListWidgetItem*)));

    connect(ui->halfEdgesListWidget,
            SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),  // better than itemClicked()
            this,
            SLOT(slot_onEdgePicked(QListWidgetItem*, QListWidgetItem*)));
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
    LOG("open triggered");
}

void MainWindow::slot_rebuildLists(const Mesh* mesh) {
    LOG("slot_rebuildlists triggered");
    // here, traverse thru mesh->vertices, faces, edges and add to ui
    for (const auto& vuptr : mesh->getVertices()) {
        ui->vertsListWidget->addItem(vuptr.get());
    }

    for (const auto& fuptr : mesh->getFaces()) {
        ui->facesListWidget->addItem(fuptr.get());
    }

    for (const auto& euptr : mesh->getEdges()) {
        ui->halfEdgesListWidget->addItem(euptr.get());
    }
}

void MainWindow::slot_onVertexPicked(QListWidgetItem* vertItem, QListWidgetItem*) {
    LOG("new vertex picked");
    // get vertex from vertItem
    // set ui->mygl->selectvertex()
    Vertex* vert = dynamic_cast<Vertex*>(vertItem);  // need a dynamic cast to get the child class
    ui->mygl->selectVertex(vert);
}

void MainWindow::slot_onFacePicked(QListWidgetItem* faceItem, QListWidgetItem*) {
    LOG("new face picked");
    // get Face from faceItem
    // set ui->mygl->selectFace()
    Face* face = dynamic_cast<Face*>(faceItem);  // need a dynamic cast to get the child class
    ui->mygl->selectFace(face);
}

void MainWindow::slot_onEdgePicked(QListWidgetItem* edgeItem, QListWidgetItem*) {
    LOG("new edge picked");
    // get Edge from edgeItem
    // set ui->mygl->selectEdge()
    HalfEdge* edge = dynamic_cast<HalfEdge*>(edgeItem);  // need a dynamic cast to get the child class
    ui->mygl->selectHalfEdge(edge);
}

