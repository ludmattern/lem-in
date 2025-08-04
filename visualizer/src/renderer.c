#include "visualizer.h"

void get_map_bounds(int *min_x, int *max_x, int *min_y, int *max_y)
{
	if (g_map.room_count == 0)
	{
		*min_x = *max_x = *min_y = *max_y = 0;
		return;
	}

	*min_x = *max_x = g_map.rooms[0].x;
	*min_y = *max_y = g_map.rooms[0].y;

	for (int i = 1; i < g_map.room_count; i++)
	{
		if (g_map.rooms[i].x < *min_x)
			*min_x = g_map.rooms[i].x;
		if (g_map.rooms[i].x > *max_x)
			*max_x = g_map.rooms[i].x;
		if (g_map.rooms[i].y < *min_y)
			*min_y = g_map.rooms[i].y;
		if (g_map.rooms[i].y > *max_y)
			*max_y = g_map.rooms[i].y;
	}
}

void calculate_scaling(void)
{
	int min_x, max_x, min_y, max_y;
	get_map_bounds(&min_x, &max_x, &min_y, &max_y);

	// Calculer les dimensions de la carte
	int map_width = max_x - min_x;
	int map_height = max_y - min_y;

	// Ajouter une marge pour éviter que les éléments touchent les bords
	int margin = 100;
	int available_width = window_width - 2 * margin;
	int available_height = window_height - 2 * margin - 50; // 50px pour le texte en haut

	// Éviter la division par zéro
	if (map_width == 0)
		map_width = 1;
	if (map_height == 0)
		map_height = 1;

	// Calculer le facteur d'échelle pour s'adapter à la fenêtre
	float scale_x = (float)available_width / map_width;
	float scale_y = (float)available_height / map_height;
	scale_factor = (scale_x < scale_y) ? scale_x : scale_y;

	// Limiter l'échelle pour éviter des éléments trop petits ou trop grands
	if (scale_factor < 5.0)
		scale_factor = 5.0;
	if (scale_factor > 50.0)
		scale_factor = 50.0;

	// Calculer les offsets pour centrer la carte
	int scaled_width = (int)(map_width * scale_factor);
	int scaled_height = (int)(map_height * scale_factor);
	offset_x = (window_width - scaled_width) / 2 - (int)(min_x * scale_factor);
	offset_y = (window_height - scaled_height) / 2 - (int)(min_y * scale_factor) + 25; // 25px pour le texte

	ft_printf("Map bounds: (%d,%d) to (%d,%d)\n", min_x, min_y, max_x, max_y);
	ft_printf("Scale factor: %.2f, Offset: (%d,%d)\n", scale_factor, offset_x, offset_y);
}

void draw_rooms(void)
{
	// Clear screen with brown color
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 139, 69, 19));

	// Draw grass band at top
	SDL_Rect grass_band = {0, 0, window_width, 20};
	SDL_FillRect(screen, &grass_band, SDL_MapRGB(screen->format, 34, 139, 34));

	// Draw each room
	for (int i = 0; i < g_map.room_count; i++)
	{
		Room *room = &g_map.rooms[i];

		int screen_x = (int)(room->x * scale_factor) + offset_x;
		int screen_y = (int)(room->y * scale_factor) + offset_y;

		Uint32 room_color = SDL_MapRGB(screen->format, 80, 40, 8); // Couleur normale pour toutes les salles

		int ellipse_width = (int)(32 * scale_factor / 20.0); // Adapter la taille à l'échelle
		int ellipse_height = (int)(20 * scale_factor / 20.0);

		// Taille minimum pour la visibilité
		if (ellipse_width < 8)
			ellipse_width = 8;
		if (ellipse_height < 6)
			ellipse_height = 6;

		// Dessiner la salle avec la couleur normale
		for (int dx = -ellipse_width / 2; dx <= ellipse_width / 2; dx++)
		{
			for (int dy = -ellipse_height / 2; dy <= ellipse_height / 2; dy++)
			{
				float normalized_x = (float)dx / (ellipse_width / 2);
				float normalized_y = (float)dy / (ellipse_height / 2);
				if (normalized_x * normalized_x + normalized_y * normalized_y <= 1.0)
				{
					SDL_Rect pixel = {screen_x + dx, screen_y + dy, 1, 1};
					SDL_FillRect(screen, &pixel, room_color);
				}
			}
		}

		// Ajouter un contour coloré pour start et end
		if (room->is_start || room->is_end)
		{
			Uint32 border_color;
			if (room->is_start)
			{
				border_color = SDL_MapRGB(screen->format, 0, 255, 0); // Vert pour start
			}
			else
			{
				border_color = SDL_MapRGB(screen->format, 255, 0, 0); // Rouge pour end
			}

			// Dessiner le contour (ligne plus épaisse autour de l'ellipse)
			int border_thickness = (int)(2 * scale_factor / 20.0);
			if (border_thickness < 1)
				border_thickness = 1;

			for (int dx = -ellipse_width / 2 - border_thickness; dx <= ellipse_width / 2 + border_thickness; dx++)
			{
				for (int dy = -ellipse_height / 2 - border_thickness; dy <= ellipse_height / 2 + border_thickness; dy++)
				{
					float normalized_x = (float)dx / (ellipse_width / 2);
					float normalized_y = (float)dy / (ellipse_height / 2);
					float distance = normalized_x * normalized_x + normalized_y * normalized_y;

					// Dessiner le contour si on est dans la zone de contour (entre 1.0 et 1.4)
					if (distance > 1.0 && distance <= 1.4)
					{
						SDL_Rect pixel = {screen_x + dx, screen_y + dy, 1, 1};
						SDL_FillRect(screen, &pixel, border_color);
					}
				}
			}
		}

		// Display the room name with SDL1 TTF (seulement si suffisamment grand)
		if (scale_factor > 10.0)
		{
			int font_size = (int)(12 * scale_factor / 20.0);
			if (font_size < 8)
				font_size = 8;
			if (font_size > 24)
				font_size = 24;

			font = TTF_OpenFont("assets/DejaVuSans.ttf", font_size);
			if (font)
			{
				SDL_Color text_color = {255, 255, 255, 255};
				SDL_Surface *text_surface = TTF_RenderText_Solid(font, room->name, text_color);
				if (text_surface)
				{
					SDL_Rect text_rect = {
						screen_x - text_surface->w / 2,
						screen_y - ellipse_height / 2 - text_surface->h - 2,
						text_surface->w,
						text_surface->h};
					SDL_BlitSurface(text_surface, NULL, screen, &text_rect);
					SDL_FreeSurface(text_surface);
				}
				TTF_CloseFont(font);
			}
		}
	}
}

