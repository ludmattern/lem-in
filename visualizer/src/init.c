//==============================================
//                                            //
//          42 lem-in visualizer              //
//                 init.c                     //
//==============================================

#include "visualizer.h"

void init_map(void)
{
    g_map.room_count = 0;
    g_map.connection_count = 0;
    g_map.ant_count = 0;
}

int init_sdl(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur SDL_Init: %s\n", SDL_GetError());
        return -1;
    }
    
    if (TTF_Init() < 0) {
        printf("Erreur TTF_Init: %s\n", TTF_GetError());
        return -1;
    }    
    window = SDL_CreateWindow("Lem-in Visualizer", 
                             SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             1200, 800, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Erreur SDL_CreateWindow: %s\n", SDL_GetError());
        return -1;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erreur SDL_CreateRenderer: %s\n", SDL_GetError());
        return -1;
    }
    
    return 0;
}
