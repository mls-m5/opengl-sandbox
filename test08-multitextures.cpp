/*
 * test8-multitextures.cpp
 *
 *  Created on: 7 maj 2019
 *      Author: Mattias Larsson Sköld
 */


#include <iostream>

#include "matgl.h"
#include "matsdl.h"
#include "shaderprogram.h"

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
uniform sampler2D texture2;


void main() {
	fragColor = texture(texture1, fTexCoords) + texture(texture2, fTexCoords);
//	fragColor = texture(texture1, fTexCoords);
//	fragColor = texture(texture2, fTexCoords);
}


)_";

}

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

using namespace GL;

int main(int argc, char **argv) {
	cout << "started..." << endl;
	int width = 800;
	int heigth = 600;
	SDL::Window window(
			"Hej",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width,
			heigth,
			SDL_WINDOW_OPENGL
	);

	SDL::GLContext context(window);

	ShaderProgram program(shader::vertex, shader::fragment);

	GL::VertexArrayObject vao;
	program.use();

	GL::VertexBufferObject vboPos(positions, 0, 3);
	GL::VertexBufferObject vboTex(texCoords, 1, 2);
	GL::VertexBufferObject vboInd(indices);

	glUniform1i(program.getUniform("texture1"), 0);
	glUniform1i(program.getUniform("texture2"), 1);


	// ------------- Create textures ------------------------------

	GL::Texture tex1(texData1, 4, 4, GL_RGB, false, false);
	GL::Texture tex2(texData2, 4, 4, GL_RGB, false, false);
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
		glActiveTexture(GL_TEXTURE1);
		tex2.bind();

		glCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));

		window.swap();
	}

	cout << "finished" << endl;
	return 0;
}



