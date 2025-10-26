#include "mygl.h"
#include <la.h>

#include <QApplication>
#include <QKeyEvent>
#include <iostream>
#include <fstream>
#include <sstream>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      timer(), currTime(0.),
      m_geomSquare(this),
      m_progLambert(this), m_progFlat(this),
      vao(),
      m_camera(width(), height()),
      m_mousePosPrev()
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

void MyGL::loadOBJ(const QString& path) {
    std::cout << "loading " << std::endl;
    std::ifstream objfile(path.toStdString());
    if (!objfile.is_open()) {std::cout << "Unable to open file"; return;}

    std::vector<glm::vec3> positions;
    std::vector<std::vector<int>> faceIndices;  // can store faces with arb many sides

    std::string line;
    while (std::getline(objfile, line)) {
        std::istringstream iss(line);
        std::string first;
        iss >> first;  // streams until whitespace, so will get the first word (v, f, vn, etc)

        if (first == "v") {
            std::cout << "vertex line " << std::endl;
            float x, y, z;
            iss >> x >> y >> z;
            positions.push_back(glm::vec3(x,y,z));
        }
        else if (first == "f") {
            std::cout << "face line " << std::endl;
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
    std::cout << positions.size() << " vertices, " << faceIndices.size() << " faces" << std::endl;

    // now build the m_mesh object
    m_mesh->buildMesh(positions, faceIndices);

    // buffer the data
    m_mesh->initializeAndBufferGeometryData();
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

    if (m_mesh && m_mesh->getIndexBufferLength() > 0) {  // only display if set
        m_progFlat.draw(*m_mesh);
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
        case Qt::Key_N: ;
        case Qt::Key_M: ;
        case Qt::Key_F: ;
        case Qt::Key_V: ;
        case Qt::Key_H: ;
    }
    update();
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
