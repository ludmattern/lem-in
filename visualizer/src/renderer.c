#include "visualizer.h"

void draw_rooms(void)
{
    // Clear screen with brown color
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 139, 69, 19));
    
    // Draw grass band at top
    SDL_Rect grass_band = {0, 0, 1200, 20};
    SDL_FillRect(screen, &grass_band, SDL_MapRGB(screen->format, 34, 139, 34));
    
    // Draw each room
    for (int i = 0; i < g_map.room_count; i++) {
        Room* room = &g_map.rooms[i];
        
        int screen_x = room->x * 20 + 100;
        int screen_y = room->y * 20 + 100;
        
        Uint32 room_color = SDL_MapRGB(screen->format, 80, 40, 8); // Couleur normale pour toutes les salles
        
        int ellipse_width = 32;
        int ellipse_height = 20;
        
        // Dessiner la salle avec la couleur normale
        for (int dx = -ellipse_width/2; dx <= ellipse_width/2; dx++) {
            for (int dy = -ellipse_height/2; dy <= ellipse_height/2; dy++) {
                float normalized_x = (float)dx / (ellipse_width/2);
                float normalized_y = (float)dy / (ellipse_height/2);
                if (normalized_x * normalized_x + normalized_y * normalized_y <= 1.0) {
                    SDL_Rect pixel = {screen_x + dx, screen_y + dy, 1, 1};
                    SDL_FillRect(screen, &pixel, room_color);
                }
            }
        }
        
        // Ajouter un contour coloré pour start et end
        if (room->is_start || room->is_end) {
            Uint32 border_color;
            if (room->is_start) {
                border_color = SDL_MapRGB(screen->format, 0, 255, 0); // Vert pour start
            } else {
                border_color = SDL_MapRGB(screen->format, 255, 0, 0); // Rouge pour end
            }
            
            // Dessiner le contour (ligne plus épaisse autour de l'ellipse)
            for (int dx = -ellipse_width/2 - 2; dx <= ellipse_width/2 + 2; dx++) {
                for (int dy = -ellipse_height/2 - 2; dy <= ellipse_height/2 + 2; dy++) {
                    float normalized_x = (float)dx / (ellipse_width/2);
                    float normalized_y = (float)dy / (ellipse_height/2);
                    float distance = normalized_x * normalized_x + normalized_y * normalized_y;
                    
                    // Dessiner le contour si on est dans la zone de contour (entre 1.0 et 1.2)
                    if (distance > 1.0 && distance <= 1.2) {
                        SDL_Rect pixel = {screen_x + dx, screen_y + dy, 1, 1};
                        SDL_FillRect(screen, &pixel, border_color);
                    }
                }
            }
        }
        
        // Display the room name with SDL1 TTF
        font = TTF_OpenFont("assets/DejaVuSans.ttf", 12);
        if (font) {
            SDL_Color text_color = {255, 255, 255, 255};
            SDL_Surface* text_surface = TTF_RenderText_Solid(font, room->name, text_color);
            if (text_surface) {
                SDL_Rect text_rect = {
                    screen_x - text_surface->w/2,
                    screen_y - 25,
                    text_surface->w,
                    text_surface->h
                };
                SDL_BlitSurface(text_surface, NULL, screen, &text_rect);
                SDL_FreeSurface(text_surface);
            }
            TTF_CloseFont(font);
        }
    }
}

