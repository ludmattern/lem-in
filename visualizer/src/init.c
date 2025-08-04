#include "visualizer.h"

void init_map(void)
{
	g_map.room_count = 0;
	g_map.connection_count = 0;
	g_map.ant_count = 0;
}

int init_sdl(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		ft_eprintf("Error: SDL_Init: %s\n", SDL_GetError());
		return -1;
	}

	if (TTF_Init() < 0)
	{
		ft_eprintf("Error: TTF_Init: %s\n", TTF_GetError());
		return -1;
	}

	screen = SDL_SetVideoMode(window_width, window_height, 32, SDL_SWSURFACE);
	if (!screen)
	{
		ft_eprintf("Error: SDL_SetVideoMode: %s\n", SDL_GetError());
		return -1;
	}

	SDL_WM_SetCaption("Lem-in Visualizer", NULL);

	return 0;
}
