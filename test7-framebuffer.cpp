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

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#include <GL/gl.h>
#include <GL/glext.h>
#include <iostream>
#include <SDL2/SDL.h>
#include "shaderprogram.h"
#include <memory>
#include <vector>

using namespace std;

namespace GL {
	template <typename T>
	GLenum getType();

	template <>
	GLenum getType<double>() {
		return GL_DOUBLE;
	}

	template <>
	GLenum getType<float>() {
		return GL_FLOAT;
	}

	template <>
	GLenum getType<GLuint>() {
		return GL_UNSIGNED_INT;
	}

	template <>
	GLenum getType<GLint>() {
		return GL_INT;
	}

	class Window {
	public:
		Window (const string &title, int x, int y, int w, int h, Uint32 flags) {
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

	class Context {
	public:
		Context(SDL_Window *window) {
			context = SDL_GL_CreateContext(window);
		}

		~Context() {
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

	class VertexBufferObject {
	public:
		// GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER is the most common
		// More info on https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBufferData.xhtml
		VertexBufferObject(GLenum target = GL_ARRAY_BUFFER): target(target) {
			glCall(glGenBuffers(1, &id));
			bind();
		}

		// Shorthand version
		// if size == 0 it will be calculated from the size of the vector
		// index is the index in the shader program
		// size is the size of each element eg 3 for 3d-position
		// stride is how far it is between each element in bytes
		// start is the starting position in bytes
		template <class T>
		VertexBufferObject(
				const std::vector<T> &data,
				GLuint index, GLuint size,
				GLuint stride = 0, size_t start = 0,
				GLenum target = GL_ARRAY_BUFFER,
				GLenum usage = GL_STATIC_DRAW):
				VertexBufferObject(target) {
			setData(data, usage);
			attribPointer(index, size, getType<T>(), false, stride, (void*)start);
		}

		// A specialized version for element buffers
		VertexBufferObject(const std::vector<GLuint> &indices): VertexBufferObject(GL_ELEMENT_ARRAY_BUFFER){
			setData(indices);
		}

		VertexBufferObject(VertexBufferObject &&other): id(other.id), target(other.target) {
			other.id = 0;
		}

		VertexBufferObject(const VertexBufferObject &) = delete;

		~VertexBufferObject() {
			glCall(glGenBuffers(1, &id));
		}

		void bind() {
			glCall(glBindBuffer(target, id));
		}

		void unbind() {
			glCall(glBindBuffer(target, 0));
		}

		// Set attribute pointer
		// and enable it if it is not GL_ELEMENT_ARRAY_BUFFER
		// Index is the index in the shader program
		// size is the total number of bytes for each element eg. 3 for a 3d position
		// type is the data type eg. GL_FLOAT or GL_DOUBLE
		// stride is if you have some other format like and needs the size of each element to be bigger
		// pointer is the offset of the first element, probably used in combination with stride
		void attribPointer(
				GLuint index,
				GLint size,
				GLenum type,
				GLboolean normalized = false,
				GLsizei stride = 0,
				const void *pointer = nullptr
		) {
			glCall(glVertexAttribPointer(index, size, type, normalized, stride, pointer));
			if (type != GL_ELEMENT_ARRAY_BUFFER) {
				glCall(glEnableVertexAttribArray(index));
			}
		}

		// Binds and set data
		template <class T>
		void setData(const std::vector<T> &data, GLenum usage = GL_STATIC_DRAW) {
			bind();
			glCall(glBufferData(target, sizeof(T) * data.size(), &data.front(), usage));
		}

		GLuint id;
		GLenum target;
	};

	class FrameBufferObject {
	public:
		FrameBufferObject(int width, int height):
		width(width), height(height){
			glCall(glGenFramebuffers(1, &id));
			glCall(glBindFramebuffer(GL_FRAMEBUFFER, id));
			glCall(glDrawBuffer(GL_COLOR_ATTACHMENT0));
		}

		// Setup opengl to render to this framebuffer
		void bind() {
			glBindTexture(GL_TEXTURE_2D, 0); // Make sure to unbind any textures
			glCall(glBindFramebuffer(GL_FRAMEBUFFER, id));
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				throw runtime_error("Framebuffer is not complete");
			}
			glCall(glViewport(0, 0, width, height));
		}

		// Sets opengl to render to the default framebuffer (the screen)
		static void unBind() {
			glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		}

		static void unBind(int w, int h) {
			unBind();
			glCall(glViewport(0, 0, w, h));
		}

		~FrameBufferObject() {
			glDeleteFramebuffers(1, &id);
		}

		GLuint id;
		int width, height;
	};

	class TextureAttachment {
	public:
		TextureAttachment(int width, int height) {
			glCall(glGenTextures(1, &id));
			bind();
			glCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
					GL_RGB, GL_UNSIGNED_BYTE, nullptr));
			glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			glCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, id, 0));
			unbind();
		}

		~TextureAttachment() {
			glCall(glDeleteTextures(1, &id));
		}

		void bind() {
			glCall(glBindTexture(GL_TEXTURE_2D, id));
		}

		void unbind() {
			glCall(glBindTexture(GL_TEXTURE_2D, 0));
		}

		GLuint id;
	};

	// A depth buffer that may be used to render somewhere else
	class DepthTextureAttachment {
	public:
		DepthTextureAttachment(int width, int height) {
			glCall(glGenTextures(1, &id));
			bind();
			glCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0,
					GL_DEPTH_COMPONENT, GL_RGB, nullptr));
			glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			glCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, id, 0));
			unbind();
		}

		~DepthTextureAttachment() {
			glCall(glDeleteTextures(1, &id));
		}

		void bind() {
			glCall(glBindTexture(GL_TEXTURE_2D, id));
		}

		void unbind() {
			glCall(glBindTexture(GL_TEXTURE_2D, 0));
		}

		GLuint id;
	};

	// A depth buffer that is not used to render to anywhere else
	class DepthBufferAttachment {
	public:
		DepthBufferAttachment (int width, int height) {
			glGenRenderbuffers(1, &id);
			bind();
			glCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height));
			glCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, id));
			unbind();
		}

		void bind() {
			glCall(glBindRenderbuffer(GL_RENDERBUFFER, id));
		}

		void unbind() {
			glCall(glBindRenderbuffer(GL_RENDERBUFFER, 0));
		}

		~DepthBufferAttachment() {
			glCall(glDeleteRenderbuffers(1, &id));
		}

		GLuint id;
	};
}

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

    GL::Window window("Hej", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (!window) {
    	cerr << "could not create main window" << endl;
    	return -1;
    }

    GL::Context context(window);
    checkSDLError(to_string(__LINE__));





    // For rendering the world -----------------------------------------------------------
	ShaderProgram worldProgram(plainShader::vertex, plainShader::fragment);
	GL::VertexArrayObject vao;
	{
		GL::VertexBufferObject vertexBuffer(objectVertices, 0, 3);

		GL::VertexBufferObject elementBuffer(indices);
	}

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
	{
		GL::VertexBufferObject vertexBuffer(vertices, 0, 3);
		GL::VertexBufferObject textureCoordinates(texCoords, 1, 2);
		GL::VertexBufferObject elementBuffer(indices);
	}

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

		worldProgram.use();
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		// Render to screen
		screenVao.bind();
		fbo.unBind(width, height); // Draw to screen

		glClearColor(0, 0, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		texAttachment.bind();
		depthAttachment.bind();

		screenProgram.use();
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		window.swap();
	}

	SDL_Quit();
}

