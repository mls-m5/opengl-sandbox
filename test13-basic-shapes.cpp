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

struct Vertex {
    vec4 pos;
    vec4 normal;

    Vertex() = default;
    Vertex(const Vertex &) = default;
    Vertex &operator=(const Vertex &) = default;

    Vertex(decltype(pos) pos, decltype(normal) normal = {})
        : pos(pos), normal(normal) {
    }
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
};

const std::string vertex =
    R"_(
        #version 330

        layout (location = 0) in vec4 vPosition;
        layout (location = 1) in vec3 vNormal;

        out vec3 fNormal;

        uniform mat4 uMVP;
        uniform mat4 uMV;

        void main() {
            gl_Position = uMVP * vPosition;
            fNormal = normalize(mat3(uMV) * vNormal);
        }
)_";

const std::string fragment =
    R"_(
        #version 330

        in vec3 fNormal;

        out vec4 fragColor;

        void main() {
            float intensity = .5 + fNormal.y / 2;
            fragColor = vec4(intensity, intensity, intensity, 1);
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

    auto &vertices = mesh.vertices;
    auto &indices = mesh.indices;

    for (float z : {1, -1}) {
        const unsigned circleStart = static_cast<unsigned>(vertices.size()) + 1;

        auto n = vec4{0, 0, z};
        vertices.push_back({{0, 0, z}, n});

        for (size_t i = 0; i < numPoints; ++i) {
            auto angle = pi2f / numPoints * i;
            vertices.push_back({{sinf(angle), cosf(angle), z}, n});
        }

        for (unsigned i = 1; i < numPoints; ++i) {
            indices.push_back(circleStart);
            indices.push_back(circleStart + i);
            indices.push_back(circleStart + i - 1);
        }

        indices.push_back(0);
        indices.push_back(circleStart);
        indices.push_back(circleStart + numPoints - 1);
    }

    // ---- Walls -------

    unsigned wallStart = static_cast<unsigned>(vertices.size());

    for (unsigned int i = 0; i < numPoints; ++i) {
        auto angle = pi2f / numPoints * i;
        auto s = sinf(angle);
        auto c = cosf(angle);
        auto n = vec4{s, c};
        vertices.push_back({{s, c, 1}, n});
        vertices.push_back({{s, c, -1}, n});
    }

    for (unsigned i = 1; i < numPoints; ++i) {
        indices.push_back(wallStart + i * 2);
        indices.push_back(wallStart + i * 2 + 1);
        indices.push_back(wallStart + (i - 1) * 2 + 1);

        indices.push_back(wallStart + i * 2);
        indices.push_back(wallStart + (i - 1) * 2);
        indices.push_back(wallStart + (i - 1) * 2 + 1);
    }

    indices.push_back(wallStart);
    indices.push_back(wallStart + 1);
    indices.push_back(wallStart + (numPoints - 1) * 2 + 1);

    indices.push_back(wallStart);
    indices.push_back(wallStart + (numPoints - 1) * 2);
    indices.push_back(wallStart + (numPoints - 1) * 2 + 1);

    return mesh;
}

