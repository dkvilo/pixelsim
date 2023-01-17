#include "dk.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define V2_IMPLEMENTATION
#include "dk_linmath.h"

#define DK_TEXT_IMPLEMENTATION
#include "dk_text.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define SCREEN_FPS 120
#define SCREEN_TICKS_PER_FRAME (1000 / SCREEN_FPS)

enum GameState
{
  MENU = 0,
  PAUSE,
  IN_GAME,
  GAME_OVER,
  STATE_COUNT
};

typedef struct
{
  u32 flags;
  struct v2 position;
  struct v2 rotation;
  struct v2 dimentions;
  struct v2 velocity;
  struct v2 acceleration;
  SDL_Color color;
} entity_t;

void
set_flag(entity_t* entity, u32 flag)
{
  entity->flags |= flag;
}

void
clear_flag(entity_t* entity, u32 flag)
{
  entity->flags &= ~flag;
}

bool
has_flag(entity_t* entity, u32 flag)
{
  return entity->flags & flag;
}

typedef struct
{
  bool grid_enabled;
  bool simulation_running;
} game_state_t;

typedef enum icon_type
{
  ICON_NONE = 0,
  ICON_PLAY,
  ICON_STOP,
  ICON_GRID,
  ICON_COUNT
} icon_type_t;

typedef struct
{
  SDL_Rect rect;
  SDL_Texture* texture;
  icon_type_t type;
} icon_t;

static icon_t *icons = NULL;

typedef struct
{
  int FrameCount;
  int FrameRate;
  int FrameTime;
  int DeltaTime;
  int _lastFrame;
  int _lastTime;
  int _currentTime;
} game_stats_t;

typedef struct
{
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Event event;
  bool running;
  u32 state;
  dk_text_t text;
  dk_text_t ui_text;
  char* raw_text_input_buffer;
  game_state_t game_state;
  game_stats_t stats;
} game_t;

#define GRID_CELL_SIZE 10
#define GRID_WIDTH (WINDOW_WIDTH - 0) / GRID_CELL_SIZE
#define GRID_HEIGHT (WINDOW_HEIGHT - 200) / GRID_CELL_SIZE

#define GRID_CELL_EMPTY 0
#define GRID_CELL_FILLED 1

// define color palette for C64 (16 colors)
#define C64_BLACK (SDL_Color){ 0, 0, 0, 255 }
#define C64_WHITE (SDL_Color){ 255, 255, 255, 255 }
#define C64_RED (SDL_Color){ 136, 0, 0, 255 }
#define C64_CYAN (SDL_Color){ 170, 255, 238, 255 }
#define C64_PURPLE (SDL_Color){ 204, 68, 204, 255 }
#define C64_GREEN (SDL_Color){ 0, 204, 85, 255 }
#define C64_BLUE (SDL_Color){ 0, 0, 170, 255 }
#define C64_YELLOW (SDL_Color){ 238, 238, 119, 255 }
#define C64_ORANGE (SDL_Color){ 221, 136, 85, 255 }
#define C64_BROWN (SDL_Color){ 102, 68, 0, 255 }
#define C64_LIGHT_RED (SDL_Color){ 255, 119, 119, 255 }
#define C64_DARK_GREY (SDL_Color){ 51, 51, 51, 255 }
#define C64_GREY (SDL_Color){ 119, 119, 119, 255 }
#define C64_LIGHT_GREEN (SDL_Color){ 170, 255, 102, 255 }
#define C64_LIGHT_BLUE (SDL_Color){ 0, 136, 255, 255 }
#define C64_LIGHT_GREY (SDL_Color){ 187, 187, 187, 255 }
#define C64_COLOR_COUNT 16

SDL_Color C64_COLORS[C64_COLOR_COUNT] = {
  C64_BLACK,
  C64_WHITE,
  C64_RED,
  C64_CYAN,
  C64_PURPLE,
  C64_GREEN,
  C64_BLUE,
  C64_YELLOW,
  C64_ORANGE,
  C64_BROWN,
  C64_LIGHT_RED,
  C64_DARK_GREY,
  C64_GREY,
  C64_LIGHT_GREEN,
  C64_LIGHT_BLUE,
  C64_LIGHT_GREY,
};

SDL_Color* DK_PALLETE = NULL;

typedef enum {
  PIXEL_TYPE_WATER,
  PIXEL_TYPE_SAND,
  PIXEL_TYPE_FIRE,
  PIXEL_TYPE_COUNT
} pixel_type_t;

#define GRID_CELL_COLOR_EMPTY C64_WHITE

SDL_Color primary_color = C64_BROWN;
SDL_Color secondary_color = C64_BLACK;

