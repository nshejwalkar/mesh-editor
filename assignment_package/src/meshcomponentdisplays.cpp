#include "meshcomponentdisplays.h"


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
    representedVertex = v;
}

void FaceDisplay::updateFace(Face* f) {
    representedFace = f;
}

void HalfEdgeDisplay::updateHalfEdge(HalfEdge* he) {
    representedHalfEdge = he;
}

void VertexDisplay::initializeAndBufferGeometryData() {
    ;
}

void FaceDisplay::initializeAndBufferGeometryData() {
    ;
}

void HalfEdgeDisplay::initializeAndBufferGeometryData() {
    ;
}
