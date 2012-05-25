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
#include "es-mesh-renderer.h"

typedef struct
{
  float x, y, z;        /* position       */
  float n_x, n_y, n_z;  /* normal         */
} Vertex;

/*
 *        f +--------+ e
 *         /        /|
 *        /        / |
 *    b  /      a /  |
 *      +--------+   |
 *      |  g     |   + h
 *      |        |  /
 *      |        | /
 *    c |        |/
 *      +--------+ d
 */

#define pos_a        1.0f,  1.0f, 1.0f
#define pos_b       -1.0f,  1.0f, 1.0f
#define pos_c       -1.0f, -1.0f, 1.0f
#define pos_d        1.0f, -1.0f, 1.0f

#define pos_e        1.0f,  1.0f, -1.0f
#define pos_f       -1.0f,  1.0f, -1.0f
#define pos_g       -1.0f, -1.0f, -1.0f
#define pos_h        1.0f, -1.0f, -1.0f

#define norm_front   0.0f,  0.0f,  1.0f
#define norm_right   1.0f,  0.0f,  0.0f
#define norm_back    0.0f,  0.0f, -1.0f
#define norm_left   -1.0f,  0.0f,  0.0f
#define norm_top     0.0f,  1.0f,  0.0f
#define norm_bottom  0.0f, -1.0f,  0.0f

static Vertex cube_vertices[] =
{
  { pos_a, norm_front },
  { pos_b, norm_front },
  { pos_c, norm_front },
  { pos_c, norm_front },
  { pos_d, norm_front },
  { pos_a, norm_front },

  { pos_e, norm_right },
  { pos_a, norm_right },
  { pos_d, norm_right },
  { pos_d, norm_right },
  { pos_h, norm_right },
  { pos_e, norm_right },

  { pos_f, norm_back },
  { pos_e, norm_back },
  { pos_h, norm_back },
  { pos_h, norm_back },
  { pos_g, norm_back },
  { pos_f, norm_back },

  { pos_b, norm_left },
  { pos_f, norm_left },
  { pos_g, norm_left },
  { pos_g, norm_left },
  { pos_c, norm_left },
  { pos_b, norm_left },

  { pos_e, norm_top },
  { pos_f, norm_top },
  { pos_b, norm_top },
  { pos_b, norm_top },
  { pos_a, norm_top },
  { pos_e, norm_top },

  { pos_c, norm_bottom },
  { pos_g, norm_bottom },
  { pos_h, norm_bottom },
  { pos_h, norm_bottom },
  { pos_d, norm_bottom },
  { pos_c, norm_bottom }
};

#undef pos_a
#undef pos_b
#undef pos_c
#undef pos_d
#undef pos_e
#undef pos_f
#undef pos_g
#undef pos_h

#undef norm_front
#undef norm_right
#undef norm_back
#undef norm_left
#undef norm_top
#undef norm_bottom

static CoglPrimitive *
create_cube_primitive (void)
{
  CoglAttributeBuffer *attribute_buffer;
  CoglAttribute *attributes[2];
  CoglPrimitive *primitive;
  CoglContext *context;

  context = es_get_cogl_context ();

  attribute_buffer = cogl_attribute_buffer_new (context,
                                                sizeof (cube_vertices),
                                                cube_vertices);
  attributes[0] = cogl_attribute_new (attribute_buffer,
                                      "cogl_position_in",
                                      sizeof (Vertex),
                                      offsetof (Vertex, x),
                                      3,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);
  attributes[1] = cogl_attribute_new (attribute_buffer,
                                      "cogl_normal_in",
                                      sizeof (Vertex),
                                      offsetof (Vertex, n_x),
                                      3,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);
  cogl_object_unref (attribute_buffer);

  primitive = cogl_primitive_new_with_attributes (COGL_VERTICES_MODE_TRIANGLES,
                                                  G_N_ELEMENTS (cube_vertices),
                                                  attributes, 2);
  cogl_object_unref (attributes[0]);
  cogl_object_unref (attributes[1]);

  return primitive;
}

/*
 *        b +--------+ a
 *         /        /
 *        /        /
 *    c  /      d /
 *      +--------+
 */

#define pos_a    10.0f, -2.0f, -10.0f
#define pos_b   -10.0f, -2.0f, -10.0f
#define pos_c   -10.0f, -2.0f,  10.0f
#define pos_d    10.0f, -2.0f,  10.0f

