#if !defined(DK_LINMATH_H)
#define DK_LINMATH_H

#include "dk.h"
#include <math.h>

struct v2
{
  f32 x;
  f32 y;
};

void
v2_add(struct v2* a, struct v2* b, struct v2* out);

void
v2_sub(struct v2* a, struct v2* b, struct v2* out);

void
v2_mul(struct v2* a, struct v2* b, struct v2* out);

void
v2_div(struct v2* a, struct v2* b, struct v2* out);

void

v2_scale(struct v2* a, f32 b, struct v2* out);

f32
v2_dot(struct v2* a, struct v2* b);

f32
v2_cross(struct v2* a, struct v2* b);

f32
v2_length(struct v2* a);

f32
v2_length_squared(struct v2* a);

void
v2_normalize(struct v2* a, struct v2* out);

void
v2_lerp(struct v2* a, struct v2* b, f32 t, struct v2* out);

void
v2_reflect(struct v2* a, struct v2* b, struct v2* out);

void
v2_rotate(struct v2* a, f32 angle, struct v2* out);

void
v2_min(struct v2* a, struct v2* b, struct v2* out);

void
v2_max(struct v2* a, struct v2* b, struct v2* out);

void
v2_clamp(struct v2* a, struct v2* min, struct v2* max, struct v2* out);

void
v2_negate(struct v2* a, struct v2* out);

void
v2_abs(struct v2* a, struct v2* out);

void
v2_floor(struct v2* a, struct v2* out);

void
v2_ceil(struct v2* a, struct v2* out);

void
v2_round(struct v2* a, struct v2* out);

void
v2_truncate(struct v2* a, struct v2* out);

void
v2_fract(struct v2* a, struct v2* out);

void
v2_mod(struct v2* a, struct v2* b, struct v2* out);

void
v2_step(struct v2* a, struct v2* b, struct v2* out);

void
v2_smoothstep(struct v2* a, struct v2* b, f32 t, struct v2* out);

void
v2_saturate(struct v2* a, struct v2* out);

void
v2_hadamard(struct v2* a, struct v2* b, struct v2* out);

f32
v2_distance(struct v2* a, struct v2* b);

struct v2
v2_add_v2(struct v2* a, struct v2* b);

struct v2
v2_sub_v2(struct v2* a, struct v2* b);

struct v2
v2_mul_v2(struct v2* a, struct v2* b);

struct v2
v2_div_v2(struct v2* a, struct v2* b);

struct v2
v2_scale_v2(struct v2* a, f32 b);

struct v2
v2_normalize_v2(struct v2* a);

struct v2
v2_zero();

struct v2
v2_one();

struct v2
v2_up();

struct v2
v2_down();

struct v2
v2_left();

struct v2
v2_right();

#if defined(V2_IMPLEMENTATION)

void
v2_add(struct v2* a, struct v2* b, struct v2* out)
{
  out->x = a->x + b->x;
  out->y = a->y + b->y;
}

void
v2_sub(struct v2* a, struct v2* b, struct v2* out)
{
  out->x = a->x - b->x;
  out->y = a->y - b->y;
}

void
v2_mul(struct v2* a, struct v2* b, struct v2* out)
{
  out->x = a->x * b->x;
  out->y = a->y * b->y;
}

void
v2_div(struct v2* a, struct v2* b, struct v2* out)
{
  out->x = a->x / b->x;
  out->y = a->y / b->y;
}

void
v2_scale(struct v2* a, f32 b, struct v2* out)
{
  out->x = a->x * b;
  out->y = a->y * b;
}

f32
v2_dot(struct v2* a, struct v2* b)
{
  return a->x * b->x + a->y * b->y;
}

f32
v2_cross(struct v2* a, struct v2* b)
{
  return a->x * b->y - a->y * b->x;
}

f32
v2_length(struct v2* a)
{
  return sqrtf(a->x * a->x + a->y * a->y);
}

f32
v2_length_squared(struct v2* a)
{
  return a->x * a->x + a->y * a->y;
}

void
v2_normalize(struct v2* a, struct v2* out)
{
  f32 length = v2_length(a);
  out->x = a->x / length;
  out->y = a->y / length;
}

