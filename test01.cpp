#if 0

g++ $0 -std=c++11 -g -o test1 -lSDL2 -lGL

exit
#endif

#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <assert.h>
#include <memory>
#include <stdint.h>
#include <stdio.h>

using namespace std;

int main(int /*argv*/, char ** /*args*/) {
    auto width = 1000;
    auto height = 1000;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    auto windowFlags = SDL_WINDOW_OPENGL;
    auto window = shared_ptr<SDL_Window>(
        SDL_CreateWindow("Opengl playground - test1 - empty window",
                         0,
                         0,
                         width,
                         height,
                         windowFlags),
        SDL_DestroyWindow);

    assert(window);
    auto context = shared_ptr<void>(SDL_GL_CreateContext(window.get()),
                                    SDL_GL_DeleteContext);

    bool running = 1;
    bool fullScreen = 0;
    while (running) {
        SDL_Event Event;
        while (SDL_PollEvent(&Event)) {
            if (Event.type == SDL_KEYDOWN) {
                switch (Event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    running = 0;
                    break;
                case 'f':
                    fullScreen = !fullScreen;
                    if (fullScreen) {
                        SDL_SetWindowFullscreen(
                            window.get(),
                            windowFlags | SDL_WINDOW_FULLSCREEN_DESKTOP);
                    }
                    else {
                        SDL_SetWindowFullscreen(window.get(), windowFlags);
                    }
                    break;
                default:
                    break;
                }
            }
            else if (Event.type == SDL_QUIT) {
                running = 0;
            }
        }

        glViewport(0, 0, width, height);
        glClearColor(0.f, 1.f, 1.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        SDL_GL_SwapWindow(window.get());
    }
    return 0;
}