typedef struct
{
  u32 col; // size in bytes (4)
  u32 row; // size in bytes (4)
  u8 size; // size in bytes (1)
  pixel_type_t type; // size in bytes (4)
  SDL_Color color; // size in bytes (4)
} pixel_t; // total size in bytes (13)

// pixel buffer
typedef struct
{
  pixel_t* pixels;
  u32 count;
} pixel_buffer_t;

pixel_type_t primary_pixel_type = PIXEL_TYPE_WATER;
pixel_buffer_t pixel_buffer;

void pixel_buffer_init(pixel_buffer_t* buffer)
{
  buffer->pixels = NULL;
  buffer->count = 0;
}

void pixel_buffer_add(pixel_buffer_t* buffer, pixel_t pixel)
{
  buffer->pixels = realloc(buffer->pixels, sizeof(pixel_t) * (buffer->count + 1));
  buffer->pixels[buffer->count] = pixel;
  buffer->count++;
}

void pixel_buffer_clear(pixel_buffer_t* buffer)
{
  free(buffer->pixels);
  buffer->pixels = NULL;
  buffer->count = 0;
}

void pixel_buffer_remove(pixel_buffer_t* buffer, u32 index)
{
  if (index < buffer->count) {
    buffer->pixels[index] = buffer->pixels[buffer->count - 1];
    buffer->count--;
    buffer->pixels = realloc(buffer->pixels, sizeof(pixel_t) * buffer->count);
  }
}

void pixel_buffer_remove_all(pixel_buffer_t* buffer, u32 col, u32 row)
{
  for (u32 i = 0; i < buffer->count; i++) {
    if (buffer->pixels[i].col == col && buffer->pixels[i].row == row) {
      pixel_buffer_remove(buffer, i);
      i--;
    }
  }
}

SDL_Color PixelTypeToColor(pixel_type_t type)
{
  switch (type) {
    case PIXEL_TYPE_WATER:
      return C64_BLUE;
    case PIXEL_TYPE_SAND:
      return C64_YELLOW;
    case PIXEL_TYPE_FIRE:
      return C64_RED;
    default:
      return C64_WHITE;
  }
}

void pixel_buffer_draw(pixel_buffer_t* buffer, SDL_Renderer* renderer)
{
  for (u32 i = 0; i < buffer->count; i++) {
    SDL_Rect rect = {
      .x = buffer->pixels[i].col * GRID_CELL_SIZE,
      .y = buffer->pixels[i].row * GRID_CELL_SIZE,
      .w = GRID_CELL_SIZE,
      .h = GRID_CELL_SIZE,
    };

    rect.x += (WINDOW_WIDTH - GRID_WIDTH * GRID_CELL_SIZE) / 2;
    rect.y += (WINDOW_HEIGHT - GRID_HEIGHT * GRID_CELL_SIZE) / 2;

    SDL_Color color = buffer->pixels[i].color;

    // if (buffer->pixels[i].row < GRID_HEIGHT) {
    //   color.r = (u8)((f32)color.r * (f32)buffer->pixels[i].row / (f32)(GRID_HEIGHT));
    //   color.g = (u8)((f32)color.g * (f32)buffer->pixels[i].row / (f32)(GRID_HEIGHT));
    //   color.b = (u8)((f32)color.b * (f32)buffer->pixels[i].row / (f32)(GRID_HEIGHT));
    // }

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
  }
}

void pixel_buffer_save_png(pixel_buffer_t* buffer, const char* filename, u32 scale)
{

  SDL_Surface* surface = SDL_CreateRGBSurface(0, GRID_WIDTH * scale, GRID_HEIGHT * scale, 32, 0, 0, 0, 0);
  SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 255, 255, 255, 255));

  for (u32 i = 0; i < buffer->count; i++) {
    pixel_t pixel = buffer->pixels[i];
    SDL_Rect rect;

    rect.x = pixel.col * scale;
    rect.y = pixel.row * scale;

    rect.w = scale;
    rect.h = scale;

    SDL_Color color = pixel.color; //PixelTypeToColor(pixel.type);
    SDL_FillRect(surface, &rect, SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a));
  }

  IMG_SavePNG(surface, filename);
  SDL_FreeSurface(surface);
}

Uint32 get_pixel(SDL_Surface* surface, u32 x, u32 y)
{
  u32 bpp = surface->format->BytesPerPixel;
  u8* p = (u8*)surface->pixels + y * surface->pitch + x * bpp;

  switch (bpp) {
    case 1:
      return *p;
    case 2:
      return *(u16*)p;
    case 3:
      if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        return p[0] << 16 | p[1] << 8 | p[2];
      else
        return p[0] | p[1] << 8 | p[2] << 16;
    case 4:
      return *(u32*)p;
    default:
      return 0;
  }

  return 0;
}

