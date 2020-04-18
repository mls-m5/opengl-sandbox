/*
 * test8-multitextures.cpp
 *
 *  Created on: 12 maj 2019
 *      Author: Mattias Larsson Sköld
 */

#include <iostream>

#include "matgui/matgl.h"
#include "matgui/shaderprogram.h"
#include "matgui/texture.h"
#include "matsdl.h"

using namespace std;

namespace shader {

const string vertex = R"_(
#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 vTexCoords;

out vec2 fTexCoords;

void main() {
	gl_Position = vec4(position, 1);
	fTexCoords = vTexCoords;
}

)_";

const string fragment = R"_(
#version 330

in vec2 fTexCoords;
out vec4 fragColor;

uniform sampler2D texture1;


void main() {
	fragColor = texture(texture1, fTexCoords);
}


)_";

} // namespace shader

namespace {

// clang-format off
std::vector<float> positions = {
        -1,  1, 0,
         1,  1, 0,
         1, -1, 0,
        -1, -1, 0,
};

std::vector<float> texCoords = {
        0, 1,
        1, 1,
        1, 0,
        0, 0,
};

std::vector<GLuint> indices = {
        0, 1, 2,
        0, 2, 3,
};

#define on 255, 255, 255
#define re 255, 0, 0
#define bl 0, 0, 255
// off misspelled as of to match length
#define of 0, 0, 0

vector <unsigned char> texData1 = {
        bl, of, bl, of,
        bl, of, bl, of,
        of, bl, of, bl,
        of, bl, of, bl,
};

vector <unsigned char> texData2 = {
        re, of, re, of,
        of, re, of, re,
        re, of, re, of,
        of, re, of, re,
};

#undef on
#undef of

// clang-format on

} // namespace

using namespace GL;

int main(int /*argc*/, char ** /*argv*/) {
    cout << "started..." << endl;
    int width = 800;
    int heigth = 600;
    SDL::Window window("Hej",
                       SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED,
                       width,
                       heigth,
                       SDL_WINDOW_OPENGL);

    SDL::GLContext context(window);

    ShaderProgram program(shader::vertex, shader::fragment);

    GL::VertexArrayObject vao;
    program.use();

    GL::VertexBufferObject vboPos(positions, 0, 3);
    GL::VertexBufferObject vboTex(texCoords, 1, 2);
    GL::VertexBufferObject vboInd(indices);

    glUniform1i(program.getUniform("texture1"), 0);

    // ------------- Create textures ------------------------------

    //	GL::Texture tex1(texData1, 4, 4, GL_RGB, false, false);

    int texWidth = 500, texHeight = 500;
    vector<unsigned char> pixels(texWidth * texHeight * 3);

    for (auto &p : pixels) {
        p = 0;
    }

    unsigned char outside = 0;

    auto getPixel = [&](int x, int y) -> unsigned char & {
        if (x < 0 || x >= texWidth || y < 0 || y >= texHeight) {
            return outside;
        }
        int i = (x + y * texWidth) * 3;
        return pixels[i];
    };

    for (int x = 30; x < 100; ++x)
        for (int y = 50; y < 100; ++y) {
            int i = (x + y * texWidth) * 3;
            pixels[i] = 255;
            pixels[i + 1] = 255;
            pixels[i + 2] = 255;
        }

    auto max = [](unsigned char c1, unsigned char c2) {
        return (c1 > c2) ? c1 : c2;
    };
    auto subtract = [](unsigned char c) { return (c > 0) ? c - 1 : 0; };

    for (int x = 0; x < texWidth; ++x)
        for (int y = 0; y < texHeight; ++y) {
            unsigned char &p = getPixel(x, y);
            p = max(p, subtract(getPixel(x - 1, y)));
            //		p = max(p, subtract(getPixel(x - 1, y - 1)));
            p = max(p, subtract(getPixel(x, y - 1)));
        }

    for (int x = texWidth - 1; x > 0; --x)
        for (int y = texHeight - 1; y > 0; --y) {
            unsigned char &p = getPixel(x, y);
            p = max(p, subtract(getPixel(x + 1, y)));
            //		p = max(p, subtract(getPixel(x + 1, y + 1)));
            p = max(p, subtract(getPixel(x, y + 1)));
        }

    GL::Texture tex1; //(texData1, 4, 4, GL_RGB, false, false);
    tex1.setData(pixels, texWidth, texHeight);
    tex1.setParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    tex1.setParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    vao.unbind();

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        glClearColor(.5, 1, 0, 1);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        vao.bind();
        program.use();

        glActiveTexture(GL_TEXTURE0);
        tex1.bind();

        glCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));

        window.swap();
    }

    cout << "finished" << endl;
    return 0;
}
