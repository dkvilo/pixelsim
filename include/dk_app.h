#if !defined(DK_APP_H)
#define DK_APP_H

#include <SDL2/SDL.h>
#include "dk_text.h"

typedef struct
{
  bool grid_enabled;
  bool simulation_running;
} app_state_t;

typedef struct
{
  int FrameCount;
  int FrameRate;
  int FrameTime;
  int DeltaTime;
  int _lastFrame;
  int _lastTime;
  int _currentTime;
} app_stats_t;

typedef struct
{
  int x;
  int y;
} app_mouse_t;

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
  app_state_t game_state;
  app_stats_t stats;
  app_mouse_t mouse;
} app_t;


#endif // DK_APP_H