void
v2_lerp(struct v2* a, struct v2* b, f32 t, struct v2* out)
{
  out->x = a->x + (b->x - a->x) * t;
  out->y = a->y + (b->y - a->y) * t;
}

void
v2_reflect(struct v2* a, struct v2* b, struct v2* out)
{
  f32 dot = v2_dot(a, b);
  out->x = a->x - 2.0f * dot * b->x;
  out->y = a->y - 2.0f * dot * b->y;
}

void
v2_rotate(struct v2* a, f32 angle, struct v2* out)
{
  f32 c = cosf(angle);
  f32 s = sinf(angle);
  out->x = a->x * c - a->y * s;
  out->y = a->x * s + a->y * c;
}

void
v2_min(struct v2* a, struct v2* b, struct v2* out)
{
  out->x = a->x < b->x ? a->x : b->x;
  out->y = a->y < b->y ? a->y : b->y;
}

void
v2_max(struct v2* a, struct v2* b, struct v2* out)
{
  out->x = a->x > b->x ? a->x : b->x;
  out->y = a->y > b->y ? a->y : b->y;
}

void
v2_clamp(struct v2* a, struct v2* min, struct v2* max, struct v2* out)
{
  out->x = a->x < min->x ? min->x : a->x > max->x ? max->x : a->x;
  out->y = a->y < min->y ? min->y : a->y > max->y ? max->y : a->y;
}

void
v2_negate(struct v2* a, struct v2* out)
{
  out->x = -a->x;
  out->y = -a->y;
}

void
v2_abs(struct v2* a, struct v2* out)
{
  out->x = fabsf(a->x);
  out->y = fabsf(a->y);
}

void
v2_floor(struct v2* a, struct v2* out)
{
  out->x = floorf(a->x);
  out->y = floorf(a->y);
}

void
v2_ceil(struct v2* a, struct v2* out)
{
  out->x = ceilf(a->x);
  out->y = ceilf(a->y);
}

void
v2_round(struct v2* a, struct v2* out)
{
  out->x = roundf(a->x);
  out->y = roundf(a->y);
}

void
v2_truncate(struct v2* a, struct v2* out)
{
  out->x = truncf(a->x);
  out->y = truncf(a->y);
}

void
v2_fract(struct v2* a, struct v2* out)
{
  out->x = a->x - floorf(a->x);
  out->y = a->y - floorf(a->y);
}

void
v2_smoothstep(struct v2* a, struct v2* b, f32 t, struct v2* out)
{
  t = t * t * (3.0f - 2.0f * t);
  out->x = a->x + (b->x - a->x) * t;
  out->y = a->y + (b->y - a->y) * t;
}

f32
v2_distance(struct v2* a, struct v2* b)
{
  struct v2 result;
  v2_sub(a, b, &result);
  return v2_length(&result);
}

struct v2
v2_add_v2(struct v2* a, struct v2* b)
{
  struct v2 result;
  v2_add(a, b, &result);
  return result;
}

struct v2
v2_sub_v2(struct v2* a, struct v2* b)
{
  struct v2 result;
  v2_sub(a, b, &result);
  return result;
}

struct v2
v2_mul_v2(struct v2* a, struct v2* b)
{
  struct v2 result;
  v2_mul(a, b, &result);
  return result;
}

struct v2
v2_div_v2(struct v2* a, struct v2* b)
{
  struct v2 result;
  v2_div(a, b, &result);
  return result;
}

struct v2
v2_scale_v2(struct v2* a, f32 b)
{
  struct v2 result;
  v2_scale(a, b, &result);
  return result;
}

struct v2
v2_normalize_v2(struct v2* a)
{
  struct v2 result;
  v2_normalize(a, &result);
  return result;
}

struct v2
v2_zero()
{
  return (struct v2){ 0 };
}

struct v2
v2_one()
{
  struct v2 result = { 1, 1 };
  return result;
}

struct v2
v2_up()
{
  struct v2 result = { 0, 1 };
  return result;
}

struct v2
v2_down()
{
  struct v2 result = { 0, -1 };
  return result;
}

struct v2
v2_left()
{
  struct v2 result = { -1, 0 };
  return result;
}

struct v2
v2_right()
{
  struct v2 result = { 1, 0 };
  return result;
}

#endif // DK_LINMATH_H

#endif // DK_LINMATH_H
