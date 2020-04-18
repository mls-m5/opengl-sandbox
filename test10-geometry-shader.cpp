/*
 * test10-geometry-shader.cpp
 *
 *  Created on: 8 maj 2019
 *      Author: Mattias Larsson Sköld
 */

// look at https://learnopengl.com/Advanced-OpenGL/Geometry-Shader

#include "matgui/matgl.h"
#include "matsdl.h"
#include <iostream>

using namespace std;

const char *vertex =
    R"_(
#version 330

in vec3 vPosition;

out VS_OUT {
    vec3 color;
} gs_out;  

void main() {
	gl_Position = vec4(vPosition / 2, 1.);
	gs_out.color = vec3(1., 0., 0.);
}

)_";

const char *geometry =
    R"_(
#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 color;
} gs_in[];  

out vec4 gColor;

void main() {    
	gColor = vec4(gs_in[0].color, 1);
    gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0); 
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4( 0.1, 0.0, 0.0, 0.0);
    EmitVertex();
    
    gl_Position = gl_in[0].gl_Position + vec4( 0., 0.1, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}  

)_";

const char *fragment =
    R"_(
#version 330

in vec4 gColor;
out vec4 fColor;

void main() {
	fColor = gColor;
}

)_";

// clang-format off
std::vector<float> positions = {
		0, 0,
		1, 0,
		1, 1,

		0, 0,
		1, 1,
		0, 1,
};
// clang-format on

int main(int argc, char **argv) {
    int width = 800, height = 600;
    SDL::Window window(
        "hej", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height);
    SDL::GLContext context(window);

    ShaderProgram program(vertex, fragment, geometry);
    program.use();
    GL::VertexArrayObject vao;
    GL::VertexBufferObject vbo(positions, 0, 2);

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        glClearColor(.1f, .4f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        program.use();
        vao.bind();
        //		glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawArrays(GL_POINTS, 0, 6);

        window.swap();
    }
}
