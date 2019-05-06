#if 0

g++ $0 shaderprogram.cpp -std=c++11 -g -o test7-framebuffer -lSDL2 -lGL  -fmax-errors=5 -Imatengine/ -O0

exit
#endif

/*
 * test7-framebuffer.cpp
 *
 *  Created on: 5 maj 2019
 *      Author: Mattias Larsson Sköld
 */


#include "matgl.h"
#include "matsdl.h"
#include <iostream>
#include <memory>
#include <vector>

using namespace std;

using namespace GL;

void checkSDLError(const std::string& line) {
	const char *error = SDL_GetError();
	if (*error != '\0') {
		cerr << "SDL error at " << line << ":" << error << endl;
		throw error;
		SDL_ClearError();
	}
}

namespace plainShader {

const string vertex =
R"_(
#version 330 core

in vec4 position;
in mat4 location;

void main() {
	gl_Position = position;
}

)_";


const string fragment =
R"_(
#version 330 core

out vec4 fragColor;

void main() {
	fragColor = vec4(1, .4, 1, 1);
}

)_";


} //Namespace


namespace texturedShader {

const string vertex =
R"_(

#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
    TexCoords = aTexCoords;
}  

)_";


const string fragment =
R"_(

#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{ 
//    FragColor = vec4(vec3(1 - texture(screenTexture, TexCoords)), 1);
    FragColor = texture(screenTexture, TexCoords);
}

)_";


} //Namespace
vector<float> vertices = {
		0, 0, 0,
		1, 0, 0,
		1, 1, 0,
		0, 1, 0,
};

vector<float> objectVertices = {
		.1, .4, 0,
		.5, .1, 0,
		5, .5, 0,
		.1, .5, 0,
};

vector<float> texCoords = {
		0, 0,
		1, 0,
		1, 1,
		0, 1,
};

vector<GLuint> indices = {
		0, 1, 2,
		0, 2, 3
};

int main(int argc, char **argv) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		cerr << "could not init sdl video" << endl;
		return -1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    int width = 800, height = 600;

    SDL::Window window("Hej", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (!window) {
    	cerr << "could not create main window" << endl;
    	return -1;
    }

    SDL::GLContext context(window);
    checkSDLError(to_string(__LINE__));





    // For rendering the world -----------------------------------------------------------
	ShaderProgram worldProgram(plainShader::vertex, plainShader::fragment);
	GL::VertexArrayObject vao;

	GL::VertexBufferObject ObjectVertexBuffer(objectVertices, 0, 3);

	GL::VertexBufferObject ObjectElementBuffer(indices);

	worldProgram.use();

	vao.unbind();


	// -------------------------- define frame buffer object ------------------------------------------

	int w = 30, h = 20;
	FrameBufferObject fbo(w, h);
	TextureAttachment texAttachment(w, h);
	DepthBufferAttachment depthAttachment(w, h);
	fbo.unBind();

	ShaderProgram screenProgram(texturedShader::vertex, texturedShader::fragment);
	// -- Rendering to the screen ---------------------------------------------
	VertexArrayObject screenVao;

	GL::VertexBufferObject screenVertexBuffer(vertices, 0, 3);
	GL::VertexBufferObject screenTextureCoordinates(texCoords, 1, 2);
	GL::VertexBufferObject screenElementBuffer(indices);

	screenProgram.use();
	texAttachment.bind();

	screenVao.unbind();

	// --------------------------------------------------------------------------------


	SDL_GL_SetSwapInterval(1);

	bool running = true;

	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN:
					///

					break;

				case SDL_KEYUP:
					//

					break;
				case SDL_QUIT:
					running = false;
					break;
				default:
					break;
			}
		}


		vao.bind();
		fbo.bind();
		glClearColor(0, .4, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

//		worldProgram.use();
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		// Render to screen
		screenVao.bind();
		fbo.unBind(width, height); // Draw to screen

		glClearColor(0, 0, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		texAttachment.bind();
		depthAttachment.bind();

//		screenProgram.use();
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		window.swap();
	}

	SDL_Quit();
}

