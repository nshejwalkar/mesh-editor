#pragma once
#include <utils.h>
#include <meshcomponents.h>
#include <drawable.h>

class Mesh : public Drawable
{
    friend class MyGL;
private:
    std::vector<uPtr<Face>> faces;
    std::vector<uPtr<Vertex>> vertices;
    std::vector<uPtr<HalfEdge>> edges;

public:
    Mesh(OpenGLContext*);
    void buildMesh(const std::vector<glm::vec3>&,
                   const std::vector<std::vector<int>>&);
    void initializeAndBufferGeometryData() override;
    void loadOBJ(QString&);
    GLenum drawMode() override;

    void splitEdge(HalfEdge*);
    void triangulateFace(Face*);
    void catmullClark();

    void addSmoothedMidpoint(HalfEdge*, std::unordered_map<Face*, Vertex*>&, std::unordered_set<HalfEdge*>&);

    const std::vector<uPtr<Face>>& getFaces() const {
        return faces;
    };

    const std::vector<uPtr<Vertex>>& getVertices() const {
        return vertices;
    };

    const std::vector<uPtr<HalfEdge>>& getEdges() const {
        return edges;
    };
};

