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

#include "es-light.h"

#include "es-main.h"

static float *
get_color_array (CoglColor *color)
{
  static float array[4];

  array[0] = cogl_color_get_red_float (color);
  array[1] = cogl_color_get_blue_float (color);
  array[2] = cogl_color_get_green_float (color);
  array[3] = cogl_color_get_alpha_float (color);

  return array;
}

static void
es_light_update (Component *component,
                 int64_t    time)
{
  Light *light = ES_LIGHT (component);
  CoglPipeline *pipeline;
  float norm_direction[3];
  int location;

  pipeline = es_get_root_pipeline ();

  /* the lighting shader expects the direction vector to be pointing towards
   * the light, we encode that with the light position in the case of a
   * directional light */
  norm_direction[0] = es_entity_get_x (component->entity);
  norm_direction[1] = es_entity_get_y (component->entity);
  norm_direction[2] = es_entity_get_z (component->entity);
  cogl_vector3_normalize (norm_direction);

  location = cogl_pipeline_get_uniform_location (pipeline,
                                                 "light0_direction_norm");
  cogl_pipeline_set_uniform_float (pipeline,
                                   location,
                                   3, 1,
                                   norm_direction);

  location = cogl_pipeline_get_uniform_location (pipeline,
                                                 "light0_ambient");
  cogl_pipeline_set_uniform_float (pipeline,
                                   location,
                                   4, 1,
                                   get_color_array (&light->ambient));

  location = cogl_pipeline_get_uniform_location (pipeline,
                                                 "light0_diffuse");
  cogl_pipeline_set_uniform_float (pipeline,
                                   location,
                                   4, 1,
                                   get_color_array (&light->diffuse));

  location = cogl_pipeline_get_uniform_location (pipeline,
                                                 "light0_specular");
  cogl_pipeline_set_uniform_float (pipeline,
                                   location,
                                   4, 1,
                                   get_color_array (&light->specular));

}

Component *
es_light_new (void)
{
  Light *light;

  light = g_slice_new0 (Light);
  light->component.type = ES_COMPONENT_TYPE_LIGHT;
  light->component.update = es_light_update;
  cogl_color_init_from_4f (&light->ambient, 1.0, 1.0, 1.0, 1.0);
  cogl_color_init_from_4f (&light->diffuse, 1.0, 1.0, 1.0, 1.0);
  cogl_color_init_from_4f (&light->specular, 1.0, 1.0, 1.0, 1.0);

  return ES_COMPONENT (light);
}

void
es_light_free (Light *light)
{
  g_slice_free (Light, light);
}

void
es_light_set_ambient (Light     *light,
                      CoglColor *ambient)
{
  cogl_color_init_from_color (&light->ambient, ambient);
}

void
es_light_set_diffuse (Light     *light,
                      CoglColor *diffuse)
{
  cogl_color_init_from_color (&light->diffuse, diffuse);
}

void
es_light_set_specular (Light     *light,
                       CoglColor *specular)
{
  cogl_color_init_from_color (&light->specular, specular);
}
