/*
 * matgl.h
 * A wrapper around some openGL-objects to improve productivity
 * and increase joy of programming
 *
 *  Created on: 6 maj 2019
 *      Author: Mattias Larsson Sköld
 */

#pragma once

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#include <GL/gl.h>
#include <GL/glext.h>
#include <SDL2/SDL.h>
#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "shaderprogram.h"

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
		glCall(glDeleteBuffers(1, &id));
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
			throw std::runtime_error("Framebuffer is not complete");
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

} // namespace GL
