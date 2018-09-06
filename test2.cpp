#if 0

g++ $0 shaderprogram.cpp -std=c++11 -g -o test2 -lSDL2 -lGL  -fmax-errors=5

exit
#endif


#include <stdio.h>
#include <stdint.h>
#include <assert.h>


//#define GL_GLEXT_PROTOTYPES
//
//#include <SDL2/SDL.h>
//#include <SDL2/SDL_opengl.h>


#include "shaderprogram.h"

#include <SDL2/SDL.h>
//#include <SDL2/SDL_opengl.h>
//#include <GL/gl.h>
#include <memory>
#include <iostream>


using namespace std;

enum VAO_IDs {
	Triangles, NumVAOs
};
enum Buffer_IDs {
	ArrayBuffer, NumBuffers
};
enum Attrib_IDs {
	vPosition = 0
};

GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
const GLuint NumVertices = 6;

std::string vertexShader = R"__(

#version 330 core

in vec4 vPosition;
in vec4 vColor;

out vec4 color;

uniform mat4 mvpMatrix;

void main() {
   //color = vColor;
   color = vec4(1,1,1,1); //debugging
   //gl_Position = mvpMatrix * vPosition;
   //vec4 pos2 = vec4(.3,.3,.3,0);
   gl_Position = vPosition + pos2;
}
)__";

std::string fragmentShader = R"__(
#version 330 core

//in vec4 color;

//out vec3 fColor;

void main() {
   //fColor = vec3(color);
   //fColor = vec3(1,1,1);
   gl_FragColor = vec4(.4,1,0,1);
}

)__";

ShaderProgram shaderProgram;

//
// init
//
void init(void)
{
	glCall(glGenVertexArrays(NumVAOs, VAOs));
	glCall(glBindVertexArray(VAOs[Triangles]));
	GLfloat vertices[NumVertices][2] = {
		{	-0.90, -0.90}, // Triangle 1
		{	0.85, -0.90},
		{	-0.90, 0.85},
		{	0.90, -0.85}, // Triangle 2
		{	0.90, 0.90},
		{	-0.85, 0.90}
	};

	glCall(glGenBuffers(NumBuffers, Buffers));
	glCall(glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]));
	glCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
			vertices, GL_STATIC_DRAW));
	shaderProgram.initProgram(vertexShader, fragmentShader);
	shaderProgram.useProgram();
	glCall(glVertexAttribPointer(vPosition, 2, GL_FLOAT,
			false, 0, BUFFER_OFFSET(0)));
	glCall(glEnableVertexAttribArray(vPosition));

}


void render() {
	glClearColor(0.f, 1.f, 1.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glCall(glBindVertexArray (VAOs[Triangles]));
	glCall(glDrawArrays(GL_TRIANGLES, 0, NumVertices));
	glCall(glFlush());
}


void die(std::string message) {
	cout << message << endl;
	throw message;
}


int main (int ArgCount, char **Args)
{
	auto width = 500;
	auto height = 500;

	if (SDL_Init(SDL_INIT_VIDEO) > 0) {
		die("failed to load video");
	}

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


	auto windowFlags = SDL_WINDOW_OPENGL;
	auto window = shared_ptr<SDL_Window>(SDL_CreateWindow(
			"Opengl playground", 0, 0, width, height, windowFlags),
			SDL_DestroyWindow);

	if (!window) {
		die("could not create window");
	}

	auto context = shared_ptr<void>(
			SDL_GL_CreateContext(window.get()),
			SDL_GL_DeleteContext);

	init();


	SDL_GL_SetSwapInterval(1);

	bool running = 1;
	bool fullScreen = 0;
	while (running) {
		SDL_Event Event;
		while (SDL_PollEvent(&Event)) {
			if (Event.type == SDL_KEYDOWN) {
				switch (Event.key.keysym.sym) {
				case SDLK_ESCAPE:
					running = 0;
					break;
//				case 'f':
//					fullScreen = !fullScreen;
//					if (fullScreen) {
//						SDL_SetWindowFullscreen(window.get(),
//								windowFlags | SDL_WINDOW_FULLSCREEN_DESKTOP);
//					} else {
//						SDL_SetWindowFullscreen(window.get(), windowFlags);
//					}
//					break;
//				default:
//					break;
				}
			} else if (Event.type == SDL_QUIT) {
				running = 0;
			}
		}

		glViewport(0, 0, width, height);
//		glClearColor(0.f, 1.f, 1.f, 0.f);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
		render();
		SDL_GL_SwapWindow(window.get());
	}
	return 0;
}
