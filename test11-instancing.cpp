/*
 * test11-instancing.cpp
 *
 *  Created on: 8 maj 2019
 *      Author: Mattias Larsson Sköld
 */

// Check https://learnopengl.com/Advanced-OpenGL/Instancing
// for more info

#include "matgui/matgl.h"
#include "matrix.h"
#include "matsdl.h"
#include <iostream>
using namespace std;

namespace {

const char *vertex = R"_(
#version 330

uniform vec3 translations[5000]; // My intel card does not like 10000
uniform mat4 rotation;

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vColor;

out vec3 fColor;

void main() {
	gl_Position = vec4(vPosition, 1) + rotation * vec4(translations[gl_InstanceID], 1);
//	gl_Position.w = gl_Position.y / 100. + .8;
	gl_Position.xy = gl_Position.xy / (1 + gl_Position.z);
	fColor = vColor;
}

)_";

const char *fragment = R"_(
#version 330

in vec3 fColor;
out vec4 fragColor;

void main() {
//	fragColor = vec4(fColor, 1);
	fragColor = vec4(1, 1, 1, 1);
}

)_";

auto s = .01f;

// clang-format off

// Positions and colors
std::vector<float> vertices = {
	0, 0, 0,  1, 0, 0,
	s, 0, 0,  0, 1, 0,
	s, s, 0,  0, 0, 1,
	0, s, 0,  1, 0, 1,
};

// clang-format on

} // namespace

int main(int /*argc*/, char ** /*argv*/) {
    int width = 800, height = 600;
    SDL::Window window("test 11",
                       SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED,
                       width,
                       height,
                       SDL_WINDOW_OPENGL);

    SDL::GLContext context(window);

    ShaderProgram program(vertex, fragment);
    program.use();

    GL::VertexArrayObject vao;
    GL::VertexBufferObject positions(vertices, 0, 4, 6 * sizeof(float), 0);
    GL::VertexBufferObject textures(vertices, 1, 4, 6 * sizeof(float), 2);

    auto stdRand = []() {
        float ret = 0;

        for (int i = 0; i < 12; ++i) {
            ret += ((float)rand() / RAND_MAX - .5);
        }
        return ret / 6;
    };

    bool random = true;
    int pointCount;
    std::vector<float> translations;
    if (random) {
        pointCount = 5000;
        srand(time(0));
        translations.reserve(pointCount * 3);
        for (int i = 0; i < pointCount; ++i) {
            translations.insert(translations.end(),
                                {stdRand(), stdRand(), stdRand()});
        }
    }
    else {
        pointCount = 5000;
        int pointWidth = sqrt(pointCount);
        for (int i = 0; i < pointCount; ++i) {
            translations.insert(translations.end(),
                                {(float)(i / pointWidth) / pointWidth - .5,
                                 (float)(i % pointWidth) / pointWidth - .5,
                                 0});
        }
    }

    glCall(glUniform3fv(0, pointCount, &translations.front()));

    vao.unbind();

    bool running = true;
    float time = 0;
    auto locationIndex = program.getUniform("rotation");
    Matrixf rotation = Matrixf::Identity();
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        time += .01;
        rotation = Matrixf::RotationX(time);

        glClearColor(0, 0, .4, 1);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        vao.bind();
        program.use();

        glCall(glUniformMatrix4fv(locationIndex, 1, false, rotation));

        glCall(glDrawArraysInstanced(GL_TRIANGLES, 0, 3, pointCount));

        window.swap();
    }
}
