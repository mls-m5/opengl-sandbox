#if 0

g++ $0 shaderprogram.cpp -std=c++11 -g -o test6-intel -lSDL2 -lGL  -fmax-errors=5 -Imatengine/ -O0

exit
#endif

/*
 * test6-intel.cpp
 *
 * This test was to find a problem that appeared in intel cards but not nvidia
 * cards
 *
 *  Created on: 25 feb. 2019
 *      Author: Mattias Larsson Sköld
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

#include "matgui/shaderprogram.h"
#include "matrix.h"
#include <iostream>

#include <iostream>
#include <vector>

using namespace std;

// Width & Height of window
#define width 800                        // 640
#define height 600                       // 480
#define framerate 50                     // Hz min egen variable
#define frametime = 1 / framerate + 1000 // Hur länge varje bild ska synas

namespace {
SDL_Window *window = 0;
SDL_GLContext context;

static GLuint shaderVecPointer;
GLint shaderColorPointer;
GLuint transformMatrixPointer;
GLuint cameraMatrixPointer;
static GLfloat transformMatrix[16];
// static GLfloat cameraMatrix[16];
static Vec camPos;
static double camPerspective;
ShaderProgram *shaderProgram;

GLuint vertexArray = 0;

static std::vector<GLfloat> smokeVertexData;
static std::vector<GLfloat> smokeColorData;

struct vertexDataStruct {
    GLfloat x, y;
};

struct colorDataStruct {
    GLfloat r, g, b, a;
};

static std::vector<vertexDataStruct> cometVertexData;
static std::vector<colorDataStruct> cometColorData;

// Square
// static const GLfloat gCometVertices[] = { -1.f, -1.f, 1.f, -1.f, 1.f, 1.f,
// -1.f, 1.f }; static const GLfloat gCometColors[] = { 		.8,
// .8, 1., .8, 		.8,
//.8, 1., .8, 		.8, .8, 1., .8, 		.8, .8, 1., .8,
//};

static const char gVertexShader[] =
    R"apa(
#version 330 core
in vec4 vPosition;
in vec4 vColor;
uniform	 mat4	 mvp_matrix;	 // model-view-projection matrix
uniform	 mat4	 proj_matrix;	 // camera matrix
out vec4 fColor;
void main() {
  gl_Position = proj_matrix * mvp_matrix * vPosition;
  float perspective = (gl_Position.z + gl_Position.y / 5.);
  gl_Position.x /= perspective;
  gl_Position.y /= perspective;
  fColor = vColor;
})apa";

static const char gFragmentShader[] =
    R"apa(
#version 330 core
in vec4 fColor;
void main() {
  if (gl_FragCoord.z < 0.1) {
    discard;
  }
  gl_FragColor = fColor;
}
)apa";

} // namespace

namespace test {
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

// clang-format off
// An array of 3 vectors which represents 3 vertices
static const GLfloat g_vertex_buffer_data[] = {
   -1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
   0.0f,  1.0f, 0.0f,
};
// clang-format on

static GLuint indices[] = {0, 1, 2};

static GLuint vertexbuffer;
static GLuint elementBuffer;
static ShaderProgram program;
static GLuint vertexArray;

void init() {
    // This is nessesary if you want to use openGL 3
    glCall(glGenVertexArrays(1, &vertexArray));
    glCall(glBindVertexArray(vertexArray));

    // This will identify our vertex buffer
    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glCall(glGenBuffers(1, &vertexbuffer));
    // The following commands will talk about our 'vertexbuffer' buffer
    glCall(glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer));
    // Give our vertices to OpenGL.
    glCall(glBufferData(GL_ARRAY_BUFFER,
                        sizeof(g_vertex_buffer_data),
                        g_vertex_buffer_data,
                        GL_STATIC_DRAW));

    // Same thing with indices
    glCall(glGenBuffers(1, &elementBuffer));
    glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer));
    glCall(glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

    glCall(glEnableVertexAttribArray(0));

    glCall(glVertexAttribPointer(0, // attribute 0. No particular reason for 0,
                                    // but must match the layout in the shader.
                                 3, // size
                                 GL_FLOAT, // type
                                 GL_FALSE, // normalized?
                                 0,        // stride
                                 (void *)0 // array buffer offset
                                 ));

    glCall(glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer));
    glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer));

    program.initProgram(vertexShader, fragmentShader);
}

// Rendering function
void render() {
    static float x = 0;
    x += 20;

    program.use();

    // 1st attribute buffer : vertices
    //	glCall(glEnableVertexAttribArray(vertexArray));
    glCall(glBindVertexArray(vertexArray));

    // Draw the triangle !
    glCall(glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0));
    //	glCall(glDisableVertexAttribArray(0));

    program.unuse();
}

} // namespace test

bool initDrawModule(double perspective) {

    glCall(glGenVertexArrays(1, &vertexArray));
    glCall(glBindVertexArray(vertexArray));

    shaderProgram = new ShaderProgram(gVertexShader, gFragmentShader);

    if (!shaderProgram->getProgram()) {
        cerr << ("Could not create program.");
        return false;
    }

    glCall(glUseProgram(shaderProgram->getProgram()));

    shaderVecPointer = shaderProgram->getAttribute("vPosition");
    shaderColorPointer = shaderProgram->getAttribute("vColor");
    transformMatrixPointer = shaderProgram->getUniform("mvp_matrix");
    cameraMatrixPointer = shaderProgram->getUniform("proj_matrix");

    smokeVertexData.reserve(100000);
    smokeColorData.reserve(200000);
    camPerspective = perspective;
    return false;
}

inline void identityMatrix(GLfloat *matrix) {
    for (int i = 0; i < 16; ++i) {
        matrix[i] = 0;
    }
    for (int i = 0; i < 4; ++i) {
        matrix[i + i * 4] = 1;
    }
}

class SineClass {
public:
    static const int tableLength = 1024;
    SineClass() {
        table = new double[tableLength + 1];
        for (int i = 0; i < tableLength; ++i) {
            table[i] = sin((double)i / tableLength * pi2);
        }
    }

    ~SineClass() {
        delete table;
    }

    inline double operator()(double a) {
        a /= pi2;
        a -= floor(a);

        return table[(int)(tableLength * a)];
    }

    double cos(double a) {
        return (*this)(a + pi / 2);
    }

    double *table;

} Sine;

void modelTransform(Vec p, double a, double scale) {
    identityMatrix(transformMatrix);
    auto s = Sine(a);
    auto c = Sine.cos(a);

    transformMatrix[0] = c * scale;
    transformMatrix[1] = s * scale;
    transformMatrix[4] = -s * scale;
    transformMatrix[5] = c * scale;

    transformMatrix[12] = p.x;
    transformMatrix[13] = p.y;
    transformMatrix[14] = p.z;

    glCall(glUniformMatrix4fv(
        transformMatrixPointer, 1, GL_FALSE, transformMatrix));
}

void resetTransform() {
    identityMatrix(transformMatrix);
    glCall(glUniformMatrix4fv(
        transformMatrixPointer, 1, GL_FALSE, transformMatrix));
}

void drawComet(Vec p, double a, double r) {
    auto dx = p.x - camPos.x;
    auto dy = p.y - camPos.y;

    if (dx * dx + dy * dy > 1500) {
        return;
    }

    glCall(glBindVertexArray(vertexArray));

    shaderProgram->use();

    // This crashes
    modelTransform(p, a / 180., r);
    //#warning "this is the problem, you need to create a vertex buffer to
    // store" #warning "the data in first and then use that when bound to the
    // vertexArray" #warning "when calling glVertexAttribPointer"
    //    glCall(glEnableVertexAttribArray(shaderVecPointer));
    //    glCall(glVertexAttribPointer(shaderVecPointer, 2, GL_FLOAT, GL_FALSE,
    //    0, gCometVertices));

    //    glCall(glEnableVertexAttribArray(shaderColorPointer));
    //    glCall(glVertexAttribPointer(shaderColorPointer, 4, GL_FLOAT,
    //    GL_FALSE, 0, gCometColors));
    //
    //    glCall(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
    //
    //    resetTransform();
}

int main(int argc, char **argv) {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        cerr << "failed to init video";
        return -1;
    }
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    window = SDL_CreateWindow("test6",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              width,
                              height,
                              SDL_WINDOW_OPENGL);
    if (!window) {
        cerr << "could not create window" << endl;
        return 1;
    }

    context = SDL_GL_CreateContext(window);
    if (!context) {
        throw runtime_error("could not create context");
        return 1;
    }
    glCall(cout << "test" << endl);

    SDL_GL_SetSwapInterval(1);

    //	initDrawModule(width / height);

    test::init();

    //    hant::init();

    //    mainLoop();
    while (true) {

        // ttime = SDL_GetTicks() + frametime;
        //    	long ttime =  15 + SDL_GetTicks();
        //        processEvents();

        // Process SDL events
        //    	void processEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                return 0;
            }
        }
        //    	}

        //        game::Update(.1);
        //    	glClearColor(0,0,0,1);

        glClear(GL_DEPTH_BUFFER_BIT |
                GL_COLOR_BUFFER_BIT); // Clear color and depth buffer

        //        game::Render();
        //        drawComet({0,0}, 1, .4);
        test::render();

        SDL_GL_SwapWindow(window); // Update screen

        //        processEvents();

        SDL_Delay(200);

        //        if (SDL_GetTicks() < ttime) {
        //        	SDL_Delay(ttime - SDL_GetTicks());
        //        }
    }

    //    hant::avsl();

    return 0;
}