void png_to_pixel_buffer(pixel_buffer_t* buffer, const char* filename, int scale)
{
  pixel_buffer_clear(buffer);

  SDL_Surface* surface = IMG_Load(filename);
  if (surface != NULL) {
    pixel_buffer_clear(buffer);

    for (u32 y = 0; y < surface->h; y += scale) {
      for (u32 x = 0; x < surface->w; x += scale) {

        Uint32 pixel = get_pixel(surface, x, y);
        SDL_Color color;
        SDL_GetRGBA(pixel, surface->format, &color.r, &color.g, &color.b, &color.a);

        pixel_t p;
        p.col = x / scale;
        p.row = y / scale;
        p.color = color;

        pixel_buffer_add(buffer, p);
      }
    }

    SDL_FreeSurface(surface);
  }
}

void pixel_buffer_save(pixel_buffer_t* buffer, const char* filename)
{
  FILE* file = fopen(filename, "wb");
  if (file != NULL) {
    fwrite(&buffer->count, sizeof(u32), 1, file);
    fwrite(buffer->pixels, sizeof(pixel_t), buffer->count, file);
    fclose(file);
  }
}

void pixel_buffer_load(pixel_buffer_t* buffer, const char* filename)
{
  FILE* file = fopen(filename, "rb");
  if (file != NULL) {
    fread(&buffer->count, sizeof(u32), 1, file);
    buffer->pixels = malloc(sizeof(pixel_t) * buffer->count);
    fread(buffer->pixels, sizeof(pixel_t), buffer->count, file);
    fclose(file);
  }
}

void update_pixel_simulation(pixel_buffer_t* buffer)
{
  for (u32 i = 0; i < buffer->count; i++) {
    pixel_t* pixel = &buffer->pixels[i];

    if (pixel->row >= GRID_HEIGHT - 1) {
      continue;
    }

    pixel_t* below_pixel = NULL;
    for (u32 j = 0; j < buffer->count; j++) {
      if (buffer->pixels[j].col == pixel->col && buffer->pixels[j].row == pixel->row + 1) {
        below_pixel = &buffer->pixels[j];
        break;
      }
    }

    if (below_pixel == NULL) {
      pixel->row++;
    } else {

      // if sand below is water, turn below water into sand
      if (PIXEL_TYPE_WATER == below_pixel->type && PIXEL_TYPE_SAND == pixel->type) {
        below_pixel->type = PIXEL_TYPE_SAND;
      }

      // if fire below is sand, turn below sand into fire
      if (PIXEL_TYPE_FIRE == pixel->type && PIXEL_TYPE_SAND == below_pixel->type) {
        below_pixel->type = PIXEL_TYPE_FIRE;
      }

      // if fire below is water, turn fire into sand
      if (PIXEL_TYPE_FIRE == pixel->type && PIXEL_TYPE_WATER == below_pixel->type) {
        pixel->type = PIXEL_TYPE_SAND;
      }

      // if water below is fire, turn water into sand
      if (PIXEL_TYPE_WATER == pixel->type && PIXEL_TYPE_FIRE == below_pixel->type) {
        pixel->type = PIXEL_TYPE_SAND;
      }

      if (PIXEL_TYPE_FIRE == pixel->type) {
        for (u32 j = 0; j < buffer->count; j++) {
          if (buffer->pixels[j].col == pixel->col && buffer->pixels[j].row == pixel->row) {
            buffer->pixels[j] = buffer->pixels[buffer->count - 1];
            buffer->count--;
          }
        }
      }

      if (PIXEL_TYPE_SAND == pixel->type) {

        if (pixel->col > 0) {
          pixel_t* left_pixel = NULL;
          pixel_t* right_pixel = NULL;
          for (u32 j = 0; j < buffer->count; j++) {
            if (buffer->pixels[j].col == pixel->col - 1 && buffer->pixels[j].row == pixel->row + 1) {
              left_pixel = &buffer->pixels[j];
            } else if (buffer->pixels[j].col == pixel->col + 1 && buffer->pixels[j].row == pixel->row + 1) {
              right_pixel = &buffer->pixels[j];
            }
          }

          if (left_pixel == NULL) {
            pixel->col--;
          } else if (right_pixel == NULL) {
            pixel->col++;
          }

        } else if (pixel->col < GRID_WIDTH - 1) {
          pixel_t* right_pixel = NULL;
          for (u32 j = 0; j < buffer->count; j++) {
            if (buffer->pixels[j].col == pixel->col + 1 && buffer->pixels[j].row == pixel->row + 1) {
              right_pixel = &buffer->pixels[j];
            }
          }

          if (right_pixel == NULL) {
            pixel->col++;
          }
        }

        if (below_pixel == PIXEL_TYPE_WATER) {
          below_pixel->type = PIXEL_TYPE_SAND;
        }

      } else if (PIXEL_TYPE_WATER == pixel->type) {

        if (pixel->col > 0) {
          pixel_t* left_pixel = NULL;
          pixel_t* right_pixel = NULL;
          for (u32 j = 0; j < buffer->count; j++) {
            if (buffer->pixels[j].col == pixel->col - 1 && buffer->pixels[j].row == pixel->row) {
              left_pixel = &buffer->pixels[j];
            } else if (buffer->pixels[j].col == pixel->col + 1 && buffer->pixels[j].row == pixel->row) {
              right_pixel = &buffer->pixels[j];
            }
          }

          if (left_pixel == NULL) {
            pixel->col--;
          } else if (right_pixel == NULL) {
            pixel->col++;
          }

        } else if (pixel->col < GRID_WIDTH - 1) {
          pixel_t* right_pixel = NULL;
          for (u32 j = 0; j < buffer->count; j++) {
            if (buffer->pixels[j].col == pixel->col + 1 && buffer->pixels[j].row == pixel->row) {
              right_pixel = &buffer->pixels[j];
            }
          }
          if (right_pixel == NULL) {
            pixel->col++;
          }
        }
      }
    }
  }
}

