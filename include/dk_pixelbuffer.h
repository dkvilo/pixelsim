#if !defined(DK_PIXELBUFFER_H)
#define DK_PIXELBUFFER_H

#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "dk_app.h"
#include "dk_macros.h"
#include "dk_color.h"
#include "dk.h"

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
pixel_buffer_merge(pixel_buffer_t* buffer, pixel_buffer_t* buffer2);

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

pixel_t
pixel_buffer_get_pixel(pixel_buffer_t* buffer, u32 col, u32 row);

void
pixel_buffer_set_pixel(pixel_buffer_t* buffer, pixel_t pixel);

#if defined(DK_PIXELBUFFER_IMPLEMENTATION)

void
pixel_buffer_init(pixel_buffer_t* buffer)
{
  buffer->pixels = NULL;
  buffer->count = 0;
}

void
pixel_buffer_merge(pixel_buffer_t* buffer, pixel_buffer_t* buffer2)
{
  for (u32 i = 0; i < buffer2->count; i++) {
    pixel_buffer_add(buffer, buffer2->pixels[i]);
  }
}

void
pixel_buffer_add(pixel_buffer_t* buffer, pixel_t pixel)
{

  assert(buffer != NULL);
  if (pixel.col >= GRID_WIDTH || pixel.row >= GRID_HEIGHT) {
    return;
  }

  for (u32 i = 0; i < buffer->count; i++) {
    if (buffer->pixels[i].col == pixel.col && buffer->pixels[i].row == pixel.row
    && buffer->pixels[i].color.r == pixel.color.r && buffer->pixels[i].color.g == pixel.color.g && buffer->pixels[i].color.b == pixel.color.b && buffer->pixels[i].color.a == pixel.color.a && buffer->pixels[i].type == pixel.type
    ) {
      return;
    }
  }

  buffer->pixels = realloc(buffer->pixels, sizeof(pixel_t) * (buffer->count + 1));
  buffer->pixels[buffer->count] = pixel;
  buffer->count++;
}

void
pixel_buffer_add_circle(pixel_buffer_t* buffer, pixel_t pixel, u32 radius)
{
  for (u32 i = 0; i < radius; i++) {
    for (u32 j = 0; j < radius; j++) {
      if (i * i + j * j <= radius * radius) {
        pixel_t p = pixel;
        p.col += i;
        p.row += j;
        pixel_buffer_add(buffer, p);

        p = pixel;
        p.col -= i;
        p.row += j;
        pixel_buffer_add(buffer, p);

        p = pixel;
        p.col += i;
        p.row -= j;
        pixel_buffer_add(buffer, p);

        p = pixel;
        p.col -= i;
        p.row -= j;
        pixel_buffer_add(buffer, p);
      }
    }
  }
}

void
pixel_buffer_add_line(pixel_buffer_t* buffer, pixel_t pixel, u32 length, u32 direction)
{
  for (u32 i = 0; i < length; i++) {
    pixel_t p = pixel;
    switch (direction) {
      case 0: p.col += i; break;
      case 1: p.row += i; break;
      case 2: p.col -= i; break;
      case 3: p.row -= i; break;
    }
    pixel_buffer_add(buffer, p);
  }
}

void
pixel_buffer_add_rect(pixel_buffer_t* buffer, pixel_t pixel, u32 width, u32 height)
{
  for (u32 i = 0; i < width; i++) {
    for (u32 j = 0; j < height; j++) {
      pixel_t p = pixel;
      p.col += i - width / 2;
      p.row += j - height / 2;
      pixel_buffer_add(buffer, p);
    }
  }
}

void
pixel_buffer_add_rect_outline(pixel_buffer_t* buffer, pixel_t pixel, u32 width, u32 height)
{
  for (u32 i = 0; i <= width; i++) {
    pixel_t p = pixel;
    p.col += i - width / 2;
    p.row -= height / 2;
    pixel_buffer_add(buffer, p);

    p = pixel;
    p.col += i - width / 2;
    p.row += height / 2;
    pixel_buffer_add(buffer, p);
  }

  for (u32 i = 0; i < height; i++) {
    pixel_t p = pixel;
    p.col -= width / 2;
    p.row += i - height / 2;
    pixel_buffer_add(buffer, p);

    p = pixel;
    p.col += width / 2;
    p.row += i - height / 2;
    pixel_buffer_add(buffer, p);
  }
}

void
pixel_buffer_clear(pixel_buffer_t* buffer)
{
  free(buffer->pixels);
  buffer->pixels = NULL;
  buffer->count = 0;
}

void
pixel_buffer_remove(pixel_buffer_t* buffer, u32 index)
{
  if (index < buffer->count) {
    buffer->pixels[index] = buffer->pixels[buffer->count - 1];
    buffer->count--;
    buffer->pixels = realloc(buffer->pixels, sizeof(pixel_t) * buffer->count);
  }
}

void
pixel_buffer_remove_all(pixel_buffer_t* buffer, u32 col, u32 row)
{
  for (u32 i = 0; i < buffer->count; i++) {
    if (buffer->pixels[i].col == col && buffer->pixels[i].row == row) {
      pixel_buffer_remove(buffer, i);
      i--;
    }
  }
}

