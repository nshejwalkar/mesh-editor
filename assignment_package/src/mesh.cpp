#include "mesh.h"
#include "meshcomponents.h"
#include <stdlib.h>
#include "debug.h"
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>


Mesh::Mesh(OpenGLContext* context)
    : Drawable(context)
{}

void Mesh::splitEdge(HalfEdge* he1) {
    // dont delete anything. just add
    Vertex* v1 = he1->vertex;
    HalfEdge* he2 = he1->sym;
    Vertex* v2 = he2->vertex;

    uPtr<Vertex> v3 = mkU<Vertex>(0.5f*(v1->pos + v2->pos));
    uPtr<HalfEdge> he1b = mkU<HalfEdge>();
    uPtr<HalfEdge> he2b = mkU<HalfEdge>();

    he1b->vertex = v1;  he1b->face = he1->face;
    he2b->vertex = v2;  he2b->face = he2->face;

    he1b->sym = he2;        he1b->next = he1->next;  he1b->vertex = v1;
    he2b->sym = he1;        he2b->next = he2->next;  he2b->vertex = v2;
    he1->sym = he2b.get();  he1->next = he1b.get();  he1->vertex = v3.get();
    he2->sym = he1b.get();  he2->next = he2b.get();  he2->vertex = v3.get();

    v1->edge = he1b.get();
    v2->edge = he2b.get();
    v3->edge = he2;

    this->vertices.push_back(std::move(v3));
    this->edges.push_back(std::move(he1b));
    this->edges.push_back(std::move(he2b));
}

void Mesh::triangulateFace(Face* f) {
    // dont delete anything. just add

    auto he0 = f->edge;  int numSides = 0;
    do {he0 = he0->next; numSides++;} while (he0 != f->edge);
    if (numSides == 3) {LOG("base case: triangle face"); return;}

    he0 = he0->next;
    uPtr<HalfEdge> heA = mkU<HalfEdge>();
    uPtr<HalfEdge> heB = mkU<HalfEdge>();

    heA->vertex = he0->vertex;
    heB->vertex = he0->next->next->vertex;
    heA->sym = heB.get();  heB->sym = heA.get();

    uPtr<Face> face2 = mkU<Face>();
    face2->edge = heA.get();
    face2->color = f->color;

    heA->face = face2.get();
    he0->next->face = face2.get();
    he0->next->next->face = face2.get();
    heB->face = f;

    heB->next = he0->next->next->next;
    he0->next->next->next = heA.get();
    heA->next = he0->next;
    he0->next = heB.get();

    this->faces.push_back(std::move(face2));
    this->edges.push_back(std::move(heA));
    this->edges.push_back(std::move(heB));

    // we just did one triangle. we can recurse onto the face we didnt create, now with 1 less sides
    Mesh::triangulateFace(f);
}

void Mesh::addSmoothedMidpoint(HalfEdge* he1,
                                       std::unordered_map<Face*, Vertex*>& face_to_cents) {
    // dont delete anything. just add
    Vertex* v1 = he1->vertex;
    HalfEdge* he2 = he1->sym;
    Vertex* v2 = he2->vertex;

    uPtr<Vertex> v3 = mkU<Vertex>(0.25f*(v1->pos + v2->pos + face_to_cents[he1->face]->pos + face_to_cents[he2->face]->pos));

    uPtr<HalfEdge> he1b = mkU<HalfEdge>();
    uPtr<HalfEdge> he2b = mkU<HalfEdge>();

    he1b->vertex = v1;  he1b->face = he1->face;
    he2b->vertex = v2;  he2b->face = he2->face;

    he1b->sym = he2;        he1b->next = he1->next;  he1b->vertex = v1;
    he2b->sym = he1;        he2b->next = he2->next;  he2b->vertex = v2;
    he1->sym = he2b.get();  he1->next = he1b.get();  he1->vertex = v3.get();
    he2->sym = he1b.get();  he2->next = he2b.get();  he2->vertex = v3.get();

    v1->edge = he1b.get();
    v2->edge = he2b.get();
    v3->edge = he2;

    this->edges.push_back(std::move(he1b));
    this->edges.push_back(std::move(he2b));
    this->vertices.push_back(std::move(v3));
}

static std::vector<Vertex*> getOriginalVertices(const Mesh& m) {
    std::vector<Vertex*> originalVerts;
    for (auto& v : m.getVertices()) originalVerts.push_back(v.get());
    return originalVerts;
}
static std::vector<HalfEdge*> getOriginalHalfEdges(const Mesh& m) {
    std::vector<HalfEdge*> originalEdges;
    for (auto& v : m.getEdges()) originalEdges.push_back(v.get());
    return originalEdges;
}
static std::vector<Face*> getOriginalFaces(const Mesh& m) {
    std::vector<Face*> originalFaces;
    for (auto& v : m.getFaces()) originalFaces.push_back(v.get());
    return originalFaces;
}

