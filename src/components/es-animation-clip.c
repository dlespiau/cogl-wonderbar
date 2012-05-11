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

#include "es-main.h"
#include "es-animation-clip.h"

typedef struct
{
  FloatSetter setter;
  void *object;
  float start, end;
  float (*easing) (float progress);
} FloatAnimationData;

typedef struct
{
  QuaternionSetter setter;
  void *object;
  CoglQuaternion start, end;
  float (*easing) (float progress);
} QuaternionAnimationData;

static float
easing_linear (float progress)
{
  return progress;
}

static void
es_animation_clip_update (Component *component,
                          int64_t    time)
{
  AnimationClip *clip = ES_ANIMATION_CLIP (component);
  float progress;
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

  /* update floats */
  if (clip->float_animation_data)
    {
      FloatAnimationData *data;
      float new_value;

      for (i = 0; i < clip->float_animation_data->len; i ++)
        {
          data = &g_array_index (clip->float_animation_data,
                                 FloatAnimationData,
                                 i);

          new_value = data->start +
            (data->end - data->start) * data->easing (progress);

          data->setter (data->object, new_value);
      }
    }

  /* update quaternions */
  if (clip->quaternion_animation_data)
    {
      QuaternionAnimationData *data;
      CoglQuaternion new_value;

      for (i = 0; i < clip->quaternion_animation_data->len; i ++)
        {
          data = &g_array_index (clip->quaternion_animation_data,
                                 QuaternionAnimationData,
                                 i);

          cogl_quaternion_slerp (&new_value,
                                 &data->start, &data->end,
                                 data->easing (progress));

          data->setter (data->object, &new_value);
        }
    }
}

/*
 * duration is given in ms in the API, but internally all computations are done
 * in micro seconds
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

  if (clip->quaternion_animation_data)
    g_array_unref (clip->quaternion_animation_data);

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
es_animation_clip_add_quaternion  (AnimationClip    *clip,
                                   void             *object,
                                   QuaternionGetter  getter,
                                   QuaternionSetter  setter,
                                   CoglQuaternion   *end_value)
{
  QuaternionAnimationData data;

  if (clip->quaternion_animation_data == NULL)
    {
      clip->quaternion_animation_data =
        g_array_sized_new (FALSE, FALSE, sizeof (QuaternionAnimationData), 1);
    }

  data.object = object;
  data.setter = setter;
  cogl_quaternion_init_from_quaternion (&data.start, getter (object));
  cogl_quaternion_init_from_quaternion (&data.end, end_value);
  data.easing = easing_linear;

  g_array_append_val (clip->quaternion_animation_data, data);
}

static int
has_float_animation_data (AnimationClip *clip)
{
  return (clip->float_animation_data == NULL ||
          clip->float_animation_data->len == 0);
}

static int
has_quaternion_animation_data (AnimationClip *clip)
{
  return (clip->quaternion_animation_data == NULL ||
          clip->quaternion_animation_data->len == 0);
}

void
es_animation_clip_start (AnimationClip *clip)
{
  if (!has_float_animation_data (clip) &&
      !has_quaternion_animation_data (clip))
    {
      g_warning ("Tried to start an animation clip without anything to animate");
      return;
    }

  if (animation_clip_has_started (clip))
    return;

  clip->start_time = es_get_current_time ();

  animation_clip_set_started (clip);
}
