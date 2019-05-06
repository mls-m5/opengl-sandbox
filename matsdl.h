/*
 * matsld.h
 *
 *  Created on: 6 maj 2019
 *      Author: mattias
 */

#pragma once

#include <string>
#include <SDL2/SDL.h>

namespace SDL {

void checkError(const std::string& line) {
	const char *error = SDL_GetError();
	if (*error != '\0') {
		std::cerr << "SDL error at " << line << ":" << error << std::endl;
		throw error;
		SDL_ClearError();
	}
}

class Window {
public:
	Window (const std::string &title, int x, int y, int w, int h, Uint32 flags) {
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			throw std::runtime_error( "could not init sdl video");
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );
	    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		window = SDL_CreateWindow(title.c_str(), x, y, w, h, flags);

	    if (!window) {
	    	throw std::runtime_error("could not create main window");
	    }
	}

	~Window() {
		SDL_DestroyWindow(window);
	}

	operator SDL_Window *() {
		return window;
	}

	void swap() {
		SDL_GL_SwapWindow(window);
	}

	SDL_Window *window;
};

class GLContext {
public:
	GLContext(SDL_Window *window) {
		context = SDL_GL_CreateContext(window);
		SDL::checkError(std::to_string(__LINE__));
	}

	~GLContext() {
		SDL_GL_DeleteContext(context);
	}

	SDL_GLContext context;
};

class VertexArrayObject  {
public:
	VertexArrayObject() {
		glCall(glGenVertexArrays(1, &id));
		bind();
	}

	void bind() {
		glCall(glBindVertexArray(id));
	}

	void unbind() {
		glCall(glBindVertexArray(0));
	}

	~VertexArrayObject() {
		glCall(glDeleteVertexArrays(1, &id));

	}
	GLuint id;
};



} // namespace SDL
