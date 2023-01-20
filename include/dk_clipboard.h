#if !defined(DK_CLIPBOARD_H)
#define DK_CLIPBOARD_H

#include "dk_pixelbuffer.h"

typedef pixel_buffer_t dk_clipboard_t;

void
dk_clipboard_clear(dk_clipboard_t* clipboard);

void
dk_clipboard_set(dk_clipboard_t* clipboard, pixel_buffer_t* buffer);

void
dk_clipboard_paste_to_buffer(dk_clipboard_t* clipboard, pixel_buffer_t* buffer);

pixel_buffer_t*
dk_clipboard_get(dk_clipboard_t* clipboard);

#define DK_CLIPBOARD_IMPLEMENTATION
#if defined(DK_CLIPBOARD_IMPLEMENTATION)

void
dk_clipboard_clear(dk_clipboard_t* clipboard) {
  if (clipboard) {
    pixel_buffer_clear(clipboard);
  }
}

void
dk_clipboard_set(dk_clipboard_t* clipboard, pixel_buffer_t* buffer) {
  if (clipboard == NULL) {
    pixel_buffer_clear(clipboard);
  }
  memcpy(clipboard, buffer, sizeof(pixel_buffer_t));
}

pixel_buffer_t*
dk_clipboard_get(dk_clipboard_t* clipboard) {
  return clipboard;
}

void
dk_clipboard_paste_to_buffer(dk_clipboard_t* clipboard, pixel_buffer_t* buffer) {

  pixel_buffer_t* temp = (pixel_buffer_t*)malloc(sizeof(pixel_buffer_t));
  temp->count = buffer->count;
  temp->pixels = (pixel_t*)malloc(sizeof(pixel_t) * temp->count);
  memcpy(temp->pixels, buffer->pixels, sizeof(pixel_t) * temp->count);

  pixel_buffer_merge(temp, clipboard);

  buffer->count = temp->count;
  buffer->pixels = (pixel_t*)malloc(sizeof(pixel_t) * buffer->count);
  memcpy(buffer->pixels, temp->pixels, sizeof(pixel_t) * temp->count);

  free(temp->pixels);
  free(temp);
}

#endif // DK_CLIPBOARD_IMPLEMENTATION

#endif
