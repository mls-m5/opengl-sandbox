/*
 * common-gl.h
 *
 *  Created on: 24 jun 2015
 *      Author: Mattias Larsson Sköld
 */


#pragma once



#define debug_print(...) printf(__VA_ARGS__)
#define debug_write(...) printf(__VA_ARGS__)


#define BUFFER_OFFSET(offset) ((void *)(offset))

#ifdef __ANDROID__
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define GL_BGRA GL_BGRA_EXT

#else
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

#define GLEW_STATIC

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

#define USING_GLEW

#else

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#include <GL/gl.h>
#include <GL/glext.h>
#endif
#endif


