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

#define DK_COLOR_IMPLEMENTATION
#include "dk_color.h"

#include "dk_pixelbuffer.h"
#include "dk_app.h"
#include "dk_macros.h"

#define DK_ICON_IMPLEMENTATION
#include "dk_icon.h"

#define DK_UI_IMPLEMENTATION
#include "dk_ui.h"

enum GameState
{
  MENU = 0,
  PAUSE,
  IN_GAME,
  GAME_OVER,
  STATE_COUNT
};

static icon_t *icons = NULL;
SDL_Color* DK_PALLETE = NULL;

SDL_Color primary_color = C64_BROWN;
SDL_Color secondary_color = C64_BLACK;

pixel_type_t primary_pixel_type = PIXEL_TYPE_WATER;
pixel_buffer_t pixel_buffer;

void
game_init(app_t* game)
{

  icons = (icon_t*) malloc(sizeof(icon_t) * ICON_COUNT);
  memset(icons, 0, sizeof(icon_t) * ICON_COUNT);

  DK_PALLETE = (SDL_Color*) malloc(sizeof(SDL_Color) * C64_COLOR_COUNT);
  load_colors(DK_PALLETE);

  game->state = MENU;
  game->raw_text_input_buffer = malloc(sizeof(char) * 1080);

  game->game_state = (app_state_t) {
    .grid_enabled = true,
    .simulation_running = false,
  };

  game->stats = (app_stats_t) {0};
  game->mouse = (app_mouse_t) {0};

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

  if (game->window == NULL) {
    printf("SDL_CreateWindow Error: %s ", SDL_GetError());
    exit(1);
  }

  game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (game->renderer == NULL) {
    printf("SDL_CreateRenderer Error: %s ", SDL_GetError());
    exit(1);
  }

  // NOTE (David): Before loading icons to the textrue, we need to initialize SDL_Renderer
  icon_load_32x32(&icons[ICON_GRID], ICON_GRID, game->renderer);

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
game_destroy(app_t* game)
{
  dk_text_destroy(&game->text);
  SDL_DestroyRenderer(game->renderer);
  SDL_DestroyWindow(game->window);
  TTF_Quit();
  SDL_Quit();
}

void
game_handle_events(app_t* game)
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
game_update(app_t* game)
{
  switch (game->state) {
    case MENU:
      break;
    case PAUSE:
      break;
    case IN_GAME: {

      SDL_GetMouseState(&game->mouse.x, &game->mouse.y);

      bool is_in_bounds =
          game->mouse.x > (WINDOW_WIDTH - GRID_WIDTH * GRID_CELL_SIZE) / 2 &&
          game->mouse.x < (WINDOW_WIDTH + GRID_WIDTH * GRID_CELL_SIZE) / 2 &&
          game->mouse.y > (WINDOW_HEIGHT - GRID_HEIGHT * GRID_CELL_SIZE) / 2 &&
          game->mouse.y < (WINDOW_HEIGHT + GRID_HEIGHT * GRID_CELL_SIZE) / 2;

      if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT) && is_in_bounds) {

        coord_x = posToGridWithOffsetX(game->mouse.x);
        coord_y = posToGridWithOffsetY(game->mouse.y);

        pixel_t pixel = { .col = coord_x, .row = coord_y, .size = GRID_CELL_SIZE, .type = primary_pixel_type, .color = primary_color };
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_E]) {
          pixel_buffer_remove_all(&pixel_buffer, coord_x, coord_y);
        } else {
          pixel_buffer_add_circle(&pixel_buffer, pixel, 10);
#if 0
          pixel_buffer_add(&pixel_buffer, pixel);
          pixel_buffer_add_line(&pixel_buffer, pixel, 5, 0);
          pixel_buffer_add_line(&pixel_buffer, pixel, 5, 1);
          pixel_buffer_add_line(&pixel_buffer, pixel, 5, 2);
          pixel_buffer_add_line(&pixel_buffer, pixel, 5, 3);
          pixel_buffer_add_rect(&pixel_buffer, pixel, 10, 10);
          pixel_buffer_add_rect_outline(&pixel_buffer, pixel, 10, 10);
#endif
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
game_render(app_t* game)
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


      int x = posToGridWithOffsetX(game->mouse.x);
      int y = posToGridWithOffsetY(game->mouse.y);

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
      if (dk_ui_button(game, rect2, C64_CYAN, "Clear")) {
        pixel_buffer_clear(&pixel_buffer);
      }

      SDL_Rect rect3 = { 210, 0, 200, 40 };
      if (dk_ui_button(game, rect3, C64_LIGHT_GREEN, "Export PNG")) {
        pixel_buffer_save_png(&pixel_buffer, "canvas.png", 20);
      }

      SDL_Point position2 = { 630, 0 };
      char* placeholder = "Filename";
      dk_ui_text_input(game, position2, placeholder, &text_input_handler);

      SDL_Rect rect5 = { 810, 0, 20, 40 };
      if (dk_ui_button(game, rect5, C64_LIGHT_GREEN, "Load")) {
        if (game->raw_text_input_buffer[0] != '\0') {
          pixel_buffer_load(&pixel_buffer, game->raw_text_input_buffer);
        }
      }

      SDL_Rect rect4 = { 880, 0, 20, 40 };
      if (dk_ui_button(game, rect4, C64_LIGHT_BLUE, "Save")) {
        if (game->raw_text_input_buffer[0] != '\0') {
          pixel_buffer_save(&pixel_buffer, game->raw_text_input_buffer);
        }
      }

      SDL_Rect rect9 = { 420, 0, 200, 40 };
      int is_mouse_pressed = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);
      if (dk_ui_button(game, rect9, C64_LIGHT_GREEN, "Load PNG") && is_mouse_pressed) {
        png_to_pixel_buffer(&pixel_buffer, "canvas.png", 20);
      }

      // SDL_Rect rect6 = { 0, 50, 200, 40 };
      // if (draw_button(game, rect6, C64_LIGHT_BLUE, "Enable Grid")) {
      //   game->game_state.grid_enabled = true;
      // }

      SDL_Rect rect10 = { 10, 53, icons[ICON_GRID].rect.w, icons[ICON_GRID].rect.h };
      if(dk_ui_icon_button(game, rect10, C64_BLACK, icons[ICON_GRID].texture)) {
        SDL_Delay(100);
        game->game_state.grid_enabled = !game->game_state.grid_enabled;
        SDL_Delay(0);
      }

      SDL_Rect rect7 = { 210, 50, 200, 40 };
      if (dk_ui_button(game, rect7, C64_LIGHT_BLUE, "Play")) {
        game->game_state.simulation_running = true;
      }

      SDL_Rect rect8 = { 420, 50, 200, 40 };
      if (dk_ui_button(game, rect8, C64_LIGHT_RED, "Stop")) {
        game->game_state.simulation_running = false;
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
        if(dk_ui_button(game, rect, C64_COLORS[i], lab)) {
          primary_color = C64_COLORS[i];
        }
      }

      // draw pixel types
      int size2 = 20;
      for (int i = 0; i < 4; i++) {
        SDL_Rect rect = { WINDOW_WIDTH - size2 * 4 + i * size2, 0, size2, size2 };
        char lab[255];
        sprintf(lab, "%d", i);
        if(dk_ui_button(game, rect, C64_COLORS[i], lab)) {
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




int
main(int argc, char const* argv[])
{
  srand((unsigned int)time(NULL));

  app_t game;
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