void Mesh::computeAndAddCentroids(Mesh& m,
                                  std::unordered_map<Face*, Vertex*>& face_to_cents,
                                  std::vector<Face*>& originalFaces) {
    /*
    In this function, we pass over every face, calculate the average of the vertices in that face,
    and add the resulting centroid to the graph.
    */
    for (auto f : originalFaces) {
        glm::vec3 avg_pos = {0,0,0};
        int numSides = 0;
        auto cur = f->edge;
        do {
            avg_pos += cur->vertex->pos;
            numSides++;
            cur = cur->next;
        } while (cur != f->edge);
        avg_pos /= numSides;
        uPtr<Vertex> centroid = mkU<Vertex>(avg_pos);

        face_to_cents[f] = centroid.get();
        m.vertices.push_back(std::move(centroid));
    }
}

void Mesh::addAllSmoothedMidpoints(Mesh& m,
                                   std::unordered_map<Face*, Vertex*>& face_to_cents,
                                   std::vector<HalfEdge*>& originalEdges) {
    /*
    In this function, we pass over all half edges, making sure to skip processing if the SYM has already been split.
    We then split every edge, set the pointers, and add the new vertex and edges to the graph structure.
    */

    std::unordered_set<HalfEdge*> already_split;
    // for each edge, compute smooth midpoint (vertex)
    for (HalfEdge* he : originalEdges) {
        if (already_split.count(he) != 0) continue;
        already_split.insert(he);
        already_split.insert(he->sym);

        addSmoothedMidpoint(he, face_to_cents);
    }
}

void Mesh::smoothAllVertices(Mesh& m,
                             std::unordered_map<Face*, Vertex*>& face_to_cents,
                             std::vector<Vertex*>& originalVerts) {
    /*
    In this function, we traverse through the vertices and compute the correct smoothed position.
    */
    for (Vertex* vertex : originalVerts) {
        // get n by moving in a star around vertex
        auto cur = vertex->edge;
        int n = 0;
        glm::vec3 sumAdjMidpts = {0.f,0.f,0.f};
        glm::vec3 sumCentroids = {0.f,0.f,0.f};
        do {
            sumAdjMidpts += cur->sym->vertex->pos;
            sumCentroids += face_to_cents[cur->face]->pos;
            cur = cur->next->sym;
            n++;
        } while(cur != vertex->edge);

        float frac = 1.f/n;
        vertex->pos = (frac*(float)(n-2)*vertex->pos) +
                      (frac*frac*sumAdjMidpts) +
                      (frac*frac*sumCentroids);
    }
}

void Mesh::quadrangulateAllFaces(Mesh& m,
                                 std::unordered_map<Face*, Vertex*>& face_to_cents,
                                 std::vector<Face*>& originalFaces) {
    /*
    In this function, we traverse through the faces and quadrangulate.
    We can collect all of the edges and
    */
    for (auto origFace : originalFaces) {

        auto centroid = face_to_cents[origFace];

        // collect all of the original edges in the face
        std::vector<HalfEdge*> edges;
        auto c = origFace->edge;
        do {
            edges.push_back(c);
            c = c->next;
        }
        while(c!=origFace->edge);
        int n = edges.size();

        // create the n-1 new faces and store them in a vector.
        std::vector<Face*> newFaces = {origFace};
        for (int i = 1; i < n/2; i++) {
            uPtr<Face> newFace = mkU<Face>();
            newFace->color = origFace->color;
            newFaces.push_back(newFace.get());
            this->faces.push_back(std::move(newFace));
        }

        std::vector<HalfEdge*> newEdges;

        /*
        This is the main logic, where we can loop through the (now) outer edges of the face,
        and create and assign the new inner edges by just indexing into the vectors we collected before.
        */
        for (int i = 0; i < n; i = i+2) {
            uPtr<HalfEdge> a = mkU<HalfEdge>();
            uPtr<HalfEdge> b = mkU<HalfEdge>();

            newEdges.push_back(a.get());
            newEdges.push_back(b.get());

            a->vertex = centroid;
            b->vertex = edges[((i-2)%n+n)%n]->vertex;
            centroid->edge = a.get();

            auto cur = edges[i]; auto prev = edges[((i-1)%n+n)%n];
            a->next = b.get();
            b->next = prev;
            prev->next = cur;
            cur->next = a.get();

            auto newFace = newFaces[i/2];
            a->face = newFace;
            b->face = newFace;
            cur->face = newFace;
            prev->face = newFace;
            newFace->edge = b.get();

            // we can skip the first two vectors, as they'll be assigned in the last loop.
            // each subface contains two outer edges and two new inner edges: prev->cur->a->b->prev.
            if (i>=2) {
                HalfEdge* lastA = newEdges[i+1-3];
                b->sym = lastA;
                lastA->sym = b.get();

                if (i == n-2) {
                    HalfEdge* firstB = newEdges[1];
                    HalfEdge* lastA = newEdges[n-2];
                    firstB->sym = lastA;
                    lastA->sym = firstB;
                }
            }

            this->edges.push_back(std::move(a));
            this->edges.push_back(std::move(b));
        };
    }
}

