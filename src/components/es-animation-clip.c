/*
 * Eclectic Sheep
 *
 * Copyright (C) 2012  Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "main.h"
#include "es-animation-clip.h"

static float
easing_linear (float progress)
{
  return progress;
}

typedef struct
{
  FloatSetter setter;
  void *object;
  float start, end;
  float (*easing) (float progress);
} FloatAnimationData;

static void
es_animation_clip_update (Component *component,
                          int64_t    time)
{
  AnimationClip *clip = ES_ANIMATION_CLIP (component);
  FloatAnimationData *data;
  float new_value, progress;
  int i;

  if (!animation_clip_has_started (clip))
    return;

  if (time >= (clip->start_time + clip->duration))
    {
      animation_clip_clear_started (clip);
      return;
    }

  /* everything is in micro seconds */
  progress = (time - clip->start_time) / (float) clip->duration;

  for (i = 0; i < clip->float_animation_data->len; i ++)
    {
      data = &g_array_index (clip->float_animation_data, FloatAnimationData, i);
      new_value = data->start +
                  (data->end - data->start) * data->easing (progress);

      data->setter (data->object, new_value);
    }
}

/*
 * duration is given in ms in the API, but internally all computations are done
 * is micro seconds
 */
Component *
es_animation_clip_new (int32_t duration)
{
  AnimationClip *renderer;

  renderer = g_slice_new0 (AnimationClip);
  renderer->component.update = es_animation_clip_update;
  renderer->duration = duration * 1000;

  return ES_COMPONENT (renderer);
}

void
es_animation_clip_free (AnimationClip *clip)
{
  if (clip->float_animation_data)
    g_array_unref (clip->float_animation_data);

  g_slice_free (AnimationClip, clip);
}

void
es_animation_clip_add_float (AnimationClip *clip,
                             void          *object,
                             FloatGetter    getter,
                             FloatSetter    setter,
                             float          end_value)
{
  FloatAnimationData data;

  if (clip->float_animation_data == NULL)
    {
      clip->float_animation_data =
        g_array_sized_new (FALSE, FALSE, sizeof (FloatAnimationData), 1);
    }

  data.object = object;
  data.setter = setter;
  data.start = getter (object);
  data.end = end_value;
  data.easing = easing_linear;

  g_array_append_val (clip->float_animation_data, data);
}

void
es_animation_clip_start (AnimationClip *clip)
{
  if (clip->float_animation_data == NULL ||
      clip->float_animation_data->len == 0)
    {
      g_warning ("Tried to start an animation clip without anything to animate");
      return;
    }

  clip->start_time = es_get_current_time ();

  animation_clip_set_started (clip);
}