bool draw_button(game_t* game, SDL_Rect rect, SDL_Color color, char* text);
bool draw_icon_button(game_t* game, SDL_Rect rect, SDL_Color color, SDL_Texture* texture);
void draw_TextInput(game_t* game, SDL_Point position, char* placeholder, void(callback)(char*));


SDL_Texture* load_texture(SDL_Renderer* renderer, char* path)
{
  SDL_Surface* surface = IMG_Load(path);
  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
  return texture;
}

void
game_init(game_t* game)
{

  icons = (icon_t*) malloc(sizeof(icon_t) * ICON_COUNT);
  memset(icons, 0, sizeof(icon_t) * ICON_COUNT);

  DK_PALLETE = (SDL_Color*) malloc(sizeof(SDL_Color) * 16);

  DK_PALLETE[0] = C64_BLACK;
  DK_PALLETE[1] = C64_WHITE;
  DK_PALLETE[2] = C64_RED;
  DK_PALLETE[3] = C64_CYAN;
  DK_PALLETE[4] = C64_PURPLE;
  DK_PALLETE[5] = C64_GREEN;
  DK_PALLETE[6] = C64_BLUE;
  DK_PALLETE[7] = C64_YELLOW;
  DK_PALLETE[8] = C64_ORANGE;
  DK_PALLETE[9] = C64_BROWN;
  DK_PALLETE[10] = C64_LIGHT_RED;
  DK_PALLETE[11] = C64_DARK_GREY;
  DK_PALLETE[12] = C64_GREY;
  DK_PALLETE[13] = C64_LIGHT_GREEN;
  DK_PALLETE[14] = C64_LIGHT_BLUE;
  DK_PALLETE[15] = C64_LIGHT_GREY;

  game->state = MENU;
  game->raw_text_input_buffer = malloc(sizeof(char) * 1080);

  game->game_state = (game_state_t) {
    .grid_enabled = false,
    .simulation_running = false,
  };

  game->stats = (game_stats_t) {0};

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("SDL_Init Error: %s ", SDL_GetError());
    exit(1);
  }

  game->window = SDL_CreateWindow("The Pong",
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  WINDOW_WIDTH,
                                  WINDOW_HEIGHT,
                                  SDL_WINDOW_SHOWN);

  // SDL_SetWindowFullscreen(game->window, SDL_WINDOW_FULLSCREEN);

  if (game->window == NULL) {
    printf("SDL_CreateWindow Error: %s ", SDL_GetError());
    exit(1);
  }

  game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (game->renderer == NULL) {
    printf("SDL_CreateRenderer Error: %s ", SDL_GetError());
    exit(1);
  }

  icons[ICON_GRID] = (icon_t) {
    .texture = load_texture(game->renderer, "assets/icons/grid.png"),
    .type = ICON_GRID,
    .rect = (SDL_Rect) {
      .x = 0,
      .y = 0,
      .w = 32,
      .h = 32,
    },
  };

  if (TTF_Init() != 0) {
    printf("TTF_Init Error: %s ", TTF_GetError());
    exit(1);
  }

  TTF_Font* font = TTF_OpenFont("assets/font/Monogram-Extended.ttf", 60);
  if (font == NULL) {
    fprintf(stderr, "Font Error: Unable to find a font file\n");
    exit(EXIT_FAILURE);
  }

  dk_text_init(&game->text, game->renderer, font, (SDL_Color){ 0, 0, 0, 255 });

  TTF_Font* ui_font = TTF_OpenFont("assets/font/Monogram-Extended.ttf", 35);
  if (ui_font == NULL) {
    fprintf(stderr, "Font Error: Unable to find a font file\n");
    exit(EXIT_FAILURE);
  }

  dk_text_init(&game->ui_text, game->renderer, ui_font, (SDL_Color){ 0, 0, 0, 255 });

  game->running = true;

  pixel_buffer_init(&pixel_buffer);
}

