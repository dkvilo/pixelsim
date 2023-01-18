#include "dk.h"

#if defined(__APPLE__) || defined(__MACH__)
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#endif

#if defined(__linux__) || defined(__unix__)
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#endif

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

typedef enum {
  BRUSH_RECT = 0,
  BRUSH_CIRCLE,
  BRUSH_LINE,
  BRUSH_RECT_OUTLINE,
  BRUSH_COUNT,
  BRUSH_ERASER,
  BRUSH_PENCIL,
} BrushType;

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

u8 pixel_size = GRID_CELL_SIZE;

pixel_type_t primary_pixel_type = PIXEL_TYPE_WATER;
pixel_buffer_t pixel_buffer;

BrushType primary_brush_type = BRUSH_CIRCLE;
i32 primary_brush_size = GRID_CELL_SIZE;

void
game_init(app_t* game)
{

  icons = (icon_t*) malloc(sizeof(icon_t) * ICON_COUNT);
  memset(icons, 0, sizeof(icon_t) * ICON_COUNT);

  DK_PALLETE = (SDL_Color*) malloc(sizeof(SDL_Color) * C64_COLOR_COUNT);
  load_colors(DK_PALLETE);

  game->state = IN_GAME;
  game->raw_text_input_buffer = malloc(sizeof(char) * 1080);

  game->game_state = (app_state_t) {
    .grid_enabled = true,
    .simulation_running = false,
  };

  game->stats = (app_stats_t) {0};
  game->mouse = (app_mouse_t) {0};
  game->camera = (app_camera_t) {0};
  game->ui_focused = false;

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("SDL_Init Error: %s ", SDL_GetError());
    exit(1);
  }

  game->window = SDL_CreateWindow("Pixel Simulator - v0.1",
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  WINDOW_WIDTH,
                                  WINDOW_HEIGHT,
                                  SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

  if (FULLSCREEN) {
    SDL_SetWindowFullscreen(game->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
  }

  if (game->window == NULL) {
    printf("SDL_CreateWindow Error: %s ", SDL_GetError());
    exit(1);
  }

  game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (game->renderer == NULL) {
    printf("SDL_CreateRenderer Error: %s ", SDL_GetError());
    exit(1);
  }

  // NOTE (David): Before load2ing icons to the textrue, we need to initialize SDL_Renderer
  icon_load_32x32(&icons[ICON_GRID], ICON_GRID, game->renderer);
  icon_load_32x32(&icons[ICON_PLAY], ICON_PLAY, game->renderer);
  icon_load_32x32(&icons[ICON_PAUSE], ICON_PAUSE, game->renderer);
  icon_load_32x32(&icons[ICON_EXIT], ICON_EXIT, game->renderer);
  icon_load_32x32(&icons[ICON_SAVE], ICON_SAVE, game->renderer);
  icon_load_32x32(&icons[ICON_CLEAR], ICON_CLEAR, game->renderer);

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
  pixel_buffer_init(&pixel_buffer);

  game->running = true;
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
      case (SDL_DROPFILE): {
        if (game->state == IN_GAME) {
          char* dropped_filedir = game->event.drop.file;
          pixel_buffer_load(&pixel_buffer, dropped_filedir);
          free(dropped_filedir);
        }
      }
        break;
      case SDL_MOUSEWHEEL:
        if (game->event.wheel.y > 0) {
          if (primary_brush_size < 100) {
            primary_brush_size += 1;
          }
        } else if (game->event.wheel.y < 0) {
          if (primary_brush_size > 1) {
            primary_brush_size -= 1;
          }
        }
        break;
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

int coord_x, coord_y = 0;

i32 posToGridWithOffsetX(i32 x)
{
  i32 _x = (x - (WINDOW_WIDTH - GRID_WIDTH * pixel_size) / 2) / pixel_size;
  return _x;
}

i32 posToGridWithOffsetY(i32 y)
{
  i32 _y = (y - (WINDOW_HEIGHT - GRID_HEIGHT * pixel_size) / 2) / pixel_size;
  return _y;
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
          game->mouse.x > (WINDOW_WIDTH - GRID_WIDTH * pixel_size) / 2 &&
          game->mouse.x < (WINDOW_WIDTH + GRID_WIDTH * pixel_size) / 2 &&
          game->mouse.y > (WINDOW_HEIGHT - GRID_HEIGHT * pixel_size) / 2 &&
          game->mouse.y < (WINDOW_HEIGHT + GRID_HEIGHT * pixel_size) / 2;

      if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) {

        coord_x = posToGridWithOffsetX(game->mouse.x);
        coord_y = posToGridWithOffsetY(game->mouse.y);

        pixel_t pixel = {
          .col = coord_x,
          .row = coord_y,
          .size = pixel_size,
          .type = primary_pixel_type,
          .color = primary_color
        };

        if (is_in_bounds) {
          if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_E]) {
            pixel_buffer_remove_all(&pixel_buffer, coord_x, coord_y);
          } else {
            switch(primary_brush_type) {
              case BRUSH_RECT:
                pixel_buffer_add_rect(&pixel_buffer, pixel, primary_brush_size, primary_brush_size);
                break;
              case BRUSH_CIRCLE:
                pixel_buffer_add_circle(&pixel_buffer, pixel, primary_brush_size);
                break;
              case BRUSH_LINE:
                pixel_buffer_add_line(&pixel_buffer, pixel, primary_brush_size, 0);
                pixel_buffer_add_line(&pixel_buffer, pixel, primary_brush_size, 1);
                pixel_buffer_add_line(&pixel_buffer, pixel, primary_brush_size, 2);
                pixel_buffer_add_line(&pixel_buffer, pixel, primary_brush_size, 3);
                break;
              case BRUSH_PENCIL:
                pixel_buffer_add(&pixel_buffer, pixel);
              case BRUSH_ERASER:
                pixel_buffer_clear(&pixel_buffer);
              case BRUSH_RECT_OUTLINE:
                pixel_buffer_add_rect_outline(&pixel_buffer, pixel, primary_brush_size, primary_brush_size);
                break;
            }
          }
        }
      }

      // translate camera with arrow keys
      if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_UP] || SDL_GetKeyboardState(NULL)[SDL_SCANCODE_W]) {
        game->camera.y += pixel_size;
      } else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_DOWN] || SDL_GetKeyboardState(NULL)[SDL_SCANCODE_S]) {
        game->camera.y -= pixel_size;
      } else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LEFT] || SDL_GetKeyboardState(NULL)[SDL_SCANCODE_A]) {
        game->camera.x += pixel_size;
      } else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RIGHT] || SDL_GetKeyboardState(NULL)[SDL_SCANCODE_D]) {
        game->camera.x -= pixel_size;
      }

      if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LEFTBRACKET]) {
        if (pixel_size < 100) {
          pixel_size++;
        }
      } else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RIGHTBRACKET]) {
        if (pixel_size > 1) {
          pixel_size--;
        }
      }

      for (int i = 0; i < pixel_buffer.count; i++) {
        pixel_buffer.pixels[i].size = pixel_size;
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
        (WINDOW_WIDTH - GRID_WIDTH * pixel_size) / 2,
        (WINDOW_HEIGHT - GRID_HEIGHT * pixel_size) / 2,
        GRID_WIDTH * pixel_size,
        GRID_HEIGHT * pixel_size
      };

      // translate camera rect with camera
      canvas_rect.x += game->camera.x;
      canvas_rect.y += game->camera.y;

      // @background
      SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
      SDL_RenderFillRect(game->renderer, &canvas_rect);

      // @grid
      if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LSHIFT] || game->game_state.grid_enabled) {
        SDL_Color grid_color = C64_LIGHT_GREY;
        SDL_SetRenderDrawColor(game->renderer, grid_color.r, grid_color.g, grid_color.b, grid_color.a);

        for (int i = 0; i < GRID_WIDTH; i++) {
              SDL_RenderDrawLine(game->renderer,
                                 (WINDOW_WIDTH - GRID_WIDTH * pixel_size) / 2 + i * pixel_size + game->camera.x,
                                 (WINDOW_HEIGHT - GRID_HEIGHT * pixel_size) / 2 + game->camera.y,
                                 (WINDOW_WIDTH - GRID_WIDTH * pixel_size) / 2 + i * pixel_size + game->camera.x,
                                 (WINDOW_HEIGHT + GRID_HEIGHT * pixel_size) / 2 + game->camera.y);
          for (int j = 0; j < GRID_HEIGHT; j++) {
            SDL_RenderDrawLine(game->renderer,
                               (WINDOW_WIDTH - GRID_WIDTH * pixel_size) / 2 + game->camera.x,
                               (WINDOW_HEIGHT - GRID_HEIGHT * pixel_size) / 2 + j * pixel_size + game->camera.y,
                               (WINDOW_WIDTH + GRID_WIDTH * pixel_size) / 2 + game->camera.x,
                               (WINDOW_HEIGHT - GRID_HEIGHT * pixel_size) / 2 + j * pixel_size + game->camera.y);
          }
        }
      }

      pixel_buffer_draw(&pixel_buffer, &game->camera, game->renderer);

      SDL_RenderDrawRect(game->renderer, &canvas_rect);

      i32 x = posToGridWithOffsetX(game->mouse.x);
      i32 y = posToGridWithOffsetY(game->mouse.y);

      SDL_Rect rect = { x - GRID_CELL_SIZE / 2, y - GRID_CELL_SIZE / 2, GRID_CELL_SIZE, GRID_CELL_SIZE };

      rect.x = (WINDOW_WIDTH - GRID_WIDTH * GRID_CELL_SIZE) / 2 + x * GRID_CELL_SIZE;
      rect.y = (WINDOW_HEIGHT - GRID_HEIGHT * GRID_CELL_SIZE) / 2 + y * GRID_CELL_SIZE;

      SDL_Color cursor_color = C64_LIGHT_BLUE;
      SDL_SetRenderDrawColor(game->renderer, cursor_color.r, cursor_color.g, cursor_color.b, cursor_color.a);
      SDL_RenderDrawRect(game->renderer, &rect);

      char str[255];
      sprintf(str, "B:%d (%d, %d)", primary_brush_size, x, y);
      SDL_Point position = { WINDOW_WIDTH - dk_text_width(&game->ui_text, str), WINDOW_HEIGHT - dk_text_height(&game->ui_text, str) };
      SDL_Rect bg_rect = { position.x - 5, position.y - 5, dk_text_width(&game->ui_text, str) + 10, dk_text_height(&game->ui_text, str) + 10 };
      SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
      SDL_RenderFillRect(game->renderer, &bg_rect);
      dk_text_draw(&game->ui_text, str, position.x, position.y);

      static i32 icon_size = 32;
      static i32 icon_padding = 10;
      static i32 icon_pos_y = 10;
      static i32 icon_offset_x = 50;

      SDL_Rect rect10 = { (icon_offset_x + icon_padding), icon_pos_y, icons[ICON_GRID].rect.w, icons[ICON_GRID].rect.h };
      SDL_Color grid_icon_color = game->game_state.grid_enabled ? C64_LIGHT_GREEN : C64_WHITE;
      if(dk_ui_icon_button(game, rect10, grid_icon_color, icons[ICON_GRID].texture, &game->ui_focused)) {
        SDL_Delay(100);
        game->game_state.grid_enabled = !game->game_state.grid_enabled;
        SDL_Delay(0);
      }

      SDL_Rect rect7 = { (rect10.x + icon_size + icon_padding), icon_pos_y, icons[ICON_PLAY].rect.w, icons[ICON_PLAY].rect.h };
      SDL_Color play_icon_color = game->game_state.simulation_running ? C64_LIGHT_GREEN : C64_LIGHT_BLUE;
      if (dk_ui_icon_button(game, rect7, play_icon_color, icons[ICON_PLAY].texture, &game->ui_focused)) {
        game->game_state.simulation_running = true;
      }

      SDL_Rect rect11 = { rect7.x + icon_size + icon_padding, icon_pos_y, icons[ICON_PAUSE].rect.w, icons[ICON_PAUSE].rect.h };
      SDL_Color pause_icon_color = game->game_state.simulation_running ? C64_LIGHT_BLUE : C64_LIGHT_RED;
      if (dk_ui_icon_button(game, rect11, pause_icon_color, icons[ICON_PAUSE].texture, &game->ui_focused)) {
        game->game_state.simulation_running = false;
      }

      SDL_Rect rect9 = { rect11.x + icon_size + icon_padding, icon_pos_y, icons[ICON_CLEAR].rect.w, icons[ICON_CLEAR].rect.h };
      if (dk_ui_icon_button(game, rect9, C64_LIGHT_RED, icons[ICON_CLEAR].texture, &game->ui_focused)) {
        pixel_buffer_clear(&pixel_buffer);
      }

      SDL_Rect rect8 = { rect9.x + icon_size + icon_padding, icon_pos_y, icons[ICON_SAVE].rect.w, icons[ICON_SAVE].rect.h };
      if (dk_ui_icon_button(game, rect8, C64_LIGHT_GREEN, icons[ICON_SAVE].texture, &game->ui_focused)) {
        char filename[255];
        time_t t = time(NULL);

        struct tm tm = *localtime(&t);
        sprintf(filename, "pixsim-export-%d-%d-%d_%d-%d-%d.psb", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        pixel_buffer_save(&pixel_buffer, filename);

        char str[255];
        sprintf(str, "File Saved to %s", filename);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Saved!", (const char*)str, NULL);
      }

      SDL_Rect rect12 = { rect8.x + icon_size + icon_padding, icon_pos_y, icons[ICON_EXIT].rect.w, icons[ICON_EXIT].rect.h };
      if (dk_ui_icon_button(game, rect12, C64_LIGHT_RED, icons[ICON_EXIT].texture, &game->ui_focused)) {
        game->running = false;
      }

      static i32 size = 25;
      for (i32 i = 0; i < C64_COLOR_COUNT; i++) {

        SDL_Rect rect = { 0 };

        i32 x = i % 2;
        i32 y = i / 2;

        i32 padding = 5;

        rect.x = x * size + padding;
        rect.y = y * size + padding;

        rect.h = size;
        rect.w = size;

        char lab[255];
        sprintf(lab, "%s", " ");
        if(dk_ui_button(game, rect, C64_COLORS[i], lab, &game->ui_focused)) {
          primary_color = C64_COLORS[i];
        }
      }

      {
        i32 w = 100;
        i32 h = 20;

        for (i32 i = 0; i < PIXEL_TYPE_COUNT; i++) {
          SDL_Rect rect = { 0 };

          i32 padding = 5;

          char lab[255];
          if (i == PIXEL_TYPE_WATER) {
            sprintf(lab, "%s", "Water");
          }

          if (i == PIXEL_TYPE_SAND) {
            sprintf(lab, "%s", "Sand");
          }

          if (i == PIXEL_TYPE_FIRE) {
            sprintf(lab, "%s", "Fire");
          }

          SDL_Point position = { WINDOW_WIDTH - (i + 1) * (w + padding), 10 };

          rect.x = position.x;
          rect.y = position.y;

          rect.h = h;
          rect.w = w;

          SDL_Color color = C64_BLACK;
          if (i == primary_pixel_type) {
            color = C64_LIGHT_BLUE;
          }

          if (dk_ui_button(game, rect, color, lab, &game->ui_focused)) {
            primary_pixel_type = i;
          }
        }

      // draw brush types
      {
        i32 w = 100;
        i32 h = 20;

        for (i32 i = 0; i < BRUSH_COUNT; i++) {
          SDL_Rect rect = { 0 };

          i32 padding = 5;

          char lab[255];
          if (i == BRUSH_RECT) {
            sprintf(lab, "%s", "Rect");
          }

          if (i == BRUSH_CIRCLE) {
            sprintf(lab, "%s", "Circle");
          }

          if (i == BRUSH_LINE) {
            sprintf(lab, "%s", "Line");
          }

          if (i == BRUSH_RECT_OUTLINE) {
            sprintf(lab, "%s", "Outline");
          }

          SDL_Point position = { WINDOW_WIDTH - (i + 1) * (w + padding), 60 };

          rect.x = position.x;
          rect.y = position.y;

          rect.h = h;
          rect.w = w;

          SDL_Color color = C64_BLACK;
          if (i == primary_brush_type) {
            color = C64_LIGHT_BLUE;
          }

          if (dk_ui_button(game, rect, color, lab, &game->ui_focused)) {
            primary_brush_type = i;
          }
        }
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

