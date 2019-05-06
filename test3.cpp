#if 0

g++ $0 shaderprogram.cpp -std=c++11 -g -o test3 -lSDL2 -lGL  -fmax-errors=5 -Imatengine/

exit
#endif

/*
 * test3.cpp
 *
 *  Created on: 6 sep. 2018
 *      Author: mattias
 */


#include <stdio.h>
#include <stdlib.h>
// If using gl3.h
/* Ensure we are using opengl's core profile only */

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#include <GL/gl.h>
#include <GL/glext.h>

#include <SDL2/SDL.h>

#include "matrix.h"
#include "shaderprogram.h"


std::string vertexShader = R"__(
#version 330 core

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec4 vColor;

out vec3 fColor;

uniform mat4 mvpMatrix;

void main() {
   fColor = vec3(0,0,1);
   gl_Position = vPosition;
}
)__";

std::string fragmentShader = R"__(

#version 330 core

in vec3 fColor;
out vec3 color;

void main(){
  color = fColor;
}

)__";





// A simple function that prints a message, the error code returned by SDL,
// and quits the application
void sdldie(const char *msg);


void checkSDLError(int line = -1);

// An array of 3 vectors which represents 3 vertices
static const GLfloat g_vertex_buffer_data[] = {
   -1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
   0.0f,  1.0f, 0.0f,
};

GLuint vertexbuffer;
ShaderProgram program;

void init() {
	// This will identify our vertex buffer
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	program.initProgram(vertexShader, fragmentShader);
}

//Rendering function
void render(){
	static float x = 0;
//	drawSquare(Vec(.1 + x, 50, 1), 20 + x * 2, 100,100, DRAW_STYLE_FILLED);
	x += 20;

	program.use();

	// 1st attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
	   0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	   3,                  // size
	   GL_FLOAT,           // type
	   GL_FALSE,           // normalized?
	   0,                  // stride
	   (void*)0            // array buffer offset
	);
	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
	glDisableVertexAttribArray(0);

	program.unuse();
}


/* Our program's entry point */
int main(int argc, char *argv[])
{
    SDL_Window *mainwindow; /* Our window handle */
    SDL_GLContext maincontext; /* Our opengl context handle */

    if (SDL_Init(SDL_INIT_VIDEO) < 0) /* Initialize SDL's Video subsystem */
        sdldie("Unable to initialize SDL"); /* Or die on error */

    /* Request opengl 3.2 context.
     * SDL doesn't have the ability to choose which profile at this time of writing,
     * but it should default to the core profile */
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2); //This prevents the rendering function from rendering anything of some reason

//    Turn on double buffering with a 24bit Z buffer.
//    You may need to change this to 16 or 32 for your system
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create our window centered at 512x512 resolution
    mainwindow = SDL_CreateWindow("sdl-window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!mainwindow) /* Die if creation failed */
        sdldie("Unable to create window");

    checkSDLError(__LINE__);


    // Create our opengl context and attach it to our window
    maincontext = SDL_GL_CreateContext(mainwindow);
    checkSDLError(__LINE__);

//    initDrawModule(512, 512);
    init();

    // This makes our buffer swap syncronized with the monitor's vertical refresh
    SDL_GL_SetSwapInterval(1);

    for (int i = 0; i < 10; ++i) {
    	// Clear our buffer with a red background
    	glClearColor ( 0.1 * i, 0.0, 0.0, 1.0 );
    	glClear ( GL_COLOR_BUFFER_BIT );
    	render();
    	//Swap our back buffer to the front
    	SDL_GL_SwapWindow(mainwindow);
    	// Wait
    	SDL_Delay(200);
    }

//    QuitDrawModule();

    // Delete our opengl context, destroy our window, and shutdown SDL
    SDL_GL_DeleteContext(maincontext);
    SDL_DestroyWindow(mainwindow);
    SDL_Quit();

    return 0;
}


void checkSDLError(int line)
{
#ifndef NDEBUG
	const char *error = SDL_GetError();
	if (*error != '\0')
	{
		printf("SDL Error: %s\n", error);
		if (line != -1)
			printf(" + line: %i\n", line);
		SDL_ClearError();
	}
#endif
}
void sdldie(const char *msg)
{
    printf("%s: %s\n", msg, SDL_GetError());
    SDL_Quit();
    exit(1);
}



