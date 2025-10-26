#pragma once
#include <glm/glm.hpp>
#include <QListWidgetItem>

class HalfEdge;

class Vertex : public QListWidgetItem
{
    friend class HalfEdge;
    friend class Mesh;
private:
    glm::vec3 pos;
    HalfEdge* edge;
    const int id;
    static int last_created;

public:
    Vertex(float, float, float);
    Vertex(const glm::vec3&);
};

class Face : public QListWidgetItem
{
    friend class HalfEdge;
    friend class Mesh;
private:
    HalfEdge* edge;
    glm::vec3 color;
    const int id;
    static int last_created;

public:
    Face();
};

class HalfEdge : public QListWidgetItem
{
    friend class Mesh;
private:
    HalfEdge* next;
    HalfEdge* sym;
    Face* face;
    Vertex* vertex;
    const int id;
    static int last_created;

public:
    HalfEdge();

    void setVertex(Vertex* v) {
        this->vertex = v;
        v->edge = this;
    }

    void setFace(Face* f) {
        this->face = f;
        f->edge = this;
    }
};
