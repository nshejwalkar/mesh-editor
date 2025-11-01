#include "mygl.h"
#include <la.h>

#include <QApplication>
#include <QKeyEvent>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <debug.h>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      timer(), currTime(0.),
      m_geomSquare(this),
      m_progLambert(this), m_progFlat(this),
      vao(),
      m_camera(width(), height()),
      m_mousePosPrev(),
      m_vertDisplay(this),
      m_faceDisplay(this),
      m_edgeDisplay(this)
{
    setFocusPolicy(Qt::StrongFocus);

    m_mesh = std::make_unique<Mesh>(this);  // create the mesh object

    connect(&timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    timer.start(16);
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
}

void MyGL::slot_splitEdge() {
    // perform the mesh operation
    LOG("performing splitEdge");
    if (!m_selectedHalfEdge) return;
    m_mesh->splitEdge(m_selectedHalfEdge);
    // update m_edgeDisplay just to rebuffer data (could also just call initandbuffer())
    m_edgeDisplay.updateHalfEdge(m_selectedHalfEdge);
    // call update() to update display
    update();
    // emit signal to mainwindow to update lists. we're unnecesarily reforming the whole list, but it doens't really matter
    emit sig_meshWasBuiltOrRebuilt(m_mesh.get());
}

void MyGL::slot_triangulateFace() {
    // perform the mesh operation
    LOG("performing triangulate");
    if (!m_selectedFace) return;
    m_mesh->triangulateFace(m_selectedFace);
    // possibly update m_[thing]display
    m_faceDisplay.updateFace(m_selectedFace);
    // call update() to update display
    update();
    // emit signal to mainwindow to update lists
    emit sig_meshWasBuiltOrRebuilt(m_mesh.get());
}

void MyGL::slot_catmullClark() {
    LOG("performing catmull clark");
    // perform the mesh operation
    m_mesh->catmullClark();
    LOG("performed catmull clark");
    // then rebuffer all three small vert/face/edge displays
    if (m_selectedFace) m_faceDisplay.updateFace(m_selectedFace);
    if (m_selectedHalfEdge) m_edgeDisplay.updateHalfEdge(m_selectedHalfEdge);
    if (m_selectedVertex) m_vertDisplay.updateVertex(m_selectedVertex);
    m_mesh->initializeAndBufferGeometryData();
    update();
    emit sig_meshWasBuiltOrRebuilt(m_mesh.get());
}

glm::vec3 MyGL::selectVertex(Vertex* v) {
    LOG("selected vertex");
    m_selectedVertex = v;
    m_vertDisplay.updateVertex(v);
    // we must trigger the whole mesh to be redrawn.
    // might be unintuitive at first, because we can draw it on top, so occlusions/depth calc doesnt even matter here
    // but its not possible to glClear only a single VBO's contributions after its drawn unless you somehow keep track of it in the framebuffer
    // and we must update() the whole mesh, including this vertex, anyway at a high frame rate, so trying to hack it is beyond not worth it
    update();
    return v->pos;
}

glm::vec3 MyGL::selectFace(Face* f) {
    LOG("selected face " << f->id);
    m_selectedFace = f;
    m_faceDisplay.updateFace(f);
    update();

    return f->color;
}

void MyGL::selectHalfEdge(HalfEdge* he) {
    LOG("selected edge");
    m_selectedHalfEdge = he;
    m_edgeDisplay.updateHalfEdge(he);
    update();
}

void MyGL::changeVertexPosition(float val, char direction) {
    LOG("changing " << direction);
    switch (direction) {
        case 'X':
            m_selectedVertex->pos.x = val;
            break;
        case 'Y':
            m_selectedVertex->pos.y = val;
            break;
        case 'Z':
            m_selectedVertex->pos.z = val;
            break;
    }
    // must update VBO. for now lets just change the whole thing
    m_mesh->initializeAndBufferGeometryData();
    update();
};

void MyGL::changeFaceColor(float val, char channel) {
    LOG("changing " << channel);
    switch (channel) {
        case 'R':
            m_selectedFace->color.r = val;
            break;
        case 'G':
            m_selectedFace->color.g = val;
            break;
        case 'B':
            m_selectedFace->color.b = val;
            break;
        }
    m_mesh->initializeAndBufferGeometryData();
    update();
}


void MyGL::loadOBJ(const QString& path) {
    std::ifstream objfile(path.toStdString());
    if (!objfile.is_open()) {std::cout << "Unable to open file"; return;}

    std::vector<glm::vec3> positions;
    std::vector<std::vector<int>> faceIndices;  // can store faces with arb many sides

    /*
    Here is the main logic of the file parsing. For each line, we check the first token of the line.
    Depending on whether the line starts with "v" or "f", we either push the positions into the positions vector above,
    or collect the relevant indices of the vertices into the faceIndices vector.
    We can then pass in this information to a Mesh class function to build the half-edge mesh graph.
    */
    std::string line;
    while (std::getline(objfile, line)) {
        std::istringstream iss(line);
        std::string first;
        iss >> first;  // streams until whitespace, so will get the first word (v, f, vn, etc)

        if (first == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            positions.push_back(glm::vec3(x,y,z));
        }
        else if (first == "f") {
            std::vector<int> verts;
            std::string vertexStr;
            while (iss >> vertexStr) {  // get just one string of pos/uv/normal
                std::replace(vertexStr.begin(), vertexStr.end(), '/', ' ');
                std::istringstream vs(vertexStr);
                int posIndex;
                vs >> posIndex;  // vs just streams the first number into posindex
                verts.push_back(posIndex - 1);
            }
            faceIndices.push_back(verts);
        }
        else continue;
    }

    // Now, we build the m_mesh object
    m_mesh->buildMesh(positions, faceIndices);

    // Trigger the rebuild slot for the non-MyGl ui widget
    LOG("emitting signal");
    emit sig_meshWasBuiltOrRebuilt(m_mesh.get());

    // Buffer the data
    m_mesh->initializeAndBufferGeometryData();

    // Update and repaint the screen
    update();

}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.5, 0.5, 0.5, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instances of Cylinder and Sphere.
    m_geomSquare.initializeAndBufferGeometryData();

    // Create and set up the diffuse shader
    m_progLambert.createAndCompileShaderProgram("lambert.vert.glsl", "lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.createAndCompileShaderProgram("flat.vert.glsl", "flat.frag.glsl");


    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_camera.recomputeAspectRatio(w, h);

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)
    glm::mat4 viewproj = m_camera.getViewProj();
    m_progLambert.setUnifMat4("u_ViewProj", viewproj);
    m_progFlat.setUnifMat4("u_ViewProj", viewproj);

    printGLErrorLog();
}

