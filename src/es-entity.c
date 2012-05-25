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

#include "components/es-mesh-renderer.h"
#include "es-entity.h"

void es_entity_init (Entity *entity)
{
  entity->position.x = 0.0f;
  entity->position.y = 0.0f;
  entity->position.z = 0.0f;

  cogl_quaternion_init_identity (&entity->rotation);
  cogl_matrix_init_identity (&entity->transform);
  entity->components = g_ptr_array_new ();
}

float
es_entity_get_x (Entity *entity)
{
  return entity->position.x;
}

void
es_entity_set_x (Entity *entity,
                 float   x)
{
  entity->position.x = x;
  entity_set_dirty (entity);
}

float
es_entity_get_y (Entity *entity)
{
  return entity->position.y;
}

void
es_entity_set_y (Entity *entity,
                 float   y)
{
  entity->position.y = y;
  entity_set_dirty (entity);
}

float
es_entity_get_z (Entity *entity)
{
  return entity->position.z;
}

void
es_entity_set_z (Entity *entity,
                 float   z)
{
  entity->position.z = z;
  entity_set_dirty (entity);
}

const CoglQuaternion *
es_entity_get_rotation (Entity *entity)
{
  return &entity->rotation;
}

void es_entity_set_rotation (Entity *entity,
                             CoglQuaternion *rotation)
{
  cogl_quaternion_init_from_quaternion (&entity->rotation, rotation);
  entity_set_dirty (entity);
}

const CoglMatrix *
es_entity_get_transform (Entity *entity)
{
  CoglMatrix translation;

  if (!entity_is_dirty (entity))
    return &entity->transform;

  cogl_matrix_init_translation (&translation,
                                entity->position.x,
                                entity->position.y,
                                entity->position.z);
  cogl_matrix_init_from_quaternion (&entity->transform, &entity->rotation);
  cogl_matrix_multiply (&entity->transform, &entity->transform, &translation);

  entity_clear_dirty (entity);

  return &entity->transform;
}

void
es_entity_add_component (Entity    *entity,
                         Component *component)
{
  g_ptr_array_add (entity->components, component);
}

void
es_entity_update (Entity  *entity,
                  int64_t  time)
{
  int i;

  for (i = 0; i < entity->components->len; i++)
    {
      Component *component = g_ptr_array_index (entity->components, i);

      if (component->update)
        component->update(component, time);
    }
}

void
es_entity_draw (Entity *entity)
{
  int i;

  for (i = 0; i < entity->components->len; i++)
    {
      Component *component = g_ptr_array_index (entity->components, i);

      if (component->draw)
        component->draw(component);
    }
}

void
es_entity_translate (Entity *entity,
                     float   tx,
                     float   tz,
                     float   ty)
{
  entity->position.x += tx;
  entity->position.y += ty;
  entity->position.z += tz;

  entity_set_dirty (entity);
}

void
es_entity_rotate_x_axis (Entity *entity,
                         float   x_angle)
{
  CoglQuaternion x_rotation, *current;

  /* XXX: avoid the allocation here, and/or make the muliplication in place */
  current = cogl_quaternion_copy (&entity->rotation);
  cogl_quaternion_init_from_x_rotation (&x_rotation, x_angle);
  cogl_quaternion_multiply (&entity->rotation, current, &x_rotation);
  cogl_quaternion_free (current);

  entity_set_dirty (entity);
}

CoglPipeline *
es_entity_get_pipeline (Entity *entity)
{
  int i;

  for (i = 0; i < entity->components->len; i++)
    {
      Component *component = g_ptr_array_index (entity->components, i);

      if (component->type == ES_COMPONENT_TYPE_MESH_RENDERER)
        return ES_MESH_RENDERER (component)->pipeline;
    }

    return NULL;
}