void
pixel_buffer_draw(pixel_buffer_t* buffer, app_camera_t* camera, SDL_Renderer* renderer)
{
  for (u32 i = 0; i < buffer->count; i++) {
    SDL_Rect rect = {
      .x = buffer->pixels[i].col * buffer->pixels[i].size,
      .y = buffer->pixels[i].row * buffer->pixels[i].size,
      .w = buffer->pixels[i].size,
      .h = buffer->pixels[i].size,
    };

    rect.x += (WINDOW_WIDTH - GRID_WIDTH * buffer->pixels[i].size) / 2;
    rect.y += (WINDOW_HEIGHT - GRID_HEIGHT * buffer->pixels[i].size) / 2;

    rect.x += camera->x;
    rect.y += camera->y;

    SDL_Color color = buffer->pixels[i].color;
#if 0
    if (buffer->pixels[i].row < GRID_HEIGHT) {
      color.r = (u8)((f32)color.r * (f32)buffer->pixels[i].row / (f32)(GRID_HEIGHT));
      color.g = (u8)((f32)color.g * (f32)buffer->pixels[i].row / (f32)(GRID_HEIGHT));
      color.b = (u8)((f32)color.b * (f32)buffer->pixels[i].row / (f32)(GRID_HEIGHT));
    }
#endif

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
  }
}

void
pixel_buffer_save_png(pixel_buffer_t* buffer, const char* filename, u32 scale)
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

    SDL_Color color = pixel.color;
    SDL_FillRect(surface, &rect, SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a));
  }

  IMG_SavePNG(surface, filename);
  SDL_FreeSurface(surface);
}

pixel_t
pixel_buffer_get_pixel(pixel_buffer_t* buffer, u32 col, u32 row)
{
  for (u32 i = 0; i < buffer->count; i++) {
    if (buffer->pixels[i].col == col && buffer->pixels[i].row == row) {
      return buffer->pixels[i];
    }
  }

  pixel_t pixel = { 0 };
  return pixel;
}

void
pixel_buffer_set_pixel(pixel_buffer_t* buffer, pixel_t pixel)
{
  for (u32 i = 0; i < buffer->count; i++) {
    if (buffer->pixels[i].col == pixel.col && buffer->pixels[i].row == pixel.row) {
      buffer->pixels[i] = pixel;
    }
  }

  pixel_buffer_add(buffer, pixel);
}

Uint32
get_pixel(SDL_Surface* surface, u32 x, u32 y)
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

void
png_to_pixel_buffer(pixel_buffer_t* buffer, const char* filename, int scale)
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

void
pixel_buffer_save(pixel_buffer_t* buffer, const char* filename)
{
  //
  // TODO (David): We need to add a version number to the file format and format signature
  //
  FILE* file = fopen(filename, "wb");
  if (file != NULL) {
    (void)fwrite(&buffer->count, sizeof(u32), 1, file);
    (void)fwrite(buffer->pixels, sizeof(pixel_t), buffer->count, file);
    fclose(file);
  }
}

void
pixel_buffer_load(pixel_buffer_t* buffer, const char* filename)
{
  FILE* file = fopen(filename, "rb");
  if (file != NULL) {
    (void)fread(&buffer->count, sizeof(u32), 1, file);
    buffer->pixels = malloc(sizeof(pixel_t) * buffer->count);
    (void)fread(buffer->pixels, sizeof(pixel_t), buffer->count, file);
    fclose(file);
  }
}

void
update_pixel_simulation(pixel_buffer_t* buffer)
{

  u32 count = buffer->count;
  for (u32 i = 0; i < count; i++) {
    pixel_t* pixel = &buffer->pixels[i];

    if (pixel->row >= GRID_HEIGHT - 1) {
      continue;
    }

    pixel_t* below_pixel = NULL;
    for (u32 j = 0; j < count; j++) {
      if (buffer->pixels[j].col == pixel->col && buffer->pixels[j].row == pixel->row + 1) {
        below_pixel = &buffer->pixels[j];
        break;
      }
    }

    if (below_pixel == NULL) {
      pixel->row++;
    } else {

      if (PIXEL_TYPE_FIRE == pixel->type) {

        if (pixel->col > 0 && pixel->col < GRID_WIDTH - 1) {

          pixel_t* left_pixel = NULL;
          pixel_t* right_pixel = NULL;
          pixel_t* left_below_pixel = NULL;
          pixel_t* right_below_pixel = NULL;
          pixel_t* above_pixel = NULL;

          for (u32 j = 0; j < buffer->count; j++) {

            if (buffer->pixels[j].col == pixel->col - 1 && buffer->pixels[j].row == pixel->row) {
              left_pixel = &buffer->pixels[j];
            }

            if (buffer->pixels[j].col == pixel->col + 1 && buffer->pixels[j].row == pixel->row) {
              right_pixel = &buffer->pixels[j];
            }

            if (buffer->pixels[j].col == pixel->col - 1 && buffer->pixels[j].row == pixel->row + 1) {
              left_below_pixel = &buffer->pixels[j];
            }

            if (buffer->pixels[j].col == pixel->col + 1 && buffer->pixels[j].row == pixel->row + 1) {
              right_below_pixel = &buffer->pixels[j];
            }

            if (buffer->pixels[j].col == pixel->col && buffer->pixels[j].row == pixel->row - 1) {
              above_pixel = &buffer->pixels[j];
            }

          }

          if (left_below_pixel == NULL && left_pixel != NULL) {
            pixel->col--;
          } else if (right_below_pixel == NULL && right_pixel != NULL) {
            pixel->col++;
          } else if (above_pixel == NULL) {
            pixel->row--;
          } else {
            if (left_pixel == NULL) {
              pixel->col--;
            } else if (right_pixel == NULL) {
              pixel->col++;
            }
          }
        }
      }

      else if (PIXEL_TYPE_SAND == pixel->type) {

        if (pixel->col > 0 && pixel->col < GRID_WIDTH - 1) {
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

        if (pixel->col > 0 && pixel->col < GRID_WIDTH - 1) {
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

SDL_Color
pixel_type_to_color(pixel_type_t type)
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

#endif
#endif // DK_PIXELBUFFER_H