#define norm     1.0f,  1.0f, 1.0f

static Vertex plane_vertices[] =
{
  { pos_a, norm },
  { pos_b, norm },
  { pos_c, norm },
  { pos_c, norm },
  { pos_d, norm },
  { pos_a, norm },
};

#undef pos_a
#undef pos_b
#undef pos_c
#undef pos_d

#undef norm

static CoglPrimitive *
create_plane_primitive (void)
{
  CoglAttributeBuffer *attribute_buffer;
  CoglAttribute *attributes[2];
  CoglPrimitive *primitive;
  CoglContext *context;

  context = es_get_cogl_context ();

  attribute_buffer = cogl_attribute_buffer_new (context,
                                                sizeof (plane_vertices),
                                                plane_vertices);
  attributes[0] = cogl_attribute_new (attribute_buffer,
                                      "cogl_position_in",
                                      sizeof (Vertex),
                                      offsetof (Vertex, x),
                                      3,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);
  attributes[1] = cogl_attribute_new (attribute_buffer,
                                      "cogl_normal_in",
                                      sizeof (Vertex),
                                      offsetof (Vertex, n_x),
                                      3,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);
  cogl_object_unref (attribute_buffer);

  primitive = cogl_primitive_new_with_attributes (COGL_VERTICES_MODE_TRIANGLES,
                                                  G_N_ELEMENTS (plane_vertices),
                                                  attributes, 2);
  cogl_object_unref (attributes[0]);
  cogl_object_unref (attributes[1]);

  return primitive;
}

static MashData *
create_ply_primitive (const gchar *filename)
{
  MashData *data = mash_data_new ();
  GError *error = NULL;

  mash_data_load (data, MASH_DATA_NONE, filename, &error);
  if (error)
    {
      g_critical ("could not load model %s: %s", filename, error->message);
      return NULL;
    }

  return data;
}

static void
es_mesh_renderer_draw (Component *component)
{
  MeshRenderer *renderer = ES_MESH_RENDERER (component);
  CoglFramebuffer *fb;

  fb = cogl_get_draw_framebuffer ();

  if (renderer->primitive)
    {
      cogl_framebuffer_draw_primitive (fb,
                                       renderer->pipeline,
                                       renderer->primitive);
    }
  else if (renderer->mesh_data)
    {
      cogl_set_source (renderer->pipeline);
      mash_data_render (renderer->mesh_data);
    }
}

static MeshRenderer *
es_mesh_renderer_new (void)
{
  MeshRenderer *renderer;

  renderer = g_slice_new0 (MeshRenderer);
  renderer->component.type = ES_COMPONENT_TYPE_MESH_RENDERER;
  renderer->component.draw = es_mesh_renderer_draw;

  return renderer;
}

Component *
es_mesh_renderer_new_from_file (const char   *file,
                                CoglPipeline *pipeline)
{
  MeshRenderer *renderer;

  renderer = es_mesh_renderer_new ();
  renderer->mesh_data = create_ply_primitive (file);
  renderer->pipeline = cogl_object_ref (pipeline);

  return ES_COMPONENT (renderer);
}

Component *
es_mesh_renderer_new_from_template (const char   *name,
                                    CoglPipeline *pipeline)
{
  MeshRenderer *renderer;

  renderer = es_mesh_renderer_new ();

  if (g_strcmp0 (name, "plane") == 0)
    renderer->primitive = create_plane_primitive ();
  else if (g_strcmp0 (name, "cube") == 0)
    renderer->primitive = create_cube_primitive ();
  else
    g_assert_not_reached ();

  renderer->pipeline = cogl_object_ref (pipeline);

  return ES_COMPONENT (renderer);
}

void es_mesh_renderer_free (MeshRenderer *renderer)
{
  if (renderer->pipeline)
    cogl_object_unref (renderer->pipeline);

  if (renderer->primitive)
    cogl_object_unref (renderer->primitive);

  if (renderer->mesh_data)
    g_object_unref (renderer->mesh_data);

  g_slice_free (MeshRenderer, renderer);
}


void
es_mesh_renderer_set_pipeline (MeshRenderer *renderer,
                               CoglPipeline *pipeline)
{
  if (renderer->pipeline)
    {
      cogl_object_unref (renderer->pipeline);
      renderer->pipeline = NULL;
    }

  if (pipeline)
    renderer->pipeline = cogl_object_ref (pipeline);
}