void
game_destroy(game_t* game)
{
  dk_text_destroy(&game->text);
  SDL_DestroyRenderer(game->renderer);
  SDL_DestroyWindow(game->window);
  TTF_Quit();
  SDL_Quit();
}

void
game_handle_events(game_t* game)
{
  while (SDL_PollEvent(&game->event)) {
    switch (game->event.type) {
      case SDL_TEXTINPUT:
        strcat(game->raw_text_input_buffer, game->event.text.text);
        break;
      case SDL_QUIT:
        game->running = false;
        break;
      case SDL_KEYDOWN:
        switch (game->event.key.keysym.sym) {
          case SDLK_BACKSPACE:
            game->raw_text_input_buffer[strlen(game->raw_text_input_buffer) - 1] = '\0';
            break;
          case SDLK_ESCAPE:
            game->state = PAUSE;
            break;
          case SDLK_SPACE:
            if (game->state == MENU || game->state == PAUSE) {
              game->state = IN_GAME;
            }
            break;
        }
        break;
    }
  }
}

int mouse_x, mouse_y = 0;
int mouse_x_prev, mouse_y_prev = 0;
int coord_x, coord_y = 0;

int posToGridWithOffsetX(int x)
{
  return (x - (WINDOW_WIDTH - GRID_WIDTH * GRID_CELL_SIZE) / 2) / GRID_CELL_SIZE;
}

int posToGridWithOffsetY(int y)
{
  return (y - (WINDOW_HEIGHT - GRID_HEIGHT * GRID_CELL_SIZE) / 2) / GRID_CELL_SIZE;
}


void
game_update(game_t* game)
{
  switch (game->state) {
    case MENU:
      break;
    case PAUSE:
      break;
    case IN_GAME: {

      SDL_GetMouseState(&mouse_x, &mouse_y);

      bool is_in_bounds =
          mouse_x > (WINDOW_WIDTH - GRID_WIDTH * GRID_CELL_SIZE) / 2 &&
          mouse_x < (WINDOW_WIDTH + GRID_WIDTH * GRID_CELL_SIZE) / 2 &&
          mouse_y > (WINDOW_HEIGHT - GRID_HEIGHT * GRID_CELL_SIZE) / 2 &&
          mouse_y < (WINDOW_HEIGHT + GRID_HEIGHT * GRID_CELL_SIZE) / 2;

      if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT) && is_in_bounds) {

        coord_x = posToGridWithOffsetX(mouse_x);
        coord_y = posToGridWithOffsetY(mouse_y);

        pixel_t pixel = { .col = coord_x, .row = coord_y, .size = GRID_CELL_SIZE, .type = primary_pixel_type, .color = primary_color };
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_E]) {
          pixel_buffer_remove_all(&pixel_buffer, coord_x, coord_y);
        } else {
          pixel_buffer_add(&pixel_buffer, pixel);
        }
      }

      if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_1]) {
        primary_pixel_type = PIXEL_TYPE_WATER;
      } else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_2]) {
        primary_pixel_type = PIXEL_TYPE_SAND;
      } else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_3]) {
        primary_pixel_type = PIXEL_TYPE_FIRE;
      }

      if (game->game_state.simulation_running) {
        update_pixel_simulation(&pixel_buffer);
      }

    } break;
    case GAME_OVER:
      break;
  }
}

void text_input_handler(char* text_input_buffer)
{
  printf("Text Input: %s\n", text_input_buffer);
}