void draw_connections(void)
{
    Uint32 line_color = SDL_MapRGB(screen->format, 80, 40, 8);
    
    // Draw each connection
    for (int i = 0; i < g_map.connection_count; i++) {
        Connection* conn = &g_map.connections[i];
                
        Room* from_room = &g_map.rooms[conn->from_room];
        Room* to_room = &g_map.rooms[conn->to_room];
        
        int x1 = from_room->x * 20 + 100;
        int y1 = from_room->y * 20 + 100;
        int x2 = to_room->x * 20 + 100;
        int y2 = to_room->y * 20 + 100;
                
        // Algorithme de Bresenham amélioré pour dessiner des lignes épaisses
        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        int sx = (x1 < x2) ? 1 : -1;
        int sy = (y1 < y2) ? 1 : -1;
        
        // Dessiner une ligne épaisse de 5 pixels
        for (int thickness = -2; thickness <= 2; thickness++) {
            int x_temp = x1;
            int y_temp = y1;
            int err_temp = dx - dy;
            
            while (x_temp != x2 || y_temp != y2) {
                // Dessiner un pixel à la position actuelle avec l'épaisseur
                for (int offset_x = -1; offset_x <= 1; offset_x++) {
                    for (int offset_y = -1; offset_y <= 1; offset_y++) {
                        SDL_Rect pixel = {x_temp + offset_x + thickness, y_temp + offset_y, 1, 1};
                        SDL_FillRect(screen, &pixel, line_color);
                    }
                }
                
                int e2 = 2 * err_temp;
                if (e2 > -dy) {
                    err_temp -= dy;
                    x_temp += sx;
                }
                if (e2 < dx) {
                    err_temp += dx;
                    y_temp += sy;
                }
            }
            
            // Dessiner le dernier pixel
            for (int offset_x = -1; offset_x <= 1; offset_x++) {
                for (int offset_y = -1; offset_y <= 1; offset_y++) {
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
    for (int i = 0; i < g_map.ant_count; i++) {
        Ant* ant = &g_map.ants[i];
        if (ant->ant_id == 0) continue;
        
        Room* current_room = &g_map.rooms[ant->current_room];
        int screen_x, screen_y;
        
        if (ant->is_moving && ant->progress < 1.0) {
            Room* target_room = &g_map.rooms[ant->target_room];
            int x1 = current_room->x * 20 + 100;
            int y1 = current_room->y * 20 + 100;
            int x2 = target_room->x * 20 + 100;
            int y2 = target_room->y * 20 + 100;
            
            screen_x = x1 + (int)((x2 - x1) * ant->progress);
            screen_y = y1 + (int)((y2 - y1) * ant->progress);
        } else {
            screen_x = current_room->x * 20 + 100;
            screen_y = current_room->y * 20 + 100;
        }
        
        Uint32 ant_color = SDL_MapRGB(screen->format, 0, 0, 0);
        for (int dx = -3; dx <= 3; dx++) {
            for (int dy = -3; dy <= 3; dy++) {
                if (dx*dx + dy*dy <= 9) {
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
        printf("Error: Failed to get map info\n");
        return (-1);
    }
    
    if (init_sdl() != 0) {
        printf("Error: Failed to initialize SDL\n");
        return -1;
    }
    
    reset_ants_to_start();
    
    current_turn = 1;
    
    SDL_Event event;
    int quit = 0;
    int animation_speed = 50;
    Uint32 last_time = SDL_GetTicks();
    
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE) {
                    if (all_ants_stopped() && current_turn < turn_line_count) {
                        process_turn_movements(current_turn);
                        current_turn++;
                    }
                } else if (event.key.keysym.sym == SDLK_r) {
                    current_turn = 0;
                    reset_ants_to_start();
                }
            }
        }
        
        update_ant_animation();
        
        draw_rooms();
        draw_connections();
        draw_ants();
        
        font = TTF_OpenFont("DejaVuSans.ttf", 16);
        if (font) {
            char turn_info[50];
            sprintf(turn_info, "Tour: %d/%d", current_turn, turn_line_count);
            SDL_Color text_color = {255, 255, 255, 255};
            SDL_Surface* text_surface = TTF_RenderText_Solid(font, turn_info, text_color);
            if (text_surface) {
                SDL_Rect text_rect = {10, 10, text_surface->w, text_surface->h};
                SDL_BlitSurface(text_surface, NULL, screen, &text_rect);
                SDL_FreeSurface(text_surface);
            }
            TTF_CloseFont(font);
        }
        
        SDL_Flip(screen);
        
        // Control the animation speed
        Uint32 current_time = SDL_GetTicks();
        if ((int)(current_time - last_time) < animation_speed) {
            SDL_Delay(animation_speed - (int)(current_time - last_time));
        }
        last_time = SDL_GetTicks();
    }
    
    // Clean up SDL
    SDL_Quit();
    
    for (int i = 0; i < turn_line_count; i++) {
        if (turn_lines[i]) {
            free(turn_lines[i]);
        }
    }
    return (0);
}
