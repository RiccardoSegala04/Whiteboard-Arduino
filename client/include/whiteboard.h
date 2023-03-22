#ifndef WHITEBOARD_H
#define WHITEBOARD_H

#include <SDL2/SDL.h>

#define WB_MENU_SIZE    40

#define WB_WIDTH        440
#define WB_HEIGHT       320

#define WB_PEN_SIZE     1

struct whiteboard_point_t {
    uint16_t x, y;
};

struct whiteboard_t {
    SDL_Window* screen;
    SDL_Renderer* renderer;

    SDL_Texture *texbuf;

    struct whiteboard_point_t cursor;
    uint16_t pen_color;
    uint16_t pen_size;
    
    uint16_t scale;
};

int whiteboard_init(struct whiteboard_t *board, uint16_t scale);
int whiteboard_destroy(struct whiteboard_t *board);

int whiteboard_display(struct whiteboard_t *board);
int whiteboard_closed(struct whiteboard_t *board);

int whiteboard_clear(struct whiteboard_t *board);
int whiteboard_set_color(struct whiteboard_t *board, uint16_t color);
int whiteboard_set_size(struct whiteboard_t *board, uint16_t size);

int whiteboard_point(struct whiteboard_t *board, uint32_t x, uint32_t y);
int whiteboard_line_to(struct whiteboard_t *board, uint32_t x, uint32_t  y);

#endif
