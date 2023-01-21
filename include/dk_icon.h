#if !defined(DK_ICON_H)
#define DK_ICON_H

#if !defined(SDL_h_)
#include <SDL2/SDL.h>
#endif

typedef struct
{
  SDL_Rect rect;
  SDL_Texture* texture;
} tileset_t;

typedef enum icon_type
{
  ICON_NONE = 0,
  ICON_PLAY,
  ICON_PAUSE,
  ICON_GRID,
  ICON_EXIT,
  ICON_SAVE,
  ICON_CLEAR,
  ICON_COPY_BUFFER,
  ICON_PASTE_BUFFER,
  ICON_ZOOM_IN,
  ICON_ZOOM_OUT,
  ICON_NUMBER0,
  ICON_NUMBER1,
  ICON_NUMBER2,
  ICON_NUMBER3,
  ICON_NUMBER4,
  ICON_NUMBER5,
  ICON_NUMBER6,
  ICON_NUMBER7,
  ICON_NUMBER8,
  ICON_NUMBER9,
  ICON_BRUSH_CROSS,
  ICON_BRUSH_RECT,
  ICON_BRUSH_CIRCLE,
  ICON_BRUSH_RECT_OUTLINE,
  ICON_PIXEL_TYPE_WATER,
  ICON_PIXEL_TYPE_SAND,
  ICON_PIXEL_TYPE_FIRE,
  ICON_COUNT
} icon_type_t;

typedef struct
{
  SDL_Rect rect;
  SDL_Texture* texture;
  icon_type_t type;
} icon_t;

typedef struct {
  SDL_Rect rect;
  SDL_Texture* texture;
  icon_type_t type;
  i32 x;
  i32 y;
} dk_image_t;

dk_image_t
icon_load__texture(SDL_Renderer* renderer, char* path);

void
load_tileset(SDL_Renderer* renderer, tileset_t* tileset, char* path);

void
get_icon_from_tileset(SDL_Renderer* renderer, tileset_t* tileset, icon_t* icon, SDL_Point coords);

void
draw_tileset(SDL_Renderer* renderer, tileset_t* tileset);

#if defined(DK_ICON_IMPLEMENTATION)

dk_image_t
icon_load__texture(SDL_Renderer* renderer, char* path)
{
  SDL_Surface* surface = IMG_Load(path);
  dk_image_t image;

  image.texture = SDL_CreateTextureFromSurface(renderer, surface);
  image.rect.w = surface->w;
  image.rect.h = surface->h;
  image.rect.x = 0;
  image.rect.y = 0;

  SDL_FreeSurface(surface);
  return image;
}

void
load_tileset(SDL_Renderer* renderer, tileset_t* tileset, char* path)
{
  dk_image_t image = icon_load__texture(renderer, path);
  tileset->texture = image.texture;
  tileset->rect.w = image.rect.w;
  tileset->rect.h = image.rect.h;

  tileset->rect.x = 0;
  tileset->rect.y = 0;
}

void get_icon_from_tileset(SDL_Renderer* renderer, tileset_t* tileset, icon_t* icon, SDL_Point coords)
{

  // icons are 16x16
  icon->rect.w = 16;
  icon->rect.h = 16;

  // calculate the x and y position of the icon
  icon->rect.x = coords.x * 16;
  icon->rect.y = coords.y * 16;

  // create a new texture
  icon->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 16, 16);

  // set the render target to the new texture
  SDL_SetRenderTarget(renderer, icon->texture);

  // copy the icon from the tileset to the new texture
  SDL_RenderCopy(renderer, tileset->texture, &icon->rect, NULL);

  // set the render target back to the screen
  SDL_SetRenderTarget(renderer, NULL);

  SDL_SetTextureBlendMode(icon->texture, SDL_BLENDMODE_BLEND);

  // Scaling up to 32x32
  icon->rect.w = 32;
  icon->rect.h = 32;

  icon->rect.x = 0;
  icon->rect.y = 0;
}

void draw_tileset(SDL_Renderer* renderer, tileset_t* tileset)
{

  SDL_RenderCopy(renderer, tileset->texture, NULL, &tileset->rect);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  for (int i = 0; i < 32; i++)
  {
    SDL_RenderDrawLine(renderer, i * 16, 0, i * 16, tileset->rect.h);
    SDL_RenderDrawLine(renderer, 0, i * 16, tileset->rect.w, i * 16);
  }
}

#endif // DK_ICON_IMPLEMENTATION


#endif // DK_ICON_H
