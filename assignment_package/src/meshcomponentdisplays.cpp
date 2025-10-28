#include "meshcomponentdisplays.h"
#include "debug.h"

VertexDisplay::VertexDisplay(OpenGLContext* context)
    : Drawable(context), representedVertex(nullptr)
{}

FaceDisplay::FaceDisplay(OpenGLContext* context)
    : Drawable(context), representedFace(nullptr)
{}

HalfEdgeDisplay::HalfEdgeDisplay(OpenGLContext* context)
    : Drawable(context), representedHalfEdge(nullptr)
{}


void VertexDisplay::updateVertex(Vertex* v) {
    LOG("updated vertex");
    representedVertex = v;
    initializeAndBufferGeometryData();
}

void FaceDisplay::updateFace(Face* f) {
    representedFace = f;
    initializeAndBufferGeometryData();
}

void HalfEdgeDisplay::updateHalfEdge(HalfEdge* he) {
    representedHalfEdge = he;
    initializeAndBufferGeometryData();
}

void VertexDisplay::initializeAndBufferGeometryData() {
    destroyGPUData();

    LOG("initializing buffer for selected vertex");

    // create a new, small vbo just for one vertex
    std::vector<glm::vec3> pos = {representedVertex->pos};
    std::vector<glm::vec3> col = {{1,1,1}};  // white
    std::vector<GLuint> idx = {0};

    // use the functions in drawable
    generateBuffer(BufferType::POSITION);
    bindBuffer(BufferType::POSITION);
    bufferData(BufferType::POSITION, pos);

    generateBuffer(BufferType::COLOR);
    bindBuffer(BufferType::COLOR);
    bufferData(BufferType::COLOR, col);

    generateBuffer(BufferType::INDEX);
    bindBuffer(BufferType::INDEX);
    bufferData(BufferType::INDEX, idx);

    this->indexBufferLength = 1;
}

void FaceDisplay::initializeAndBufferGeometryData() {
    destroyGPUData();

    LOG("initializing buffer for selected face");
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> col;
    std::vector<GLuint> idx;  // 0,1,1,2,2,3...

    glm::vec3 line_color = 1.f - (representedFace->color);

    HalfEdge* cur = representedFace->edge;
    int i = 0;
    do {
        pos.push_back(cur->vertex->pos);
        col.push_back(line_color);
        idx.push_back(i); idx.push_back(i+1);
        cur = cur->next;
        i++;
    } while (cur != representedFace->edge);

    idx.pop_back();
    idx.push_back(0);  // want to end in a loop: 011220 for example

    // use the functions in drawable
    generateBuffer(BufferType::POSITION);
    bindBuffer(BufferType::POSITION);
    bufferData(BufferType::POSITION, pos);

    generateBuffer(BufferType::COLOR);
    bindBuffer(BufferType::COLOR);
    bufferData(BufferType::COLOR, col);

    generateBuffer(BufferType::INDEX);
    bindBuffer(BufferType::INDEX);
    bufferData(BufferType::INDEX, idx);

    this->indexBufferLength = idx.size();
}

void HalfEdgeDisplay::initializeAndBufferGeometryData() {
    destroyGPUData();

    LOG("initializing buffer for selected edge");

    // create a new, small vbo just for one edge
    std::vector<glm::vec3> pos = {representedHalfEdge->sym->vertex->pos, representedHalfEdge->vertex->pos};
    std::vector<glm::vec3> col = {{1,0,0},{1,1,0}};  // red->yellow
    std::vector<GLuint> idx = {0,1};

    // use the functions in drawable
    generateBuffer(BufferType::POSITION);
    bindBuffer(BufferType::POSITION);
    bufferData(BufferType::POSITION, pos);

    generateBuffer(BufferType::COLOR);
    bindBuffer(BufferType::COLOR);
    bufferData(BufferType::COLOR, col);

    generateBuffer(BufferType::INDEX);
    bindBuffer(BufferType::INDEX);
    bufferData(BufferType::INDEX, idx);

    this->indexBufferLength = 2;
}
