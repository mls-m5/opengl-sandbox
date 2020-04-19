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

        uniform mat4 uMVP;

        void main() {
            gl_Position = uMVP * vPosition;
//            gl_Position.w = 1 + gl_Position.z / 2.;
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

    const unsigned firstCirtartle1 = 1;

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
        mesh.indices.push_back(firstCirtartle1 + i);
        mesh.indices.push_back(firstCirtartle1 + i - 1);
    }

    mesh.indices.push_back(0);
    mesh.indices.push_back(firstCirtartle1);
    mesh.indices.push_back(firstCirtartle1 + numPoints - 1);

    for (unsigned i = 1; i < numPoints; ++i) {
        mesh.indices.push_back(center2);
        mesh.indices.push_back(firstCircle2 + i);
        mesh.indices.push_back(firstCircle2 + i - 1);
    }

    mesh.indices.push_back(center2);
    mesh.indices.push_back(firstCircle2);
    mesh.indices.push_back(firstCircle2 + numPoints - 1);

    // ---- Walls -------

    unsigned wallStart = static_cast<unsigned>(mesh.vertices.size());

    for (unsigned int i = 0; i < numPoints; ++i) {
        auto angle = pi2 / numPoints * i;
        mesh.vertices.emplace_back(sin(angle), cos(angle), 1);
        mesh.vertices.emplace_back(sin(angle), cos(angle), -1);
    }

    for (unsigned i = 1; i < numPoints; ++i) {
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

Mesh createBoxVertices() {
    Mesh mesh;

    auto &vertices = mesh.vertices;
    auto &indices = mesh.indices;

    for (auto i : {1, -1}) {
        const auto start = static_cast<unsigned>(vertices.size());
        vertices.emplace_back(-1, -1, i);
        vertices.emplace_back(1, -1, i);
        vertices.emplace_back(1, 1, i);
        vertices.emplace_back(-1, 1, i);

        indices.insert(indices.begin(), {start, start + 1, start + 2});
        indices.insert(indices.begin(), {start, start + 2, start + 3});
    }

    for (auto i : {1, -1}) {
        const auto start = static_cast<unsigned>(vertices.size());
        vertices.emplace_back(-1, i, -1);
        vertices.emplace_back(1, i, -1);
        vertices.emplace_back(1, i, 1);
        vertices.emplace_back(-1, i, 1);

        indices.insert(indices.begin(), {start, start + 1, start + 2});
        indices.insert(indices.begin(), {start, start + 2, start + 3});
    }

    for (auto i : {1, -1}) {
        const auto start = static_cast<unsigned>(vertices.size());
        vertices.emplace_back(i, -1, -1);
        vertices.emplace_back(i, 1, -1);
        vertices.emplace_back(i, 1, 1);
        vertices.emplace_back(i, -1, 1);

        indices.insert(indices.begin(), {start, start + 1, start + 2});
        indices.insert(indices.begin(), {start, start + 2, start + 3});
    }

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

    GL::VertexBufferObject vboNormals(testPositions, 1, 3);
    GL::VertexBufferObject vboIndices(testIndices);

    vao.unbind();

    // ------------------ Create cylinder ---------------------------

    auto cylMesh = createCylinderVertices();

    GL::VertexArrayObject cylVao;

    program.use();

    GL::VertexBufferObject cylVboPos(
        &cylMesh.vertices.front().x, cylMesh.vertices.size() * 4, 0, 4);
    vboNormals.bind(); // Reuse this
    GL::VertexBufferObject cylIndices(cylMesh.indices);

    cylVao.unbind();

    // ----------------- Cube --------------------------------------

    auto boxMesh = createBoxVertices();

    GL::VertexArrayObject boxVao;

    program.use();

    GL::VertexBufferObject boxPos(
        &boxMesh.vertices.front().x, boxMesh.vertices.size() * 4, 0, 4);

    GL::VertexBufferObject boxIndices(boxMesh.indices);

    boxVao.unbind();

    // ----------------- Transform ---------------------------------

    GLint modelUniform;

    glCall(modelUniform = program.getUniform("uMVP"));
    //    glUniformMatrix4fv(modelUniform, 1, false, modelTransform);

    // -------------------------------------------------------------

    bool running = true;
    double angle = 0;

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
        projectionMatrix.w3 = .5f;

        glEnable(GL_DEPTH_TEST);

        const bool drawCylinder = true;

        if (drawCylinder) {
            auto modelTransform = projectionMatrix * Matrixf::Scale(.2f) *
                                  Matrixf::RotationY(angle) *
                                  Matrixf::RotationZ(angle / 3.);
            cylVao.bind();
            program.use();
            glUniformMatrix4fv(modelUniform, 1, false, modelTransform);
            glCall(glDrawElements(GL_TRIANGLES,
                                  static_cast<int>(cylMesh.indices.size()),
                                  GL_UNSIGNED_INT,
                                  nullptr));
        }

        const bool drawBox = true;

        if (drawBox) {
            auto modelTransform =
                projectionMatrix * Matrixf::Translation(-.4f, 0) *
                Matrixf::Scale(.2f) * Matrixf::RotationY(angle) *
                Matrixf::RotationZ(angle / 3.);

            boxVao.bind();
            program.use();
            glUniformMatrix4fv(modelUniform, 1, false, modelTransform);
            glCall(glDrawElements(GL_TRIANGLES,
                                  static_cast<int>(boxMesh.indices.size()),
                                  GL_UNSIGNED_INT,
                                  nullptr));
        }

        window.swap();
    }

    return 0;
}