void Mesh::catmullClark() {
    /*
    This function calls four helper functions that each independently perform a step of the Catmull-Clark algorithm.
    In each helper is a more detailed comment to explain the implemented logic.
    */
    std::vector<Vertex*> originalVerts = getOriginalVertices(*this);
    std::vector<HalfEdge*> originalEdges = getOriginalHalfEdges(*this);
    std::vector<Face*> originalFaces = getOriginalFaces(*this);

    // for each face, compute centroids (vertices) and store in an unorderedmap <Face*, Vertex*> to easily query later
    std::unordered_map<Face*, Vertex*> face_to_cents;

    computeAndAddCentroids(*this, face_to_cents, originalFaces);

    addAllSmoothedMidpoints(*this, face_to_cents, originalEdges);

    // smooth original vertices
    smoothAllVertices(*this, face_to_cents, originalVerts);

    // for every face, quadrangulate
    quadrangulateAllFaces(*this, face_to_cents, originalFaces);

}

// passed in from MyGL::loadOBJ
void Mesh::buildMesh(const std::vector<glm::vec3>& positions, const std::vector<std::vector<int>>& faceIndices) {
    // reset the mesh
    this->vertices.clear();
    this->faces.clear();
    this->edges.clear();

    // First, fill out the vertices
    for (const glm::vec3& pos : positions) {
        auto v = std::make_unique<Vertex>(pos);
        this->vertices.push_back(std::move(v));
    }

    std::map<std::pair<int,int>, HalfEdge*> vertsToEdge;  // stores <source, dest> vertex : halfedge to make setting syms easy
    std::map<HalfEdge*, std::pair<int,int>> edgeToVerts;  // opposite

    // Next, go through the faceIndices and fill out faces and edges
    for (const auto& indices : faceIndices) {  // each indices is a vector of size n, the number of edges on that face
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
        this->faces.push_back(std::move(f));
    }

    // Now point the syms
    for (auto& heUptr : this->edges) {
        HalfEdge* he = heUptr.get();
        auto [a,b] = edgeToVerts[he];
        auto symEdge = vertsToEdge[{b,a}];
        he->sym = symEdge;
    }
}

void Mesh::initializeAndBufferGeometryData() {
    destroyGPUData();
    // the below vectors are for each vertex. must add vertex multiple times, one for each face. (24 for cube)
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> col;
    std::vector<glm::vec3> nor;
    std::vector<GLuint> idx;  // 3*2*6 for cube

    int anchor = 0;
    for(auto& f : this->faces) {
        anchor = pos.size();
        // first, traverse around HEs and push verts in vbo
        HalfEdge* cur = f->edge;

        // every vertex on this face will have the same normal, so calculate it now
        // we are assuming CCW vertex order, so cross product will always be out of face (+)
        // also assuming the mesh is well formed, so catmull clark wont result in 3 colinear vertices
        // EXCEPT when we split an edge ourselves, so just move cur until this isn't the case
        glm::vec3 diff1 = (cur->vertex->pos - cur->next->vertex->pos);
        glm::vec3 diff2 = (cur->next->vertex->pos - cur->next->next->vertex->pos);
        glm::vec3 face_normal = glm::cross(diff1, diff2);
        while (glm::dot(face_normal, face_normal) < 1e-12f) {
            cur = cur->next;
            glm::vec3 diff1 = (cur->vertex->pos - cur->next->vertex->pos);
            glm::vec3 diff2 = (cur->next->vertex->pos - cur->next->next->vertex->pos);
            face_normal = glm::cross(diff1, diff2);
        }

        int numVerts = 0;
        do {
            pos.push_back(cur->vertex->pos);
            col.push_back(f->color);
            nor.push_back(face_normal);
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
    generateBuffer(BufferType::POSITION);
    bindBuffer(BufferType::POSITION);
    bufferData(BufferType::POSITION, pos);

    generateBuffer(BufferType::COLOR);
    bindBuffer(BufferType::COLOR);
    bufferData(BufferType::COLOR, col);

    generateBuffer(BufferType::NORMAL);
    bindBuffer(BufferType::NORMAL);
    bufferData(BufferType::NORMAL, nor);

    generateBuffer(BufferType::INDEX);
    bindBuffer(BufferType::INDEX);
    bufferData(BufferType::INDEX, idx);

    this->indexBufferLength = idx.size();
}

GLenum Mesh::drawMode() {
    return GL_TRIANGLES;
}