//This function is called by Qt any time your GL window is supposed to update
//For example, when the function update() is called, paintGL is called implicitly.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 viewproj = m_camera.getViewProj();
    // glm::mat4 model = glm::mat4(1.f);
    m_progLambert.setUnifMat4("u_ViewProj", viewproj);
    m_progFlat.setUnifMat4("u_ViewProj", viewproj);
    m_progLambert.setUnifVec3("u_CamPos", m_camera.eye);
    m_progFlat.setUnifMat4("u_Model", glm::mat4(1.f));
    m_progLambert.setUnifMat4("u_Model", glm::mat4(1.f));

    if (m_mesh && m_mesh->getIndexBufferLength() > 0) {  // only display if set
        m_progLambert.draw(*m_mesh);  // binds to existing buffers

        glDisable(GL_DEPTH_TEST);
        if (m_vertDisplay.getIndexBufferLength() > 0) m_progFlat.draw(m_vertDisplay);
        if (m_faceDisplay.getIndexBufferLength() > 0) m_progFlat.draw(m_faceDisplay);
        if (m_edgeDisplay.getIndexBufferLength() > 0) m_progFlat.draw(m_edgeDisplay);
        glEnable(GL_DEPTH_TEST);

        return;
    }

    //Create a model matrix. This one rotates the square by PI/4 radians then translates it by <-2,0,0>.
    //Note that we have to transpose the model matrix before passing it to the shader
    //This is because OpenGL expects column-major matrices, but you've
    //implemented row-major matrices.
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-2,0,0)) * glm::rotate(glm::mat4(), 0.25f * 3.14159f, glm::vec3(0,1,0));
    //Send the geometry's transformation matrix to the shader
    m_progLambert.setUnifMat4("u_Model", model);
    m_progLambert.setUnifMat4("u_ModelInvTr", glm::inverse(glm::transpose(model)));
    //Draw the example sphere using our lambert shader
    m_progLambert.draw(m_geomSquare);

    //Now do the same to render the cylinder
    //We've rotated it -45 degrees on the Z axis, then translated it to the point <2,2,0>
    model = glm::translate(glm::mat4(1.0f), glm::vec3(2,2,0)) * glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0,0,1));
    m_progLambert.setUnifMat4("u_Model", model);
    m_progLambert.setUnifMat4("u_ModelInvTr", glm::inverse(glm::transpose(model)));
    m_progLambert.draw(m_geomSquare);
}

void MyGL::keyPressEvent(QKeyEvent *e) {
    switch (e->key()) {
        case Qt::Key_N:
            LOG("N");
            if (m_edgeDisplay.getIndexBufferLength() > 0) selectHalfEdge(m_selectedHalfEdge->next);
            break;
        case Qt::Key_M:
            LOG("M");
            if (m_edgeDisplay.getIndexBufferLength() > 0) selectHalfEdge(m_selectedHalfEdge->sym);
            break;
        case Qt::Key_F:
            LOG("F");
            if (m_edgeDisplay.getIndexBufferLength() > 0) selectFace(m_selectedHalfEdge->face);
            break;
        case Qt::Key_V:
            LOG("V");
            if (m_edgeDisplay.getIndexBufferLength() > 0) selectVertex(m_selectedHalfEdge->vertex);
            break;
        case Qt::Key_H:
            if (e->modifiers() & Qt::ShiftModifier) {
                LOG("Shift H");
                if (m_faceDisplay.getIndexBufferLength() > 0) selectHalfEdge(m_selectedFace->edge);
                break;
            } else {
                LOG("H");
                if (m_vertDisplay.getIndexBufferLength() > 0) selectHalfEdge(m_selectedVertex->edge);
                break;
            }
    }
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    if(e->buttons() & (Qt::LeftButton | Qt::RightButton))
    {
        m_mousePosPrev = glm::vec2(e->pos().x(), e->pos().y());
    }
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    glm::vec2 pos(e->pos().x(), e->pos().y());
    if(e->buttons() & Qt::LeftButton)
    {
        // Rotation
        glm::vec2 diff = 0.2f * (pos - m_mousePosPrev);
        m_mousePosPrev = pos;
        m_camera.RotateAboutGlobalUp(-diff.x);
        m_camera.RotateAboutLocalRight(-diff.y);
    }
    else if(e->buttons() & Qt::RightButton)
    {
        // Panning
        glm::vec2 diff = 0.05f * (pos - m_mousePosPrev);
        m_mousePosPrev = pos;
        m_camera.PanAlongRight(-diff.x);
        m_camera.PanAlongUp(diff.y);
    }
}

void MyGL::wheelEvent(QWheelEvent *e) {
    m_camera.Zoom(e->angleDelta().y() * 0.001f);
}

void MyGL::tick() {
    ++currTime;
    update();
}