void draw_connections(void)
{
	Uint32 line_color = SDL_MapRGB(screen->format, 80, 40, 8);

	// Draw each connection
	for (int i = 0; i < g_map.connection_count; i++)
	{
		Connection *conn = &g_map.connections[i];

		Room *from_room = &g_map.rooms[conn->from_room];
		Room *to_room = &g_map.rooms[conn->to_room];

		int x1 = (int)(from_room->x * scale_factor) + offset_x;
		int y1 = (int)(from_room->y * scale_factor) + offset_y;
		int x2 = (int)(to_room->x * scale_factor) + offset_x;
		int y2 = (int)(to_room->y * scale_factor) + offset_y;

		// Algorithme de Bresenham amélioré pour dessiner des lignes épaisses
		int dx = abs(x2 - x1);
		int dy = abs(y2 - y1);
		int sx = (x1 < x2) ? 1 : -1;
		int sy = (y1 < y2) ? 1 : -1;

		// Épaisseur adaptée à l'échelle
		int line_thickness = (int)(scale_factor / 10.0);
		if (line_thickness < 1)
			line_thickness = 1;
		if (line_thickness > 5)
			line_thickness = 5;

		// Dessiner une ligne épaisse
		for (int thickness = -line_thickness; thickness <= line_thickness; thickness++)
		{
			int x_temp = x1;
			int y_temp = y1;
			int err_temp = dx - dy;

			while (x_temp != x2 || y_temp != y2)
			{
				// Dessiner un pixel à la position actuelle avec l'épaisseur
				for (int offset_x = -1; offset_x <= 1; offset_x++)
				{
					for (int offset_y = -1; offset_y <= 1; offset_y++)
					{
						SDL_Rect pixel = {x_temp + offset_x + thickness, y_temp + offset_y, 1, 1};
						SDL_FillRect(screen, &pixel, line_color);
					}
				}

				int e2 = 2 * err_temp;
				if (e2 > -dy)
				{
					err_temp -= dy;
					x_temp += sx;
				}
				if (e2 < dx)
				{
					err_temp += dx;
					y_temp += sy;
				}
			}

			// Dessiner le dernier pixel
			for (int offset_x = -1; offset_x <= 1; offset_x++)
			{
				for (int offset_y = -1; offset_y <= 1; offset_y++)
				{
					SDL_Rect pixel = {x_temp + offset_x + thickness, y_temp + offset_y, 1, 1};
					SDL_FillRect(screen, &pixel, line_color);
				}
			}
		}
	}
}

