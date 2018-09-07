/*
 * shaderProgram.h
 *
 *  Created on: 16 mar 2014
 *      Author: mattias
 */

#pragma once

#include "common-gl.h"

#include <string>

class ShaderProgram {
public:
	ShaderProgram(ShaderProgram &s){
		this->_program = s._program;
	}
	ShaderProgram();
	ShaderProgram(std::string vertexCode, std::string fragmentCode);
	void initProgram(std::string vertexCode, std::string fragmentCode);
	void loadShaderFromFile(std::string vertexFile, std::string fragmentFile);

	inline void useProgram() __attribute__ ((deprecated)) {
		use();
	}

	void use();
	void unuse();

	GLuint getProgram() { return _program; };
	GLint getUniform( char const* name );
	GLint getAttribute( char const* name );
	virtual ~ShaderProgram();

private:
	GLuint _program = 0;
};


class StandardShaderProgram: public ShaderProgram {
public:

	GLuint vertexPointer;
	GLuint colorPointer;
	GLuint mvpMatrixPointer;

	StandardShaderProgram(std::string vertexCode, std::string fragmentCode);
	void disable();
};


static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    debug_print("GL %s = %s\n", name, v);
}


static int checkGlError(const char* op, bool throwError = false) {
	bool ret = false;
    for (GLint error = glGetError(); error; error
            = glGetError()) {
    	const char * c;
    	switch (error) {
    	case 0x0500:
    		c = "GL_INVALID_ENUM";
    		break;
    	case 0x0501:
    		c = "GL_INVALID_VALUE";
    		break;
    	case 0x0502:
    		c = "GL_INVALID_OPERATION";
    		break;
    	case 0x0503:
    		c = "GL_STACK_OVERFLOW";
    		break;
    	case 0x0504:
    		c = "GL_STACK_UNDERFLOW";
    		break;
    	case 0x0505:
    		c = "GL_OUT_OF_MEMORY";
    		break;
    	case 0x0506:
    		c = "GL_INVALID_FRAMEBUFFER_OPERATION";
    		break;
    	case 0x0507:
    		c = "GL_CONTEXT_LOST";
    		break;
    	case 0x8031:
    		c = "GL_TABLE_TOO_LARGE1";
    		break;
    	}
        debug_print("after %s()\n glError (0x%x) %s \n\n", op, error, c);
        printGLString(op, error);
        if (throwError) {
        	throw c;
        }
    }
    return ret;
}

#ifdef NDEBUG
#define glCall(call) call;
#else
#define glCall(call) call; if(checkGlError(#call, true)) {std::cout << "at" << __FILE__ << ":" << __LINE__ << std::endl;};
#endif
