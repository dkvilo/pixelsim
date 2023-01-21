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

#define DK_PIXELBUFFER_IMPLEMENTATION
#include "dk_pixelbuffer.h"

#include "dk_app.h"
#include "dk_macros.h"

#define DK_ICON_IMPLEMENTATION
#include "dk_icon.h"

#define DK_UI_IMPLEMENTATION
#include "dk_ui.h"

#define DK_CLIPBOARD_IMPLEMENTATION
#include "dk_clipboard.h"

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

static tileset_t* tileset = NULL;

static icon_t *icons = NULL;
SDL_Color* DK_PALLETE = NULL;

SDL_Color primary_color = C64_BROWN;
SDL_Color secondary_color = C64_BLACK;

u8 pixel_size = GRID_CELL_SIZE;

pixel_type_t primary_pixel_type = PIXEL_TYPE_WATER;

BrushType primary_brush_type = BRUSH_CIRCLE;
i32 primary_brush_size = 2;

static const int frame_count = 9;
pixel_buffer_t* frames;
int active_frame_buffer_index = 0;

dk_clipboard_t* clipboard = NULL;

void
game_init(app_t* game)
{

  tileset = (tileset_t*) malloc(sizeof(tileset_t));

  icons = (icon_t*) malloc(sizeof(icon_t) * ICON_COUNT);
  memset(icons, 0, sizeof(icon_t) * ICON_COUNT);

  DK_PALLETE = (SDL_Color*) malloc(sizeof(SDL_Color) * C64_COLOR_COUNT);
  load_colors(DK_PALLETE);

  game->state = IN_GAME;
  game->raw_text_input_buffer = malloc(sizeof(char) * 1080);

  game->game_state = (app_state_t) { .grid_enabled = true, .simulation_running = false };

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

  load_tileset(game->renderer, tileset, "assets/icons/tileset.png");

  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_GRID], (SDL_Point){7, 2});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_CLEAR], (SDL_Point){9, 8});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_SAVE], (SDL_Point){11, 1});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_EXIT], (SDL_Point){11, 0});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_PLAY], (SDL_Point){1, 0});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_PAUSE], (SDL_Point){0, 0});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_ZOOM_IN], (SDL_Point){3, 6});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_ZOOM_OUT], (SDL_Point){4, 6});

  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_NUMBER1], (SDL_Point){13, 6});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_NUMBER2], (SDL_Point){14, 6});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_NUMBER3], (SDL_Point){15, 6});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_NUMBER4], (SDL_Point){16, 6});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_NUMBER5], (SDL_Point){9, 7});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_NUMBER6], (SDL_Point){10, 7});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_NUMBER7], (SDL_Point){11, 7});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_NUMBER8], (SDL_Point){12, 7});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_NUMBER9], (SDL_Point){13, 7});

  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_BRUSH_CROSS], (SDL_Point){0, 2});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_BRUSH_RECT], (SDL_Point){15, 3});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_BRUSH_CIRCLE], (SDL_Point){13, 2});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_BRUSH_RECT_OUTLINE], (SDL_Point){3, 7});

  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_PIXEL_TYPE_WATER], (SDL_Point){7, 15});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_PIXEL_TYPE_SAND], (SDL_Point){7, 15});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_PIXEL_TYPE_FIRE], (SDL_Point){7, 15});

  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_COPY_BUFFER], (SDL_Point){4, 3});
  get_icon_from_tileset(game->renderer, tileset, &icons[ICON_PASTE_BUFFER], (SDL_Point){15, 0});


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

  frames = (pixel_buffer_t*) malloc(sizeof(pixel_buffer_t) * frame_count);
  for (int i = 0; i < frame_count; i++) {
    pixel_buffer_init(&frames[i]);
  }

  clipboard = (pixel_buffer_t*) malloc(sizeof(pixel_buffer_t));
  memset(clipboard, 0, sizeof(pixel_buffer_t));

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
          pixel_buffer_load(&frames[active_frame_buffer_index], dropped_filedir);
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
          case SDLK_1:
            active_frame_buffer_index = 0;
            break;
          case SDLK_2:
            active_frame_buffer_index = 1;
            break;
          case SDLK_3:
            active_frame_buffer_index = 2;
            break;
          case SDLK_4:
            active_frame_buffer_index = 3;
            break;
          case SDLK_5:
            active_frame_buffer_index = 4;
            break;
          case SDLK_6:
            active_frame_buffer_index = 5;
            break;
          case SDLK_7:
            active_frame_buffer_index = 6;
            break;
          case SDLK_8:
            active_frame_buffer_index = 7;
            break;
          case SDLK_9:
            active_frame_buffer_index = 8;
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

        if (is_in_bounds && !game->ui_focused) {
          if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_E]) {
            pixel_buffer_remove_all(&frames[active_frame_buffer_index], coord_x, coord_y);
          } else {
            switch(primary_brush_type) {
              case BRUSH_RECT:
                pixel_buffer_add_rect(&frames[active_frame_buffer_index], pixel, primary_brush_size, primary_brush_size);
                break;
              case BRUSH_CIRCLE:
                pixel_buffer_add_circle(&frames[active_frame_buffer_index], pixel, primary_brush_size);
                break;
              case BRUSH_LINE:
                pixel_buffer_add_line(&frames[active_frame_buffer_index], pixel, primary_brush_size, 0);
                pixel_buffer_add_line(&frames[active_frame_buffer_index], pixel, primary_brush_size, 1);
                pixel_buffer_add_line(&frames[active_frame_buffer_index], pixel, primary_brush_size, 2);
                pixel_buffer_add_line(&frames[active_frame_buffer_index], pixel, primary_brush_size, 3);
                break;
              case BRUSH_PENCIL:
                pixel_buffer_add(&frames[active_frame_buffer_index], pixel);
              case BRUSH_ERASER:
                pixel_buffer_clear(&frames[active_frame_buffer_index]);
              case BRUSH_RECT_OUTLINE:
                pixel_buffer_add_rect_outline(&frames[active_frame_buffer_index], pixel, primary_brush_size, primary_brush_size);
                break;
            }
          }
        }
      }

