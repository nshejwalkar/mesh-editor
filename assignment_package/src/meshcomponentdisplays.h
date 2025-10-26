#pragma once
#include <meshcomponents.h>
#include "drawable.h"

class VertexDisplay : public Drawable {
protected:
    Vertex *representedVertex;

public:
    VertexDisplay(OpenGLContext*);
    // Creates VBO data to make a visual
    // representation of the currently selected Vertex
    void initializeAndBufferGeometryData() override;
    // Change which Vertex representedVertex points to
    void updateVertex(Vertex*);
};

class FaceDisplay : public Drawable {
protected:
    Face *representedFace;

public:
    FaceDisplay(OpenGLContext*);
    // Creates VBO data to make a visual
    // representation of the currently selected Face
    void initializeAndBufferGeometryData() override;
    // Change which Face representedFace points to
    void updateFace(Face*);
};

class HalfEdgeDisplay : public Drawable {
protected:
    HalfEdge *representedHalfEdge;

public:
    HalfEdgeDisplay(OpenGLContext*);
    // Creates VBO data to make a visual
    // representation of the currently selected HalfEdge
    void initializeAndBufferGeometryData() override;
    // Change which HalfEdge representedHalfEdge points to
    void updateHalfEdge(HalfEdge*);
};
