// Copyright © Mattias Larsson Sköld 2020

//! This test is for creating vertex array objects containing boxes and
//! cylinders

#include "matgui/matgl.h"
#include "matrix.h"
#include "matsdl.h"

#include <string>

namespace {

struct vec4 {
    float x = 0, y = 0, z = 0, w = 1;

    vec4(float x = 0, float y = 0, float z = 0, float w = 1)
        : x(x), y(y), z(z), w(w) {
    }
};

struct Mesh {
    std::vector<vec4> vertices;
    std::vector<unsigned> indices;
};

struct vec3 {
    float x = 0, y = 0, z = 0;
};

const std::string vertex =
    R"_(
        #version 330

        layout (location = 0) in vec4 vPosition;
        layout (location = 1) in vec3 vNormal;

        out vec3 fNormal;

        uniform mat4 uModel;

        void main() {
            gl_Position = uModel * vPosition;
            gl_Position.w = 1 + gl_Position.z / 2.;
            fNormal = vPosition.xyz; // Test
        }
)_";

const std::string fragment =
    R"_(
        #version 330

        in vec3 fNormal;

        out vec4 fragColor;

        void main() {
//            fragColor = vec4(1, 1, 1, 1);
//            if (fNormal.z > 1) {
                fragColor = vec4(1,.5 + fNormal.z / 2,1,1);
//            }
//            else {
//                fragColor = vec4(1,1,.5,1);
//            }
        }

        )_";

const std::vector<vec4> vTestPositions = {
    {.0f, .0f},
    {.1f, .0f},
    {.1f, .1f},
    {.0f, .1f},
};

const std::vector<unsigned> testIndices = {
    0,
    1,
    2,

    0,
    2,
    3,
};

// clang-format off
const std::vector<float> testPositions = {
    0,     0, 0, 1,
    .1f,   0, 0, 1,
    .1f, .1f, 0, 1,
    0,   .1f, 0, 1,
};
// clang-format on

Mesh createCylinderVertices() {
    const unsigned numPoints = 40;

    Mesh mesh;

    mesh.vertices.emplace_back(0, 0, 1);

    const unsigned firstCircle1 = 1;

    for (size_t i = 0; i < numPoints; ++i) {
        auto angle = pi2 / numPoints * i;
        mesh.vertices.emplace_back(sin(angle), cos(angle), 1);
    }

    mesh.vertices.emplace_back(0, 0, -1);
    const unsigned center2 = static_cast<unsigned>(mesh.vertices.size() - 1);
    const unsigned firstCircle2 = center2 + 1;

    for (size_t i = 0; i < numPoints; ++i) {
        auto angle = pi2 / numPoints * i;
        mesh.vertices.emplace_back(sin(angle), cos(angle), -1);
    }

    for (unsigned i = 1; i < numPoints; ++i) {
        mesh.indices.push_back(0);
        mesh.indices.push_back(firstCircle1 + i);
        mesh.indices.push_back(firstCircle1 + i - 1);
    }

    mesh.indices.push_back(0);
    mesh.indices.push_back(firstCircle1);
    mesh.indices.push_back(firstCircle1 + numPoints - 1);

    for (unsigned i = 1; i < numPoints; ++i) {
        mesh.indices.push_back(center2);
        mesh.indices.push_back(firstCircle2 + i);
        mesh.indices.push_back(firstCircle2 + i - 1);
    }

    mesh.indices.push_back(center2);
    mesh.indices.push_back(firstCircle2);
    mesh.indices.push_back(firstCircle2 + numPoints - 1);

    // ---- Walls -------

    unsigned wallStart = mesh.vertices.size();

    for (int i = 0; i < numPoints; ++i) {
        auto angle = pi2 / numPoints * i;
        mesh.vertices.emplace_back(sin(angle), cos(angle), 1);
        mesh.vertices.emplace_back(sin(angle), cos(angle), -1);
    }

    for (int i = 1; i < numPoints; ++i) {
        mesh.indices.push_back(wallStart + i * 2);
        mesh.indices.push_back(wallStart + i * 2 + 1);
        mesh.indices.push_back(wallStart + (i - 1) * 2 + 1);

        mesh.indices.push_back(wallStart + i * 2);
        mesh.indices.push_back(wallStart + (i - 1) * 2);
        mesh.indices.push_back(wallStart + (i - 1) * 2 + 1);
    }

    mesh.indices.push_back(wallStart);
    mesh.indices.push_back(wallStart + 1);
    mesh.indices.push_back(wallStart + (numPoints - 1) * 2 + 1);

    mesh.indices.push_back(wallStart);
    mesh.indices.push_back(wallStart + (numPoints - 1) * 2);
    mesh.indices.push_back(wallStart + (numPoints - 1) * 2 + 1);

    return mesh;
}

} // namespace

int main(int /*argc*/, char ** /*argv*/) {
    const int width = 800;
    const int height = 600;

    SDL::Window window("test13 basic shapes",
                       SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED,
                       width,
                       height,
                       SDL_WINDOW_OPENGL);

    SDL::GLContext context(window);

    ShaderProgram program(vertex, fragment);

    GL::VertexArrayObject vao;
    program.use();

    GL::VertexBufferObject vboPos(
        &vTestPositions.front().x, vTestPositions.size() * 4, 0, 4);
    //    GL::VertexBufferObject vboPos(testPositions, 0, 4);

    GL::VertexBufferObject vboNormals(testPositions, 1, 3);
    GL::VertexBufferObject vboIndices(testIndices);

    vao.unbind();

    // ------------------ Create cylinder ---------------------------

    auto face = createCylinderVertices();

    GL::VertexArrayObject cylVao;

    program.use();

    GL::VertexBufferObject cylVboPos(
        &face.vertices.front().x, face.vertices.size() * 4, 0, 4);
    vboNormals.bind(); // Reuse this
    GL::VertexBufferObject cylIndices(face.indices);

    cylVao.unbind();

    auto modelTransform = Matrixf::Scale(.5, .5);

    GLint modelUniform;

    glCall(modelUniform = program.getUniform("uModel"));
    glUniformMatrix4fv(modelUniform, 1, false, modelTransform);

    // -------------------------------------------------------------

    bool running = true;
    float angle = 0;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        glClearColor(.1f, .3f, 0, 0);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        if (0) {
            vao.bind();
            program.use();
            glCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
        }

        angle += .01;

        auto projectionMatrix = Matrixf::Identity();
        modelTransform.w3 = .5f;
        modelTransform =
            projectionMatrix * Matrixf::Scale(.5) * Matrixf::RotationY(angle);
        glUniformMatrix4fv(modelUniform, 1, false, modelTransform);

        glEnable(GL_DEPTH_TEST);

        cylVao.bind();
        program.use();
        glCall(glDrawElements(
            GL_TRIANGLES, face.indices.size(), GL_UNSIGNED_INT, nullptr));

        window.swap();
    }

    return 0;
}
