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

    ui->faceRedSpinBox->blockSignals(true);
    ui->faceGreenSpinBox->blockSignals(true);
    ui->faceBlueSpinBox->blockSignals(true);
    ui->vertPosXSpinBox->blockSignals(true);
    ui->vertPosYSpinBox->blockSignals(true);
    ui->vertPosZSpinBox->blockSignals(true);

        // Widget that emits the signal
    connect(ui->mygl,
        // Signal name
        SIGNAL(sig_meshWasBuiltOrRebuilt(const Mesh*)),
        // Widget with the slot that receives the signal
        this,
        // Slot name
        SLOT(slot_rebuildLists(const Mesh*)));

    // connect to a slot here that casts the widget then passes it to mygl.
    connect(ui->vertsListWidget,
            &QListWidget::currentItemChanged,  // better than itemClicked()
            this,
            [this](QListWidgetItem* newVertex){if (!newVertex) return; slot_onVertexPicked(newVertex);});

    connect(ui->facesListWidget,
            &QListWidget::currentItemChanged,
            this,
            [this](QListWidgetItem* newFace){if (!newFace) return; slot_onFacePicked(newFace);});

    connect(ui->halfEdgesListWidget,
            &QListWidget::currentItemChanged,
            this,
            [this](QListWidgetItem* newEdge){if (!newEdge) return; slot_onEdgePicked(newEdge);});


    // connect directly to slots in mygl to perform graph operations. we dont need to pass anything from here, so the slot lives there.
    connect(ui->splitEdgeButton,
            SIGNAL(clicked()),
            ui->mygl,
            SLOT(slot_splitEdge()));

    connect(ui->triangulateButton,
            SIGNAL(clicked()),
            ui->mygl,
            SLOT(slot_triangulateFace()));

    connect(ui->catmullClarkButton,
            SIGNAL(clicked()),
            ui->mygl,
            SLOT(slot_catmullClark()));

    // change position of vertex using new Qt syntax
    connect(ui->vertPosXSpinBox,
            &QDoubleSpinBox::valueChanged,
            ui->mygl,
            [this](float val){ui->mygl->changeVertexPosition(val, 'X');});

    connect(ui->vertPosYSpinBox,
            &QDoubleSpinBox::valueChanged,
            ui->mygl,
            [this](float val){ui->mygl->changeVertexPosition(val, 'Y');});

    connect(ui->vertPosZSpinBox,
            &QDoubleSpinBox::valueChanged,
            ui->mygl,
            [this](float val){ui->mygl->changeVertexPosition(val, 'Z');});

    // change color of face using new Qt syntax
    connect(ui->faceRedSpinBox,
            &QDoubleSpinBox::valueChanged,
            ui->mygl,
            [this](float val){ui->mygl->changeFaceColor(val, 'R');});

    connect(ui->faceGreenSpinBox,
            &QDoubleSpinBox::valueChanged,
            ui->mygl,
            [this](float val){ui->mygl->changeFaceColor(val, 'G');});

    connect(ui->faceBlueSpinBox,
            &QDoubleSpinBox::valueChanged,
            ui->mygl,
            [this](float val){ui->mygl->changeFaceColor(val, 'B');});
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resetAllWidgetValues() {
    ui->vertsListWidget->clearSelection();
    ui->halfEdgesListWidget->clearSelection();
    ui->facesListWidget->clearSelection();

    ui->faceRedSpinBox->setValue(0.0);
    ui->faceGreenSpinBox->setValue(0.0);
    ui->faceBlueSpinBox->setValue(0.0);
    ui->vertPosXSpinBox->setValue(0.0);
    ui->vertPosYSpinBox->setValue(0.0);
    ui->vertPosZSpinBox->setValue(0.0);
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionOpenOBJ_triggered()
{
    LOG("opening new file triggered");
    resetAllWidgetValues();
    QString filename = QFileDialog::getOpenFileName(this, "Open .obj File", getCurrentPath());
    ui->mygl->loadOBJ(filename);  // pass it off to mygl
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

// better UX: when selecting a new vertex, update the spinners with position
void MainWindow::slot_onVertexPicked(QListWidgetItem* vertItem) {
    LOG("new vertex picked");
    // get vertex from vertItem
    // set ui->mygl->selectvertex()
    Vertex* vert = dynamic_cast<Vertex*>(vertItem);  // need a dynamic cast to get the child class

    glm::vec3 pos = ui->mygl->selectVertex(vert);  // selects and returns position
    ui->vertPosXSpinBox->setValue(pos.x);
    ui->vertPosYSpinBox->setValue(pos.y);
    ui->vertPosZSpinBox->setValue(pos.z);
}

// better UX
void MainWindow::slot_onFacePicked(QListWidgetItem* faceItem) {
    LOG("new face picked");
    // get Face from faceItem
    // set ui->mygl->selectFace()
    Face* face = dynamic_cast<Face*>(faceItem);  // need a dynamic cast to get the child class

    glm::vec3 col = ui->mygl->selectFace(face);
    ui->faceRedSpinBox->setValue(col.r);
    ui->faceGreenSpinBox->setValue(col.g);
    ui->faceBlueSpinBox->setValue(col.b);
}

void MainWindow::slot_onEdgePicked(QListWidgetItem* edgeItem) {
    LOG("new edge picked");
    // get Edge from edgeItem
    // set ui->mygl->selectEdge()
    HalfEdge* edge = dynamic_cast<HalfEdge*>(edgeItem);  // need a dynamic cast to get the child class
    ui->mygl->selectHalfEdge(edge);
}

