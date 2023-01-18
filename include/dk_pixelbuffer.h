#if !defined(DK_PIXELBUFFER_H)
#define DK_PIXELBUFFER_H

#include <assert.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "dk_app.h"
#include "dk_macros.h"

#if !defined(__DK_COLOR_H__)
#include "dk_color.h"
#endif

#if !defined(__DK_H__)
#include "dk.h"
#endif

typedef enum {
  PIXEL_TYPE_WATER,
  PIXEL_TYPE_SAND,
  PIXEL_TYPE_FIRE,
  PIXEL_TYPE_COUNT
} pixel_type_t;

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

SDL_Color
pixel_type_to_color(pixel_type_t type);

void
pixel_buffer_save_png(pixel_buffer_t* buffer, const char* filename, u32 scale);

void
pixel_buffer_init(pixel_buffer_t* buffer);

void
pixel_buffer_add(pixel_buffer_t* buffer, pixel_t pixel);

void
pixel_buffer_clear(pixel_buffer_t* buffer);

void
pixel_buffer_remove(pixel_buffer_t* buffer, u32 index);

void
pixel_buffer_remove_all(pixel_buffer_t* buffer, u32 col, u32 row);

void
pixel_buffer_draw(pixel_buffer_t* buffer, app_camera_t* camera, SDL_Renderer* renderer);

void
png_to_pixel_buffer(pixel_buffer_t* buffer, const char* filename, int scale);

void
pixel_buffer_save(pixel_buffer_t* buffer, const char* filename);

void
pixel_buffer_load(pixel_buffer_t* buffer, const char* filename);

void
update_pixel_simulation(pixel_buffer_t* buffer);

void
pixel_buffer_add_circle(pixel_buffer_t* buffer, pixel_t pixel, u32 radius);

void
pixel_buffer_add_line(pixel_buffer_t* buffer, pixel_t pixel, u32 length, u32 direction);

void
pixel_buffer_add_rect(pixel_buffer_t* buffer, pixel_t pixel, u32 width, u32 height);

void
pixel_buffer_add_rect_outline(pixel_buffer_t* buffer, pixel_t pixel, u32 width, u32 height);


#endif // DK_PIXELBUFFER_H