void draw_ants(void)
{
	// Draw each ant
	for (int i = 0; i < g_map.ant_count; i++)
	{
		Ant *ant = &g_map.ants[i];
		if (ant->ant_id == 0)
			continue;

		Room *current_room = &g_map.rooms[ant->current_room];
		int screen_x, screen_y;

		if (ant->is_moving && ant->progress < 1.0)
		{
			Room *target_room = &g_map.rooms[ant->target_room];
			int x1 = (int)(current_room->x * scale_factor) + offset_x;
			int y1 = (int)(current_room->y * scale_factor) + offset_y;
			int x2 = (int)(target_room->x * scale_factor) + offset_x;
			int y2 = (int)(target_room->y * scale_factor) + offset_y;

			screen_x = x1 + (int)((x2 - x1) * ant->progress);
			screen_y = y1 + (int)((y2 - y1) * ant->progress);
		}
		else
		{
			screen_x = (int)(current_room->x * scale_factor) + offset_x;
			screen_y = (int)(current_room->y * scale_factor) + offset_y;
		}

		// Taille des fourmis adaptée à l'échelle
		int ant_size = (int)(3 * scale_factor / 20.0);
		if (ant_size < 2)
			ant_size = 2;
		if (ant_size > 8)
			ant_size = 8;

		Uint32 ant_color = SDL_MapRGB(screen->format, 0, 0, 0);
		for (int dx = -ant_size; dx <= ant_size; dx++)
		{
			for (int dy = -ant_size; dy <= ant_size; dy++)
			{
				if (dx * dx + dy * dy <= ant_size * ant_size)
				{
					SDL_Rect pixel = {screen_x + dx, screen_y + dy, 1, 1};
					SDL_FillRect(screen, &pixel, ant_color);
				}
			}
		}
	}
}

int display_map(void)
{
	if (get_map_info() == -1)
	{
		ft_printf("Error: Failed to get map info\n");
		return (-1);
	}
	if (init_sdl() != 0)
	{
		ft_printf("Error: Failed to initialize SDL\n");
		return -1;
	}

	// Calculer la mise à l'échelle automatique
	calculate_scaling();

	reset_ants_to_start();

	current_turn = 1;

	SDL_Event event;
	int quit = 0;
	int animation_speed = 50;
	int auto_play = 0;			// Mode lecture automatique
	int animation_finished = 0; // Indicateur de fin d'animation
	Uint32 last_time = SDL_GetTicks();
	Uint32 last_auto_advance = SDL_GetTicks();

	ft_printf("Visualizer controls:\n");
	ft_printf("  SPACE - Next turn\n");
	ft_printf("  R     - Reset animation\n");
	ft_printf("  A     - Auto-play mode\n");
	ft_printf("  ESC   - Quit\n");
	ft_printf("  CLICK - Close window\n");

	while (!quit)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				quit = 1;
			else if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_SPACE)
				{
					if (all_ants_stopped() && current_turn < turn_line_count)
					{
						process_turn_movements(current_turn);
						current_turn++;
					}
				}
				else if (event.key.keysym.sym == SDLK_r)
				{
					current_turn = 0;
					animation_finished = 0;
					reset_ants_to_start();
				}
				else if (event.key.keysym.sym == SDLK_a)
				{
					auto_play = !auto_play;
					ft_printf("Auto-play: %s\n", auto_play ? "ON" : "OFF");
				}
				else if (event.key.keysym.sym == SDLK_ESCAPE)
					quit = 1;
			}
		}

		// Mode auto-play : avance automatiquement toutes les 1000ms
		if (auto_play && !animation_finished)
		{
			Uint32 current_auto_time = SDL_GetTicks();
			if (current_auto_time - last_auto_advance > 1000)
			{
				if (all_ants_stopped() && current_turn < turn_line_count)
				{
					process_turn_movements(current_turn);
					current_turn++;
					last_auto_advance = current_auto_time;
				}
				else if (current_turn >= turn_line_count && all_ants_stopped())
				{
					animation_finished = 1;
					ft_printf("Animation completed! Press 'R' to restart or ESC to quit.\n");
				}
			}
		}

		// Vérifier si l'animation est terminée en mode manuel
		if (!auto_play && current_turn >= turn_line_count && all_ants_stopped() && !animation_finished)
		{
			animation_finished = 1;
			ft_printf("Animation completed! Press 'R' to restart or ESC to quit.\n");
		}

		update_ant_animation();

		draw_rooms();
		draw_connections();
		draw_ants();

		font = TTF_OpenFont("assets/DejaVuSans.ttf", 16);
		if (font)
		{
			char turn_info[100];
			if (animation_finished)
				ft_sprintf(turn_info, "FINI - Tours: %d/%d", current_turn, turn_line_count);
			else
			{
				ft_sprintf(turn_info, "Tour: %d/%d %s", current_turn, turn_line_count,
						   auto_play ? "(AUTO)" : "");
			}
			SDL_Color text_color = {255, 255, 255, 255};
			SDL_Surface *text_surface = TTF_RenderText_Solid(font, turn_info, text_color);
			if (text_surface)
			{
				SDL_Rect text_rect = {10, 10, text_surface->w, text_surface->h};
				SDL_BlitSurface(text_surface, NULL, screen, &text_rect);
				SDL_FreeSurface(text_surface);
			}
			TTF_CloseFont(font);
		}

		SDL_Flip(screen);

		// Control the animation speed
		Uint32 current_time = SDL_GetTicks();
		if ((int)(current_time - last_time) < animation_speed)
			SDL_Delay(animation_speed - (int)(current_time - last_time));
		last_time = SDL_GetTicks();
	}

	// Clean up SDL
	SDL_Quit();

	for (int i = 0; i < turn_line_count; i++)
	{
		if (turn_lines[i])
			free(turn_lines[i]);
	}
	return (0);
}