void
game_render(game_t* game)
{
  SDL_Color color = C64_LIGHT_GREEN;
  SDL_SetRenderDrawColor(game->renderer, color.r, color.g, color.b, color.a);
  SDL_RenderClear(game->renderer);

  switch (game->state) {
    case MENU: {
      char* menu_str = "Press Space to Start";
      dk_text_draw(
        &game->text,
        menu_str,
        (WINDOW_WIDTH / 2) - dk_text_width(&game->text, menu_str) / 2,
        WINDOW_HEIGHT / 2 - dk_text_height(&game->text, menu_str) / 2);
    } break;

    case PAUSE: {
      char* pause_str = "Press Space to Resume";
      dk_text_draw(
        &game->text,
        pause_str,
        (WINDOW_WIDTH / 2) - dk_text_width(&game->text, pause_str) / 2,
        (WINDOW_HEIGHT / 2) - dk_text_height(&game->text, pause_str) / 2);
    } break;

    case IN_GAME: {

      game->text.color = (SDL_Color){ 19, 15, 64, 100 };


      SDL_Color bg_color = {47, 53, 66, 255};
      SDL_SetRenderDrawColor(game->renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
      SDL_RenderClear(game->renderer);

      SDL_Rect canvas_rect = {
        (WINDOW_WIDTH - GRID_WIDTH * GRID_CELL_SIZE) / 2,
        (WINDOW_HEIGHT - GRID_HEIGHT * GRID_CELL_SIZE) / 2,
        GRID_WIDTH * GRID_CELL_SIZE,
        GRID_HEIGHT * GRID_CELL_SIZE
      };

      // @background
      SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
      SDL_RenderFillRect(game->renderer, &canvas_rect);

      // @grid
      if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LSHIFT] || game->game_state.grid_enabled) {
        SDL_Color grid_color = C64_LIGHT_GREY;
        SDL_SetRenderDrawColor(game->renderer, grid_color.r, grid_color.g, grid_color.b, grid_color.a);
        for (int i = 0; i < GRID_WIDTH; i++) {
          SDL_RenderDrawLine(game->renderer,
                             (WINDOW_WIDTH - GRID_WIDTH * GRID_CELL_SIZE) / 2 + i * GRID_CELL_SIZE,
                             (WINDOW_HEIGHT - GRID_HEIGHT * GRID_CELL_SIZE) / 2,
                             (WINDOW_WIDTH - GRID_WIDTH * GRID_CELL_SIZE) / 2 + i * GRID_CELL_SIZE,
                             (WINDOW_HEIGHT + GRID_HEIGHT * GRID_CELL_SIZE) / 2);

          for (int j = 0; j < GRID_HEIGHT; j++) {
            SDL_RenderDrawLine(game->renderer,
                               (WINDOW_WIDTH - GRID_WIDTH * GRID_CELL_SIZE) / 2,
                               (WINDOW_HEIGHT - GRID_HEIGHT * GRID_CELL_SIZE) / 2 + j * GRID_CELL_SIZE,
                               (WINDOW_WIDTH + GRID_WIDTH * GRID_CELL_SIZE) / 2,
                               (WINDOW_HEIGHT - GRID_HEIGHT * GRID_CELL_SIZE) / 2 + j * GRID_CELL_SIZE);
          }
        }
      }

      pixel_buffer_draw(&pixel_buffer, game->renderer);

      // @border
      SDL_RenderDrawRect(game->renderer, &canvas_rect);


      int x = posToGridWithOffsetX(mouse_x);
      int y = posToGridWithOffsetY(mouse_y);

      SDL_Rect rect = { x - GRID_CELL_SIZE / 2, y - GRID_CELL_SIZE / 2, GRID_CELL_SIZE, GRID_CELL_SIZE };

      rect.x = (WINDOW_WIDTH - GRID_WIDTH * GRID_CELL_SIZE) / 2 + x * GRID_CELL_SIZE;
      rect.y = (WINDOW_HEIGHT - GRID_HEIGHT * GRID_CELL_SIZE) / 2 + y * GRID_CELL_SIZE;

      // draw border around cursor
      SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
      SDL_RenderDrawRect(game->renderer, &rect);

      char str[255];
      sprintf(str, "(%d, %d)", x, y);
      SDL_Point position = { WINDOW_WIDTH - dk_text_width(&game->text, str), WINDOW_HEIGHT - dk_text_height(&game->text, str) };
      dk_text_draw(&game->text, str, position.x, position.y);

      SDL_Rect rect2 = { 0, 0, 200, 40 };
      if (draw_button(game, rect2, C64_CYAN, "Clear")) {
        pixel_buffer_clear(&pixel_buffer);
      }

      SDL_Rect rect3 = { 210, 0, 200, 40 };
      if (draw_button(game, rect3, C64_LIGHT_GREEN, "Export PNG")) {
        pixel_buffer_save_png(&pixel_buffer, "canvas.png", 20);
      }

      SDL_Point position2 = { 630, 0 };
      char* placeholder = "Filename";
      draw_TextInput(game, position2, placeholder, &text_input_handler);

      SDL_Rect rect5 = { 810, 0, 20, 40 };
      if (draw_button(game, rect5, C64_LIGHT_GREEN, "Load")) {
        if (game->raw_text_input_buffer[0] != '\0') {
          pixel_buffer_load(&pixel_buffer, game->raw_text_input_buffer);
        }
      }

      SDL_Rect rect4 = { 880, 0, 20, 40 };
      if (draw_button(game, rect4, C64_LIGHT_BLUE, "Save")) {
        if (game->raw_text_input_buffer[0] != '\0') {
          pixel_buffer_save(&pixel_buffer, game->raw_text_input_buffer);
        }
      }

      SDL_Rect rect9 = { 420, 0, 200, 40 };

      int is_mouse_pressed = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);

      if (draw_button(game, rect9, C64_LIGHT_GREEN, "Load PNG") && is_mouse_pressed) {
        png_to_pixel_buffer(&pixel_buffer, "canvas.png", 20);
      }

      // SDL_Rect rect6 = { 0, 50, 200, 40 };
      // if (draw_button(game, rect6, C64_LIGHT_BLUE, "Enable Grid")) {
      //   game->game_state.grid_enabled = true;
      // }

      SDL_Rect rect10 = { 10, 53, icons[ICON_GRID].rect.w, icons[ICON_GRID].rect.h };
      if(draw_icon_button(game, rect10, C64_BLACK, icons[ICON_GRID].texture)) {
        SDL_Delay(100);
        game->game_state.grid_enabled = !game->game_state.grid_enabled;
        SDL_Delay(0);
      }

      SDL_Rect rect7 = { 210, 50, 200, 40 };
      if (draw_button(game, rect7, C64_LIGHT_BLUE, "Play")) {
        game->game_state.simulation_running = true;
      }

      SDL_Rect rect8 = { 420, 50, 200, 40 };
      if (draw_button(game, rect8, C64_LIGHT_RED, "Stop")) {
        game->game_state.simulation_running = false;
        SDL_MessageBoxData messageboxdata = { SDL_MESSAGEBOX_INFORMATION, game->window, "Simulation Stopped", "Simulation Stopped", 0, NULL, NULL };
        int buttonid = 0;
        SDL_ShowMessageBox(&messageboxdata, &buttonid);
      }

      {
        char str2[255];
        sprintf(str2, "FPS: %d, %d", game->stats.FrameRate, game->stats.FrameTime);
        SDL_Point position2 = { WINDOW_WIDTH - dk_text_width(&game->text, str2), 0 };
        dk_text_draw(&game->text, str2, position2.x, position2.y);
      }

      // bottom draw colrs for palette selection
      int size = 50;
      for (int i = 0; i < 16; i++) {
        SDL_Rect rect = { i * size, WINDOW_HEIGHT - size - 20, size, size };
        char lab[255];
        sprintf(lab, "%d", i);
        if(draw_button(game, rect, C64_COLORS[i], lab)) {
          primary_color = C64_COLORS[i];
        }
      }

      // draw pixel types
      int size2 = 20;
      for (int i = 0; i < 4; i++) {
        SDL_Rect rect = { WINDOW_WIDTH - size2 * 4 + i * size2, 0, size2, size2 };
        char lab[255];
        sprintf(lab, "%d", i);
        if(draw_button(game, rect, C64_COLORS[i], lab)) {
          primary_pixel_type = i;
        }
      }

    }

    break;
    case GAME_OVER: {
      char* game_over_str = "Game Over";
      dk_text_draw(&game->text,
                   game_over_str,
                   (WINDOW_WIDTH / 2) -
                     dk_text_width(&game->text, game_over_str) / 2,
                   WINDOW_HEIGHT / 2);
    } break;
  }

  SDL_RenderPresent(game->renderer);
}

