/*
 * test9-2dshadow.cpp
 *
 *  Created on: 7 maj 2019
 *      Author: Mattias Larsson Sköld
 */

#include "matengine/matrix.h"
#include "matgui/matgl.h"
#include "matsdl.h"
#include <iostream>

using namespace std;

namespace plainShader {

const char *vertex =
    R"_(
#version 330

layout (location = 0) in vec3 vPosition;
uniform mat4 location;
uniform vec3 lampPos;

void main() {
	vec4 p = location * vec4(vPosition, 1);
	vec3 d = p.xyz - lampPos;
	d.z = 0;
	vec3 e = d / length(d) * 10.;
//	vec3 e = d;
//	e.z = 0;
	gl_Position =  p + vec4(e * p.z, 0);
}

)_";

const char *fragment =
    R"_(
#version 330

out vec4 fragColor;

void main() {
	fragColor = vec4(1, 1, 1, 1);
}

)_";

} // namespace plainShader

namespace screenShader {
const char *vertex =
    R"_(
#version 330

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vTexCoord;

out vec2 fTexCoord;

void main() {
	gl_Position = vec4(vPosition, 1.);
	fTexCoord = vTexCoord;
}
)_";

const char *fragment =
    R"_(
#version 330

in vec2 fTexCoord;

uniform sampler2D texture1;

out vec4 fragColor;

void main() {
	float c = texture(texture1, fTexCoord).x;
	fragColor = vec4(c, c ,c, c);
//	fragColor = vec4(1,1,1,1);
}
)_";

} // namespace screenShader

namespace {

float s = .1;

// clang-format off
std::vector<float> objectVertices = {
		0, 0, 0,
		0, 0, 1,
		s, 0, 0,
		s, 0, 1,
		s, s, 0,
		s, s, 1,
		0, s, 0,
		0, s, 1,
};


std::vector<float> screenPositions = {
		-1, -1, 0,
		 1, -1, 0,
		 1,  1, 0,
		-1,  1, 0,
};

std::vector<float> screenTexCoords = {
		 0, 1,
		 1, 1,
		 1, 0,
		 0, 0,
};

std::vector<unsigned> screenIndices = {
		0, 1, 2,
		0, 2, 3,
};

// clang-format on

} // namespace

int main(int /*argc*/, char ** /*argv*/) {
    int width = 800, height = 600;
    SDL::Window window("test9",
                       SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED,
                       width,
                       height,
                       SDL_WINDOW_OPENGL);

    SDL::GLContext context(window);

    GL::VertexArrayObject vao;
    GL::VertexBufferObject vboObject(objectVertices, 0, 3);

    std::vector<GLuint> objectIndices;
    {
        auto ins = [&objectIndices](std::initializer_list<GLuint> objects) {
            objectIndices.insert(objectIndices.end(), objects);
        };

        for (int i = 0; i < 4; ++i) {
            int ind1 = (i * 2);
            int ind2 = ((i + 1) % 4) * 2;
            ins({ind1, ind2, ind1 + 1});
            ins({ind2, ind2 + 1, ind1 + 1});
        }
        ins({0, 2, 4});
        ins({0, 4, 6});

        ins({1, 3, 5});
        ins({1, 5, 7});
    }
    GL::VertexBufferObject vboObjectIndices(objectIndices);

    ShaderProgram objectShader(plainShader::vertex, plainShader::fragment);

    objectShader.use();

    glLineWidth(4);

    vao.unbind();

    // ------------ frame buffer object ---------------------

    ShaderProgram screenShader(screenShader::vertex, screenShader::fragment);
    int w = 400, h = 400;
    GL::FrameBufferObject fbo(w, h);
    //	GL::TextureAttachment texAttach(w, h);
    GL::DepthTextureAttachment depthAttach(w, h);

    // ----- vao for screen -------------------
    GL::VertexArrayObject screenVao;
    GL::VertexBufferObject screenVbo(screenPositions, 0, 3);
    GL::VertexBufferObject screenTexVbo(screenTexCoords, 1, 2);
    GL::VertexBufferObject screenIndicesVbo(screenIndices);

    auto running = true;

    auto angle = 0.;

    Vec lampPos;

    //	auto lampPosIndex = objectShader.getUniform("lampPos");

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_MOUSEMOTION) {
                lampPos = Vec((double)event.motion.x / width * 2. - 1.,
                              (double)event.motion.y / height * 2. - 1.);
            }
        }

        angle += .01;

        Matrixf location = Matrixf::RotationX(0) * Matrixf::RotationZ(angle);
        objectShader.use();
        glCall(glUniformMatrix4fv(0, 1, false, location));
        //		glCall(glUniform3dv(lampPosIndex, 1, &lampPos.x));
        glCall(glUniform3f(1, lampPos.x, lampPos.y, lampPos.z));

        vao.bind();

        depthAttach.unbind();
        fbo.bind();

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glDrawElements(GL_TRIANGLES, objectIndices.size(), GL_UNSIGNED_INT, 0);

        Matrixf location2 = Matrixf::Translation(.5, 0, 0);
        glCall(glUniformMatrix4fv(0, 1, false, location2));

        glDrawElements(GL_TRIANGLES, objectIndices.size(), GL_UNSIGNED_INT, 0);

        fbo.unBind(width, height);

        glClearColor(.4, .4, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        screenVao.bind();
        depthAttach.bind();
        screenShader.use();

        glDrawElements(GL_TRIANGLES, screenIndices.size(), GL_UNSIGNED_INT, 0);

        window.swap();
    }
}
