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

#ifndef __ES_ENTITY_H__
#define __ES_ENTITY_H__

#include <stdint.h>

#include <cogl/cogl.h>

#define ES_COMPONENT(p) ((Component *)(p))

typedef struct _component Component;
typedef struct _entity    Entity;

typedef enum
{
  ES_COMPONENT_TYPE_ANIMATION_CLIP,
  ES_COMPONENT_TYPE_CAMERA,
  ES_COMPONENT_TYPE_LIGHT,
  ES_COMPONENT_TYPE_MESH_RENDERER,

  ES_N_COMPNONENTS
} ComponentType;

struct _component
{
  ComponentType type;
  Entity *entity;     /* back pointer to the entity the component belongs to */
  void (*start)   (Component *component);
  void (*update)  (Component *component, int64_t time);
  void (*draw)    (Component *component, CoglFramebuffer *fb);
};

typedef enum
{
  ENTITY_FLAG_NONE        = 0,
  ENTITY_FLAG_DIRTY       = 1 << 0,
  ENTITY_FLAG_CAST_SHADOW = 1 << 1,
} EntityFlag;

#define ENTITY_HAS_FLAG(entity,flag)    ((entity)->flags & ENTITY_FLAG_##flag)
#define ENTITY_SET_FLAG(entity,flag)    ((entity)->flags |= ENTITY_FLAG_##flag)
#define ENTITY_CLEAR_FLAG(entity,flag)  ((entity)->flags &= ~(ENTITY_FLAG_##flag))

/* DIRTY */
#define entity_is_dirty(entity)         (ENTITY_HAS_FLAG (entity, DIRTY))
#define entity_set_dirty(entity)        (ENTITY_SET_FLAG (entity, DIRTY))
#define entity_clear_dirty(entity)      (ENTITY_CLEAR_FLAG (entity, DIRTY))

/* CAST_SHADOW */
#define entity_cast_shadow(entity)      (ENTITY_HAS_FLAG (entity, CAST_SHADOW))

/* FIXME:
 *  - directly store the position in the transform matrix?
 */
struct _entity
{
  /* private fields */
  uint32_t flags;
  struct { float x, y, z; } position;
  CoglQuaternion rotation;
  CoglMatrix transform;
  GPtrArray *components;
};

void                    es_entity_init          (Entity *entity);
float                   es_entity_get_x         (Entity *entity);
void                    es_entity_set_x         (Entity *entity,
                                                 float   x);
float                   es_entity_get_y         (Entity *entity);
void                    es_entity_set_y         (Entity *entity,
                                                 float   y);
float                   es_entity_get_z         (Entity *entity);
void                    es_entity_set_z         (Entity *entity,
                                                 float   z);
void                    es_entity_set_position  (Entity *entity,
                                                 float position[3]);
CoglQuaternion *        es_entity_get_rotation  (Entity *entity);
void                    es_entity_set_rotation  (Entity *entity,
                                                 CoglQuaternion *rotation);
CoglMatrix *            es_entity_get_transform (Entity *entity);
void                    es_entity_add_component (Entity    *entity,
                                                 Component *component);
void                    es_entity_update        (Entity  *entity,
                                                 int64_t  time);
void                    es_entity_draw          (Entity *entity,
                                                 CoglFramebuffer *fb);
void                    es_entity_translate     (Entity *entity,
                                                 float   tx,
                                                 float   tz,
                                                 float   ty);
void                    es_entity_rotate_x_axis (Entity *entity,
                                                 float   x_angle);
void                    es_entity_rotate_y_axis (Entity *entity,
                                                 float   y_angle);
void                    es_entity_rotate_z_axis (Entity *entity,
                                                 float   z_angle);

CoglPipeline *          es_entity_get_pipeline  (Entity *entity);

void                    es_entity_set_cast_shadow (Entity   *entity,
                                                   gboolean  cast_shadow);

#endif /* __ES_ENTITY_H__ */
