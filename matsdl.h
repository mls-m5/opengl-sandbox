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

class Window {
public:
	Window (const std::string &title, int x, int y, int w, int h, Uint32 flags) {
		window = SDL_CreateWindow(title.c_str(), x, y, w, h, flags);
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
