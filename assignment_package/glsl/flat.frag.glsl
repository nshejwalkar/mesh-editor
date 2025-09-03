#version 330 core

// Refer to the lambert shader files for useful comments

in vec3 fs_Col;

out vec3 out_Col;

void main()
{
    // Copy the color; there is no shading.
    out_Col = fs_Col;
}
