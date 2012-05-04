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

#include "entity.h"

void es_entity_init (Entity *entity)
{
  entity->position.x = 0.0f;
  entity->position.y = 0.0f;
  entity->position.z = 0.0f;

  cogl_quaternion_init_identity (&entity->rotation);
  cogl_matrix_init_identity (&entity->transform);
  entity->components = g_ptr_array_new ();
}

const CoglMatrix *
es_entity_get_transform (Entity *entity)
{
  CoglMatrix translation;

  if (!entity_is_dirty (entity))
    return &entity->transform;

  cogl_matrix_init_translation (&translation,
                                es_entity_get_x (entity),
                                es_entity_get_y (entity),
                                es_entity_get_z (entity));
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
es_entity_draw (Entity *entity)
{
  int i;

  for (i = 0; i < entity->components->len; i++)
    {
      Component *component = g_ptr_array_index (entity->components, i);

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
