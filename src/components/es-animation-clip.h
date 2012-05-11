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

#ifndef __ES_ANIMATION_CLIP_H__
#define __ES_ANIMATION_CLIP_H__

#include <cogl/cogl.h>

#include "es-entity.h"

#define FLOAT_GETTER(func) ((FloatGetter) (func))
#define FLOAT_SETTER(func) ((FloatSetter) (func))

typedef float (*FloatGetter) (void *object);
typedef void (*FloatSetter) (void *object, float f);

#define QUATERNION_GETTER(func) ((QuaternionGetter) (func))
#define QUATERNION_SETTER(func) ((QuaternionSetter) (func))

typedef CoglQuaternion * (*QuaternionGetter) (void *object);
typedef void (*QuaternionSetter) (void *object, CoglQuaternion *quaternion);

#define ES_ANIMATION_CLIP(p) ((AnimationClip *)(p))

typedef struct _AnimationClip AnimationClip;

typedef enum
{
  ANIMATION_CLIP_FLAG_NONE,
  ANIMATION_CLIP_FLAG_STARTED
} AnimationClipFlag;

#define ANIMATION_CLIP_HAS_FLAG(clip,flag)    \
    ((clip)->flags & ANIMATION_CLIP_FLAG_##flag)

#define ANIMATION_CLIP_SET_FLAG(clip,flag)    \
    ((clip)->flags |= ANIMATION_CLIP_FLAG_##flag)

#define ANIMATION_CLIP_CLEAR_FLAG(clip,flag)  \
    ((clip)->flags &= ~(ANIMATION_CLIP_FLAG_##flag))

/* STARTED flag */
#define animation_clip_has_started(clip)      \
    (ANIMATION_CLIP_HAS_FLAG (clip, STARTED))
#define animation_clip_set_started(clip)      \
    (ANIMATION_CLIP_SET_FLAG (clip, STARTED))
#define animation_clip_clear_started(clip)    \
    (ANIMATION_CLIP_CLEAR_FLAG (clip, STARTED))

struct _AnimationClip
{
  Component component;
  uint32_t flags;
  int64_t duration;   /* micro seconds */
  int64_t start_time; /* micro seconds */
  GArray *float_animation_data;
  GArray *quaternion_animation_data;
};

Component * es_animation_clip_new             (int32_t duration);

void        es_animation_clip_free            (AnimationClip *clip);

void        es_animation_clip_add_float       (AnimationClip *clip,
                                               void          *object,
                                               FloatGetter    getter,
                                               FloatSetter    setter,
                                               float          end_value);
void        es_animation_clip_add_quaternion  (AnimationClip    *clip,
                                               void             *object,
                                               QuaternionGetter  getter,
                                               QuaternionSetter  setter,
                                               CoglQuaternion   *end_value);
void        es_animation_clip_start           (AnimationClip *clip);
void        es_animation_clip_stop            (AnimationClip *clip);

#endif /* __ES_ANIMATION_CLIP_H__ */
