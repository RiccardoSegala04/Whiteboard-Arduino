#include <whiteboard.h>
#include <strings.h>
#include <SDL2_gfxPrimitives.h>

int whiteboard_init(struct whiteboard_t* board, uint16_t scale) {

    bzero(board, sizeof(*board));

    board->scale = scale;

    board->pen_size = WB_PEN_SIZE*scale;
    board->pen_color = 0xffff;

    SDL_Init(SDL_INIT_EVERYTHING);
    
    board->screen = SDL_CreateWindow(
            "Whiteboard", 
            SDL_WINDOWPOS_CENTERED, 
            SDL_WINDOWPOS_CENTERED, 
            WB_WIDTH*scale, WB_HEIGHT*scale, 0
    );

    if(!board->screen) {
        return 1;
    }

    board->renderer = SDL_CreateRenderer(
            board->screen, 
            -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

   if(!board->renderer) {
        return 2;
    }
    
    board->texbuf = SDL_CreateTexture(
        board->renderer, 
        SDL_PIXELFORMAT_RGBA8888, 
        SDL_TEXTUREACCESS_TARGET,
        WB_WIDTH*scale, WB_HEIGHT*scale
    );

    return 0;
}

int whiteboard_destroy(struct whiteboard_t *board) {
    
    SDL_DestroyWindow(board->screen);
    SDL_DestroyRenderer(board->renderer);
    SDL_Quit();

    return 0;
}

int whiteboard_display(struct whiteboard_t *board) {
    
    SDL_RenderCopy(board->renderer, board->texbuf, NULL, NULL);
    SDL_RenderPresent(board->renderer);

    return 0;
}

int whiteboard_closed(struct whiteboard_t *board) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        if(event.type==SDL_QUIT) {
            return 1;
        }
    }

    return 0;
}

int whiteboard_clear(struct whiteboard_t *board) {

    SDL_SetRenderTarget(board->renderer, board->texbuf);
    SDL_SetRenderDrawColor(board->renderer, 255, 255, 255, 255);
    SDL_RenderClear(board->renderer);
    SDL_SetRenderTarget(board->renderer, NULL);

    return 0;
}

int whiteboard_set_color(struct whiteboard_t *board, uint16_t color) {
    board->pen_color = color;
    return 0;
}

int whiteboard_set_size(struct whiteboard_t *board, uint16_t size) {
    board->pen_size = size;
    return 0;
}

static int map_color(int fmax, int tmax, int val) {
    return tmax*val/fmax; 
}

int whiteboard_draw_point(struct whiteboard_t *board, uint32_t x, uint32_t y) {

    SDL_SetRenderTarget(board->renderer, board->texbuf);

    SDL_SetRenderDrawColor(board->renderer, 
        map_color(31, 255, board->pen_color>>11),  
        map_color(63, 255, (board->pen_color>>5) & 0x3F), 
        map_color(31, 255, board->pen_color & 0x1f), 
        255
    );

    filledCircleRGBA(
        board->renderer, x, y, board->pen_size*board->scale,     
        map_color(31, 255, board->pen_color>>11),  
        map_color(63, 255, (board->pen_color>>5) & 0x3F), 
        map_color(31, 255, board->pen_color & 0x1f), 
        255
    );

    SDL_SetRenderTarget(board->renderer, NULL);

    return 0;
}

int whiteboard_point(struct whiteboard_t *board, uint32_t x, uint32_t y) {
    x*=board->scale;
    y*=board->scale;
    int rc = whiteboard_draw_point(board, x, y);
    board->cursor.x = x;
    board->cursor.y = y;

    return rc;
}

static void swap(uint32_t* a, uint32_t* b) {
    uint32_t tmp = *a;
    *a = *b;
    *b = tmp;
}

int whiteboard_line_to(struct whiteboard_t *board, uint32_t x, uint32_t  y) {
    x*=board->scale;
    y*=board->scale;
    
    int tmpx = x;
    int tmpy = y;

    uint32_t x0 = board->cursor.x;
    uint32_t y0 = board->cursor.y;

    int16_t steep = abs(y - y0) > abs(x - x0);
    if (steep) {
        swap(&x0, &y0);
        swap(&x, &y);
    }

    if (x0 > x) {
        swap(&x0, &x);
        swap(&y0, &y);
    }

    int16_t dx, dy;
    dx = x - x0;
    dy = abs(y - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0 <= x; x0++) {
        if (steep) {
            whiteboard_draw_point(board, y0, x0);
        } else {
            whiteboard_draw_point(board, x0, y0);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }    

    board->cursor.x = tmpx;
    board->cursor.y = tmpy;
    return 0;
}


