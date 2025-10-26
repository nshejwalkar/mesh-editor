#include "meshcomponents.h"

// initialize static variables once
int Vertex::last_created = 0;
int Face::last_created = 0;
int HalfEdge::last_created = 0;

Vertex::Vertex(float x, float y, float z)
    : QListWidgetItem(),
    pos(x,y,z),
    edge(nullptr),
    id(last_created++)
{}

Vertex::Vertex(const glm::vec3& w)
    : QListWidgetItem(),
    pos(w),
    edge(nullptr),
    id(last_created++)
{}

Face::Face()
    : QListWidgetItem(),
    edge(nullptr),
    color(1.f, 1.f, 1.f),
    id(last_created++)
{}

HalfEdge::HalfEdge()
    : QListWidgetItem(),
    next(nullptr), sym(nullptr), face(nullptr), vertex(nullptr),
    id(last_created++)
{}
