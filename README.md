# Half-Edge Mesh Editor

This project implements a half-edge mesh data structure and an interactive mesh editor built on top of OpenGL. 
Half-edge meshes enable efficient traversal, adjacency queries and topology edits that are inefficient on simpler index buffer based meshes. 
Essentially, this data structure trades ease of rendering for flexibility, making it perfect for editing.

Features include mesh parsing, Catmull-Clark subdivision and several other geometry processing algorithms (split edge, triangulation/quadrangulation, etc).

<img width="960" height="720" alt="image" src="https://github.com/user-attachments/assets/5e44abdf-dcf2-4e4b-9e79-71c572feda72" />
