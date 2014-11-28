// tutorial02.c
// Based on https://github.com/chelyaev/ffmpeg-tutorial/blob/master/tutorial02.c

#include <SDL.h>

int main(int argc, char *argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }
    return 0;
}