Mesh createBoxVertices() {
    Mesh mesh;

    auto &vertices = mesh.vertices;
    auto &indices = mesh.indices;

    for (float i : {1, -1}) {
        const auto start = static_cast<unsigned>(vertices.size());
        vec4 n = {0, 0, i};
        vertices.emplace_back(vec4{-1, -1, i}, n);
        vertices.emplace_back(vec4{1, -1, i}, n);
        vertices.emplace_back(vec4{1, 1, i}, n);
        vertices.emplace_back(vec4{-1, 1, i}, n);

        indices.insert(indices.begin(), {start, start + 1, start + 2});
        indices.insert(indices.begin(), {start, start + 2, start + 3});
    }

    for (float i : {1, -1}) {
        const auto start = static_cast<unsigned>(vertices.size());
        vec4 n = {0, i, 0};
        vertices.emplace_back(vec4{-1, i, -1}, n);
        vertices.emplace_back(vec4{1, i, -1}, n);
        vertices.emplace_back(vec4{1, i, 1}, n);
        vertices.emplace_back(vec4{-1, i, 1}, n);

        indices.insert(indices.begin(), {start, start + 1, start + 2});
        indices.insert(indices.begin(), {start, start + 2, start + 3});
    }

    for (float i : {1, -1}) {
        const auto start = static_cast<unsigned>(vertices.size());
        vec4 n = {i, 0, 0};
        vertices.emplace_back(vec4{i, -1, -1}, n);
        vertices.emplace_back(vec4{i, 1, -1}, n);
        vertices.emplace_back(vec4{i, 1, 1}, n);
        vertices.emplace_back(vec4{i, -1, 1}, n);

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

    // ------------------ Create cylinder
    // ---------------------------

    auto cylMesh = createCylinderVertices();

    GL::VertexArrayObject cylVao;

    program.use();

    GL::VertexBufferObject cylVboPos(&cylMesh.vertices.front().pos.x,
                                     cylMesh.vertices.size() * 4 * 2,
                                     0,
                                     4,
                                     8 * sizeof(float));

    GL::VertexBufferObject cylVboNormals(&cylMesh.vertices.front().normal.x,
                                         cylMesh.vertices.size() * 4 * 2,
                                         1,
                                         4,
                                         8 * sizeof(float));

    GL::VertexBufferObject cylIndices(cylMesh.indices);

    cylVao.unbind();

    // ----------------- Cube
    // --------------------------------------

    auto boxMesh = createBoxVertices();

    GL::VertexArrayObject boxVao;

    program.use();

    GL::VertexBufferObject boxPos(&boxMesh.vertices.front().pos.x,
                                  boxMesh.vertices.size() * 4 * 2,
                                  0,
                                  4,
                                  8 * sizeof(float));

    GL::VertexBufferObject boxNormals(&boxMesh.vertices.front().normal.x,
                                      boxMesh.vertices.size() * 4 * 2,
                                      1,
                                      4,
                                      8 * sizeof(float));

    GL::VertexBufferObject boxIndices(boxMesh.indices);

    boxVao.unbind();

    // ----------------- Transform
    // ---------------------------------

    GLint MVPuniform, MVuniform;

    glCall(MVPuniform = program.getUniform("uMVP"));
    glCall(MVuniform = program.getUniform("uMV"));

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
            auto mvTransform = projectionMatrix * Matrixf::Scale(.2f) *
                               Matrixf::RotationY(angle) *
                               Matrixf::RotationZ(angle / 3.);
            auto mvpTransform = projectionMatrix * mvTransform;
            cylVao.bind();
            program.use();

            glUniformMatrix4fv(MVPuniform, 1, false, mvpTransform);
            glUniformMatrix4fv(MVuniform, 1, false, mvTransform);
            glCall(glDrawElements(GL_TRIANGLES,
                                  static_cast<int>(cylMesh.indices.size()),
                                  GL_UNSIGNED_INT,
                                  nullptr));
        }

        const bool drawBox = true;

        if (drawBox) {
            auto mvTransform = projectionMatrix *
                               Matrixf::Translation(-.4f, 0) *
                               Matrixf::Scale(.2f) * Matrixf::RotationY(angle) *
                               Matrixf::RotationZ(angle / 3.);
            auto mvpTransform = projectionMatrix * mvTransform;

            boxVao.bind();
            program.use();
            glUniformMatrix4fv(MVPuniform, 1, false, mvpTransform);
            glUniformMatrix4fv(MVuniform, 1, false, mvTransform);
            glCall(glDrawElements(GL_TRIANGLES,
                                  static_cast<int>(boxMesh.indices.size()),
                                  GL_UNSIGNED_INT,
                                  nullptr));
        }

        window.swap();
    }

    return 0;
}