bool draw_button(game_t* game, SDL_Rect rect, SDL_Color color, char* text)
{

  SDL_Color btn_color = color;

  int text_width = dk_text_width(&game->ui_text, text);
  int text_height = dk_text_height(&game->ui_text, text);

  if (rect.w < text_width) {
    rect.w = text_width + 10;
  }

  if (rect.h < text_height) {
    rect.h = text_height + 10;
  }

  int mouse_pressed = 0;

  if (rect.x < mouse_x && mouse_x < rect.x + rect.w && rect.y < mouse_y && mouse_y < rect.y + rect.h) {

    mouse_pressed = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);

    // Hover color
    btn_color.r = color.r * 0.5;
    btn_color.g = color.g * 0.5;
    btn_color.b = color.b * 0.5;
    btn_color.a = color.a * 0.5;

    // Button Pressed color
    if (mouse_pressed) {
      btn_color.r = color.r * 0.2;
      btn_color.g = color.g * 0.2;
      btn_color.b = color.b * 0.2;
      btn_color.a = color.a * 0.2;
    }

  } else {
    btn_color = color;
  }

  SDL_SetRenderDrawColor(game->renderer, btn_color.r, btn_color.g, btn_color.b, btn_color.a);
  SDL_RenderFillRect(game->renderer, &rect);

  SDL_Point position = {
    rect.x + rect.w / 2 - dk_text_width(&game->ui_text, text) / 2,
    rect.y + rect.h / 2 - dk_text_height(&game->ui_text, text) / 2
  };

  // BG color is dark, use white text
  SDL_Color text_color = C64_BLACK;
  if (btn_color.r < 128 && btn_color.g < 128 && btn_color.b < 128) {
    text_color = C64_WHITE;
  }

  game->ui_text.color = text_color;
  dk_text_draw(&game->ui_text, text, position.x, position.y);

  if (mouse_pressed) {
    return true;
  }

  return false;
}

