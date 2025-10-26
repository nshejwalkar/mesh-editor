#include "mesh.h"
#include "meshcomponents.h"
#include <stdlib.h>
#include <iostream>

Mesh::Mesh(OpenGLContext* context)
    : Drawable(context)
{}

// passed in from MyGL::loadOBJ
void Mesh::buildMesh(const std::vector<glm::vec3>& positions, const std::vector<std::vector<int>>& faceIndices) {
    std::cout << "building mesh" << std::endl;
    // reset the mesh
    this->vertices.clear();
    this->faces.clear();
    this->edges.clear();

    std::cout << "verts" << std::endl;
    // first fill out the vertices
    for (const glm::vec3& pos : positions) {
        auto v = std::make_unique<Vertex>(pos);
        this->vertices.push_back(std::move(v));
    }

    std::map<std::pair<int,int>, HalfEdge*> vertsToEdge;  // stores <source, dest> vertex : halfedge to make setting syms easy
    std::map<HalfEdge*, std::pair<int,int>> edgeToVerts;  // opposite

    std::cout << "faces" << std::endl;
    int i=0;
    // next, go through the faceIndices and fill out faces and edges
    for (const auto& indices : faceIndices) {  // each indices is a vector of size n, the number of edges on that face
        std::cout << i << std::endl;
        const int n = indices.size();

        auto f = std::make_unique<Face>();
        f->color = glm::vec3(static_cast<float>(std::rand()) / RAND_MAX,
                             static_cast<float>(std::rand()) / RAND_MAX,
                             static_cast<float>(std::rand()) / RAND_MAX);

        // first add edges to local vector
        std::vector<HalfEdge*> faceEdges;  // just stores the edges in this face only first
        for (int i = 0; i < n; i++) {
            // create a halfedge for every index, point it to a vertex
            auto he = std::make_unique<HalfEdge>();
            faceEdges.push_back(he.get());
            this->edges.push_back(std::move(he));
        }

        // then fill in information using local indices
        for (int i = 0; i < n; i++) {
            int source_vert_idx = indices[i];
            int dest_vert_idx = indices[(i+1)%n];  // using this modulo we can find the next vertex

            HalfEdge* he = faceEdges[i];
            he->setFace(f.get());
            he->setVertex(this->vertices[dest_vert_idx].get());
            he->next = faceEdges[(i+1)%n];

            vertsToEdge[{source_vert_idx, dest_vert_idx}] = he;  // set vertices
            edgeToVerts[he] = {source_vert_idx, dest_vert_idx};
        }
        i++;
        this->faces.push_back(std::move(f));
    }

    std::cout << "syms" << std::endl;
    // now point the syms
    for (auto& heUptr : this->edges) {
        HalfEdge* he = heUptr.get();
        auto [a,b] = edgeToVerts[he];
        auto symEdge = vertsToEdge[{b,a}];
        he->sym = symEdge;
    }
}

void Mesh::initializeAndBufferGeometryData() {
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> col;
    std::vector<glm::vec3> nor;
    std::vector<GLuint> idx;

    int anchor = 0;
    for(auto& f : this->faces) {
        anchor = pos.size();
        // first, traverse around HEs and push verts in vbo
        HalfEdge* cur = f->edge;
        int numVerts = 0;
        do {
            pos.push_back(cur->vertex->pos);
            col.push_back(f->color);
            numVerts++;
            cur = cur->next;
        } while (cur != f->edge);

        // then, triangulate and push indices in ibo
        for(int i = 0; i < numVerts-2; i++) {
            idx.push_back(anchor);
            idx.push_back(anchor+i+1);
            idx.push_back(anchor+i+2);
        }
    }

    // use the functions in drawable
    std::cout<< "pos" << std::endl;
    generateBuffer(BufferType::POSITION);
    bindBuffer(BufferType::POSITION);
    bufferData(BufferType::POSITION, pos);

    std::cout<< "col" << std::endl;

    generateBuffer(BufferType::COLOR);
    bindBuffer(BufferType::COLOR);
    bufferData(BufferType::COLOR, col);

    std::cout<< "idx" << std::endl;

    generateBuffer(BufferType::INDEX);
    bindBuffer(BufferType::INDEX);
    bufferData(BufferType::INDEX, idx);

    this->indexBufferLength = idx.size();
}

GLenum Mesh::drawMode() {
    return GL_TRIANGLES;
}