#if 0
      if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_UP] || SDL_GetKeyboardState(NULL)[SDL_SCANCODE_W]) {
        game->camera.y += pixel_size;
      } else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_DOWN] || SDL_GetKeyboardState(NULL)[SDL_SCANCODE_S]) {
        game->camera.y -= pixel_size;
      } else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LEFT] || SDL_GetKeyboardState(NULL)[SDL_SCANCODE_A]) {
        game->camera.x += pixel_size;
      } else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RIGHT] || SDL_GetKeyboardState(NULL)[SDL_SCANCODE_D]) {
        game->camera.x -= pixel_size;
      }
#endif

      if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LEFTBRACKET]) {
        if (pixel_size < 100) {
          pixel_size++;
        }
      } else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RIGHTBRACKET]) {
        if (pixel_size > 1) {
          pixel_size--;
        }
      }

      for (int i = 0; i < frames[active_frame_buffer_index].count; i++) {
        frames[active_frame_buffer_index].pixels[i].size = pixel_size;
      }

      if (game->game_state.simulation_running) {
        update_pixel_simulation(&frames[active_frame_buffer_index]);
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

      pixel_buffer_draw(&frames[active_frame_buffer_index], &game->camera, game->renderer);

      SDL_RenderDrawRect(game->renderer, &canvas_rect);
      i32 x = posToGridWithOffsetX(game->mouse.x);
      i32 y = posToGridWithOffsetY(game->mouse.y);

      SDL_Rect rect = { x - pixel_size / 2, y - pixel_size / 2, pixel_size, pixel_size };

      rect.x = (WINDOW_WIDTH - GRID_WIDTH * pixel_size) / 2 + x * pixel_size;
      rect.y = (WINDOW_HEIGHT - GRID_HEIGHT * pixel_size) / 2 + y * pixel_size;

      bool is_in_bounds =
        game->mouse.x > (WINDOW_WIDTH - GRID_WIDTH * pixel_size) / 2 &&
        game->mouse.x < (WINDOW_WIDTH + GRID_WIDTH * pixel_size) / 2 &&
        game->mouse.y > (WINDOW_HEIGHT - GRID_HEIGHT * pixel_size) / 2 &&
        game->mouse.y < (WINDOW_HEIGHT + GRID_HEIGHT * pixel_size) / 2;

      if (is_in_bounds) {
        SDL_Color cursor_color = C64_LIGHT_BLUE;
        SDL_SetRenderDrawColor(game->renderer, cursor_color.r, cursor_color.g, cursor_color.b, cursor_color.a);
        SDL_RenderDrawRect(game->renderer, &rect);
      }

      // black panel on bottom
      {
        SDL_Rect rect = { 0 };
        rect.x = 0;
        rect.y = WINDOW_HEIGHT - 40;
        rect.w = WINDOW_WIDTH;
        rect.h = 40;

        SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(game->renderer, &rect);
      }
      {
        char str[255];
        sprintf(str, "B:%d (%d, %d)", primary_brush_size, x, y);
        SDL_Point position = { WINDOW_WIDTH - dk_text_width(&game->ui_text, str), WINDOW_HEIGHT - dk_text_height(&game->ui_text, str) - 10 };
        dk_text_draw(&game->ui_text, str, position.x, position.y);
      }

      {
        SDL_Rect rect = { 0 };
        rect.x = 10;
        rect.y = WINDOW_HEIGHT - 35;
        rect.w = 35;
        rect.h = 35;
        if(dk_ui_icon_button(game, rect, C64_WHITE, icons[ICON_ZOOM_IN].texture, &game->ui_focused)) {
          if (primary_brush_size < 20) primary_brush_size++;
        }
      }

      {
        SDL_Rect rect = { 0 };
        rect.x = 60;
        rect.y = WINDOW_HEIGHT - 35;
        rect.w = 35;
        rect.h = 35;
        if(dk_ui_icon_button(game, rect, C64_WHITE, icons[ICON_ZOOM_OUT].texture, &game->ui_focused)) {
          if (primary_brush_size > 1) primary_brush_size--;
        }
      }

      static i32 icon_size = 32;
      static i32 icon_padding = 10;
      static i32 icon_pos_y = 10;
      static i32 icon_offset_x = 0;

      // draw panel for icons
      SDL_Rect panel_rect = { 0, 0, WINDOW_WIDTH, icon_size + icon_padding * 2 };
      SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
      SDL_RenderFillRect(game->renderer, &panel_rect);

      SDL_Rect rect10 = { (icon_offset_x + icon_padding), icon_pos_y, icons[ICON_GRID].rect.w, icons[ICON_GRID].rect.h };
      SDL_Color grid_icon_color = game->game_state.grid_enabled ? C64_LIGHT_GREEN : C64_WHITE;
      SDL_PollEvent(&game->event);
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
        pixel_buffer_clear(&frames[active_frame_buffer_index]);
      }

      SDL_Rect rect8 = { rect9.x + icon_size + icon_padding, icon_pos_y, icons[ICON_SAVE].rect.w, icons[ICON_SAVE].rect.h };
      if (dk_ui_icon_button(game, rect8, C64_LIGHT_GREEN, icons[ICON_SAVE].texture, &game->ui_focused)) {
        char filename[255];
        time_t t = time(NULL);

        struct tm tm = *localtime(&t);
        sprintf(filename, "pixsim-export-%d-%d-%d_%d-%d-%d.psb", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        pixel_buffer_save(&frames[active_frame_buffer_index], filename);

        char str[512];
        sprintf(str, "File Saved to %s", filename);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Saved!", (const char*)str, NULL);
      }

      SDL_Rect rect12 = { rect8.x + icon_size + icon_padding, icon_pos_y, icons[ICON_EXIT].rect.w, icons[ICON_EXIT].rect.h };
      if (dk_ui_icon_button(game, rect12, C64_LIGHT_RED, icons[ICON_EXIT].texture, &game->ui_focused)) {

        SDL_MessageBoxButtonData buttons[] = {
          { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Yes" },
          { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "No" },
        };

        SDL_MessageBoxData messageboxdata = {
          SDL_MESSAGEBOX_INFORMATION,
          NULL,
          "Exit",
          "Are you sure you want to exit?",
          SDL_arraysize(buttons),
          buttons,
          NULL
        };

        int buttonid;
        if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
          SDL_Log("error displaying message box");
          game->running = false;
        }

        if (buttonid == 0) {

          // Ask to save
          SDL_MessageBoxButtonData buttons[] = {
            { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Yes" },
            { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "No" },
          };

          SDL_MessageBoxData messageboxdata = {
            SDL_MESSAGEBOX_INFORMATION,
            NULL,
            "Save",
            "Do you want to save before exiting?",
            SDL_arraysize(buttons),
            buttons,
            NULL
          };

          int buttonid;
          if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
            SDL_Log("error displaying message box");
            game->running = false;
          }

          if (buttonid == 0) {
            char filename[255];
            time_t t = time(NULL);

            struct tm tm = *localtime(&t);
            sprintf(filename, "pixsim-export-%d-%d-%d_%d-%d-%d.psb", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
            pixel_buffer_save(&frames[active_frame_buffer_index], filename);

            char str[512];
            sprintf(str, "File Saved to %s", filename);
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Saved!", (const char*)str, NULL);
          }

          game->running = false;

        }
      }

      // COPY BUTTON
      SDL_Rect rect13 = { rect12.x + icon_size + icon_padding, icon_pos_y, icons[ICON_COPY_BUFFER].rect.w, icons[ICON_COPY_BUFFER].rect.h };
      if (dk_ui_icon_button(game, rect13, C64_LIGHT_BLUE, icons[ICON_COPY_BUFFER].texture, &game->ui_focused)) {
        dk_clipboard_set(clipboard, &frames[active_frame_buffer_index]);
      }

      // PASTE BUTTON
      SDL_Rect rect14 = { rect13.x + icon_size + icon_padding, icon_pos_y, icons[ICON_PASTE_BUFFER].rect.w, icons[ICON_PASTE_BUFFER].rect.h };
      if (dk_ui_icon_button(game, rect14, C64_LIGHT_BLUE, icons[ICON_PASTE_BUFFER].texture, &game->ui_focused)) {
        dk_clipboard_paste_to_buffer(clipboard, &frames[active_frame_buffer_index]);
      }

      // tooltip for save button
      {
        bool is_visible = false;
        dk_ui_tooltip(game, rect8, "Save Buffer", &is_visible);
      }

      // tooltip for clear button
      {
        bool is_visible = false;
        dk_ui_tooltip(game, rect9, "Clear Canvas", &is_visible);
      }

      // tooltip for pause button
      {
        bool is_visible = false;
        dk_ui_tooltip(game, rect11, "Stop Simulation", &is_visible);
      }

      // tooltip for play button
      {
        bool is_visible = false;
        dk_ui_tooltip(game, rect7, "Play Simulation", &is_visible);
      }

      // tooltip for exit button
      {
        bool is_visible = false;
        dk_ui_tooltip(game, rect12, "Quit Program", &is_visible);
      }

      // tooltip for pixel type
      {
        bool is_visible = false;
        char* str = "Enable/Disable Grid";
        dk_ui_tooltip(game, rect10, str, &is_visible);
      }

      // tooltip for copy button
      {
        bool is_visible = false;
        char* str = "Copy Buffer";
        dk_ui_tooltip(game, rect13, str, &is_visible);
      }

      // tooltip for paste button
      {
        bool is_visible = false;
        char* str = "Paste In Buffer";
        dk_ui_tooltip(game, rect14, str, &is_visible);
      }

      static i32 size = 25;
      for (i32 i = 0; i < C64_COLOR_COUNT; i++) {

        SDL_Rect rect = { 0 };

        i32 x = i % 2;
        i32 y = i / 2;

        i32 padding = 5;

        rect.x = x * size + padding;
        rect.y = y * size + padding + icon_size + icon_padding * 2 + 45;

        rect.h = size;
        rect.w = size;

        char lab[255];
        sprintf(lab, "%s", " ");
        if(dk_ui_button(game, rect, C64_COLORS[i], lab, &game->ui_focused)) {
          primary_color = C64_COLORS[i];
        }
      }

      char* version = "v0.1";
      int version_width = dk_text_width(&game->ui_text, version);
      int version_height = dk_text_height(&game->ui_text, version);
      SDL_Rect aboutMe = { WINDOW_WIDTH / 2 - version_width / 2, WINDOW_HEIGHT - version_height, version_width, version_height};
      dk_ui_button(game, aboutMe, C64_BLACK, "v0.1", &game->ui_focused);

      int brush_icon_margin = 10;
      // draw circle brush icon
      {
        SDL_Rect rect = { 0 };

        rect.x = WINDOW_WIDTH - 32 - brush_icon_margin;
        rect.y = 10;

        rect.h = 32;
        rect.w = 32;

        SDL_Color color = C64_WHITE;
        if (primary_brush_type == BRUSH_CIRCLE) {
          color = C64_LIGHT_GREEN;
        }

        if (dk_ui_icon_button(game, rect, color, icons[ICON_BRUSH_CIRCLE].texture, &game->ui_focused)) {
          primary_brush_type = BRUSH_CIRCLE;
        }
      }

      // draw line brush icon
      {
        SDL_Rect rect = { 0 };

        rect.x = WINDOW_WIDTH - 74 - brush_icon_margin;
        rect.y = 10;

        rect.h = 32;
        rect.w = 32;

        SDL_Color color = C64_WHITE;
        if (primary_brush_type == BRUSH_LINE) {
          color = C64_LIGHT_GREEN;
        }

        if (dk_ui_icon_button(game, rect, color, icons[ICON_BRUSH_CROSS].texture, &game->ui_focused)) {
          primary_brush_type = BRUSH_LINE;
        }
      }

      // draw rect brush iocn
      {
        SDL_Rect rect = { 0 };

        rect.x = WINDOW_WIDTH - 116 - brush_icon_margin;
        rect.y = 10;

        rect.h = 32;
        rect.w = 32;

        SDL_Color color = C64_WHITE;
        if (primary_brush_type == BRUSH_RECT) {
          color = C64_LIGHT_GREEN;
        }

        if (dk_ui_icon_button(game, rect, color, icons[ICON_BRUSH_RECT].texture, &game->ui_focused)) {
          primary_brush_type = BRUSH_RECT;
        }
      }

      // draw rect brush iocn
      {
        SDL_Rect rect = { 0 };

        rect.x = WINDOW_WIDTH - 158 - brush_icon_margin;
        rect.y = 10;

        rect.h = 32;
        rect.w = 32;

        SDL_Color color = C64_WHITE;
        if (primary_brush_type == BRUSH_RECT_OUTLINE) {
          color = C64_LIGHT_GREEN;
        }

        if (dk_ui_icon_button(game, rect, color, icons[ICON_BRUSH_RECT_OUTLINE].texture, &game->ui_focused)) {
          primary_brush_type = BRUSH_RECT_OUTLINE;
        }
      }

      for (int i = 0; i < frame_count; ++i) {
        SDL_Rect rect = { 0 };
        rect.w = 32;
        rect.h = 32;
        rect.x = 140;
        rect.y = 100 + i * 40;

        int icon_offset = ((int)ICON_NUMBER0 + (i + 1  ));
        if (i == active_frame_buffer_index) {
          SDL_Color color = C64_LIGHT_BLUE;
          SDL_SetRenderDrawColor(game->renderer, color.r, color.g, color.b, color.a);
          SDL_RenderFillRect(game->renderer, &rect);
        }

        if(dk_ui_icon_button(game, rect, C64_ORANGE, icons[icon_offset].texture, &game->ui_focused)) {
          active_frame_buffer_index = i;
        }
      }

      for (int i = 0; i < 3; ++i) {
        SDL_Rect rect = { 0 };

        rect.x = WINDOW_WIDTH - 32 - 140;
        rect.y = 100 + i * 40;

        rect.h = 32;
        rect.w = 32;

        SDL_Color color = pixel_type_to_color(i);
        if (i == primary_pixel_type) {
          SDL_Color bg_color = C64_LIGHT_BLUE;
          SDL_SetRenderDrawColor(game->renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
          SDL_RenderFillRect(game->renderer, &rect);
        }

        int icon_offset = ((int)ICON_BRUSH_RECT_OUTLINE + (i + 1));
        if (dk_ui_icon_button(game, rect, color, icons[icon_offset].texture, &game->ui_focused)) {
          primary_pixel_type = i;
        }
      }

      // @dev @tileset
      if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_SPACE]) {

        game->ui_focused = true;

        SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
        SDL_RenderClear(game->renderer);

        draw_tileset(game->renderer, tileset);

        const int TILE_SIZE = 16;
        SDL_Point mouse_tile_pos = { game->mouse.x / TILE_SIZE, game->mouse.y / TILE_SIZE };
        SDL_Rect tile_rect = { mouse_tile_pos.x * TILE_SIZE, mouse_tile_pos.y * TILE_SIZE, TILE_SIZE, TILE_SIZE };

        SDL_Color color = C64_LIGHT_BLUE;
        SDL_SetRenderDrawColor(game->renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawRect(game->renderer, &tile_rect);

        // selected tile the center of the screen
        SDL_Rect selected_tile_rect = { WINDOW_WIDTH / 2 - TILE_SIZE / 2, WINDOW_HEIGHT / 2 - TILE_SIZE / 2, TILE_SIZE, TILE_SIZE };

        SDL_SetTextureBlendMode(tileset->texture, SDL_BLENDMODE_BLEND);

        // making the copy of selected tile to another texture
        SDL_Texture* selected_tile_texture = SDL_CreateTexture(game->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, TILE_SIZE, TILE_SIZE);
        SDL_SetRenderTarget(game->renderer, selected_tile_texture);
        SDL_RenderCopy(game->renderer, tileset->texture, &tile_rect, NULL);

        // selected tile to screen
        SDL_SetRenderTarget(game->renderer, NULL);

        SDL_SetTextureBlendMode(selected_tile_texture, SDL_BLENDMODE_BLEND);

        selected_tile_rect.x = tileset->rect.w + TILE_SIZE;
        selected_tile_rect.y = 0;
        selected_tile_rect.w = TILE_SIZE * 8;
        selected_tile_rect.h = TILE_SIZE * 8;

        // border around selected tile
        SDL_SetRenderDrawColor(game->renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawRect(game->renderer, &selected_tile_rect);

        SDL_RenderCopy(game->renderer, selected_tile_texture, NULL, &selected_tile_rect);

        char tileset_coords_str[255];
        sprintf(tileset_coords_str, "(%d, %d)", mouse_tile_pos.x, mouse_tile_pos.y);
        dk_text_draw(&game->ui_text, tileset_coords_str, selected_tile_rect.x, selected_tile_rect.y + selected_tile_rect.h + 10);

        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_C]) {
          SDL_SetRenderTarget(game->renderer, NULL);
          SDL_SetTextureBlendMode(selected_tile_texture, SDL_BLENDMODE_BLEND);
          SDL_SetTextureScaleMode(selected_tile_texture, SDL_ScaleModeNearest);

          SDL_Rect selected_tile_rect = { 0, 0, TILE_SIZE, TILE_SIZE };
          SDL_RenderCopy(game->renderer, selected_tile_texture, NULL, &selected_tile_rect);

          SDL_Surface* surface = SDL_CreateRGBSurface(0, TILE_SIZE, TILE_SIZE, 32, 0, 0, 0, 0);
          SDL_RenderReadPixels(game->renderer, &selected_tile_rect, SDL_PIXELFORMAT_RGBA8888, surface->pixels, surface->pitch);

          char clipboard_path[255];
          sprintf(clipboard_path, "%d-%d-%s", mouse_tile_pos.x, mouse_tile_pos.y, "exported.bmp");
          SDL_SaveBMP(surface, clipboard_path);
          SDL_FreeSurface(surface);
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