bool draw_icon_button(game_t* game, SDL_Rect rect, SDL_Color color, SDL_Texture* texture)
{

  SDL_Color btn_color = color;

  int mouse_pressed = 0;

  if (rect.x < mouse_x && mouse_x < rect.x + rect.w && rect.y < mouse_y && mouse_y < rect.y + rect.h) {

    mouse_pressed = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);

    // Hover color
    btn_color.r = color.r * 0.5;
    btn_color.g = color.g * 0.5;
    btn_color.b = color.b * 0.5;
    btn_color.a = color.a * 0.5;

    // Button Pressed color
    if (mouse_pressed) {
      btn_color.r = color.r * 0.2;
      btn_color.g = color.g * 0.2;
      btn_color.b = color.b * 0.2;
      btn_color.a = color.a * 0.2;
    }

  } else {
    btn_color = color;
  }

  SDL_Rect image_rect = {
    rect.x + rect.w / 2 - rect.w / 2,
    rect.y + rect.h / 2 - rect.h / 2,
    rect.w,
    rect.h
  };

  SDL_RenderCopy(game->renderer, texture, NULL, &image_rect);

  if (mouse_pressed) {
    return true;
  }

  return false;
}

void draw_TextInput(game_t* game, SDL_Point position, char* placeholder, void(callback)(char*)) {

  int text_width = dk_text_width(&game->ui_text, game->raw_text_input_buffer);
  int text_height = dk_text_height(&game->ui_text, game->raw_text_input_buffer);

  SDL_Color color = C64_YELLOW;
  SDL_SetRenderDrawColor(game->renderer, color.r, color.g, color.b, color.a);

  SDL_Rect rect = { position.x, position.y, 200, 40 };

  if (rect.w < text_width) {
    rect.w = text_width + 10;
  }

  if (rect.h < text_height) {
    rect.h = text_height + 10;
  }

  SDL_RenderFillRect(game->renderer, &rect);

  const int text_margin = 5;
  if (strlen(game->raw_text_input_buffer) == 0) {
    dk_text_draw(&game->ui_text, placeholder, position.x + text_margin, position.y);
  } else {
    dk_text_draw(&game->ui_text, game->raw_text_input_buffer, position.x + text_margin, position.y);
  }

  // Unerline Curso
  SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
  SDL_RenderFillRect(game->renderer, &(SDL_Rect){ position.x + text_width + text_margin, position.y + text_height, 10, 5 });

  static int number_of_keys = 0;
  if (SDL_GetKeyboardState(&number_of_keys)[SDL_SCANCODE_RETURN]) {
    callback(game->raw_text_input_buffer);
  }
}


int
main(int argc, char const* argv[])
{
  srand((unsigned int)time(NULL));

  game_t game;
  game_init(&game);


  while (game.running) {

    game.stats.FrameCount++;
    game.stats._currentTime = SDL_GetTicks();
    game.stats.DeltaTime = game.stats._currentTime - game.stats._lastTime;
    game.stats.FrameTime += game.stats.DeltaTime;
    game.stats._lastTime = game.stats._currentTime;


    if (game.stats.FrameTime >= 1000) {
      game.stats.FrameRate = game.stats.FrameCount;
      game.stats.FrameCount = 0;
      game.stats.FrameTime = 0;
    }

    game.stats._lastFrame = game.stats._currentTime;

    game_handle_events(&game);
    game_update(&game);
    game_render(&game);

    int target_frame_time = 1000 / 60; // 60 FPS
    int frame_time = SDL_GetTicks() - game.stats._lastFrame;
    if (frame_time < target_frame_time) {
      SDL_Delay(target_frame_time - frame_time);
    }
  }

  game_destroy(&game);

  return 0;
}

