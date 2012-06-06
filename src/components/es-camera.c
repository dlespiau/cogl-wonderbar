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

#include "es-camera.h"

#include "es-main.h"

typedef struct
{
  float x, y, z, w;
  float r, g, b, a;
} EsVertex4C4;

static void
es_camera_update (Component *component,
                  int64_t    time)
{
  Camera *camera = ES_CAMERA (component);

  if (CAMERA_HAS_FLAG (camera, PROJECTION_DIRTY))
    {
      CAMERA_CLEAR_FLAG (camera, PROJECTION_DIRTY);

      if (CAMERA_HAS_FLAG (camera, ORTHOGRAPHIC))
        {
          cogl_framebuffer_orthographic (camera->fb,
                                         camera->size,
                                         camera->size,
                                         -camera->size,
                                         -camera->size,
                                         camera->z_near,
                                         camera->z_far);
        }
      else /* perspective */
        {
          float aspect_ratio;

          aspect_ratio = cogl_framebuffer_get_width (camera->fb) /
                         cogl_framebuffer_get_height (camera->fb);

          cogl_framebuffer_perspective (camera->fb,
                                        camera->fov,
                                        aspect_ratio,
                                        camera->z_near,
                                        camera->z_far);
        }
    }
}

static void
draw_frustum (Camera          *camera,
              CoglFramebuffer *fb,
              EsVertex4C4        vertices[8])
{
  CoglContext *context;
  CoglAttributeBuffer *attribute_buffer;
  CoglAttribute *attributes[2];
  CoglPrimitive *primitive;
  CoglPipeline *pipeline;
  CoglIndices *indices;
  unsigned char indices_data[24] = {
      0,1, 1,2, 2,3, 3,0,
      4,5, 5,6, 6,7, 7,4,
      0,4, 1,5, 2,6, 3,7
  };
  CoglDepthState depth_state;

  context = es_get_cogl_context ();

  attribute_buffer = cogl_attribute_buffer_new (context,
                                                8 * sizeof (EsVertex4C4),
                                                vertices);

  attributes[0] = cogl_attribute_new (attribute_buffer,
                                      "cogl_position_in",
                                      sizeof (EsVertex4C4),
                                      offsetof (EsVertex4C4, x),
                                      3,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);

  attributes[1] = cogl_attribute_new (attribute_buffer,
                                      "cogl_color_in",
                                      sizeof (EsVertex4C4),
                                      offsetof (EsVertex4C4, r),
                                      4,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);


  indices = cogl_indices_new (context,
                              COGL_INDICES_TYPE_UNSIGNED_BYTE,
                              indices_data,
                              G_N_ELEMENTS (indices_data));

  primitive = cogl_primitive_new_with_attributes (COGL_VERTICES_MODE_LINES,
                                                  8, attributes, 2);

  cogl_primitive_set_indices (primitive, indices, G_N_ELEMENTS(indices_data));

  cogl_object_unref (attribute_buffer);
  cogl_object_unref (attributes[0]);
  cogl_object_unref (attributes[1]);
  cogl_object_unref (indices);

  pipeline = cogl_pipeline_new (context);

  /* enable depth testing */
  cogl_depth_state_init (&depth_state);
  cogl_depth_state_set_test_enabled (&depth_state, TRUE);
  cogl_pipeline_set_depth_state (pipeline, &depth_state, NULL);

  cogl_framebuffer_draw_primitive (fb, pipeline, primitive);

  cogl_object_unref (primitive);
  cogl_object_unref (pipeline);
}

static void
es_camera_draw (Component       *component,
                CoglFramebuffer *fb)
{
  Camera *camera = ES_CAMERA (component);
  float r, g, b;

  if (camera->fb == fb)
    {
      /* we are responsible for that fb, let's clear it */
      r = cogl_color_get_red_float (&camera->background_color);
      g = cogl_color_get_green_float (&camera->background_color);
      b = cogl_color_get_blue_float (&camera->background_color);

      cogl_framebuffer_clear4f (camera->fb,
                                COGL_BUFFER_BIT_COLOR |
                                COGL_BUFFER_BIT_DEPTH | COGL_BUFFER_BIT_STENCIL,
                                r, g, b, 1);
    }
  else
    {
      /* When the fb we have to draw to is not the one this camera is
       * responsible for, we can draw its frustum to visualize it */
      EsVertex4C4 vertices[8] = {
          /* near plane in projection space */
          {-1, -1, -1, 1, /* position */ .8, .8, .8, .8 /* color */ },
          { 1, -1, -1, 1,                .8, .8, .8, .8             },
          { 1,  1, -1, 1,                .8, .8, .8, .8             },
          {-1,  1, -1, 1,                .8, .8, .8, .8             },
          /* far plane in projection space */
          {-1, -1, 1, 1,  /* position */ .3, .3, .3, .3 /* color */ },
          { 1, -1, 1, 1,                 .3, .3, .3, .3             },
          { 1,  1, 1, 1,                 .3, .3, .3, .3             },
          {-1,  1, 1, 1,                 .3, .3, .3, .3             }
      };
      CoglMatrix projection, projection_inv;
      int i;

      cogl_framebuffer_get_projection_matrix (camera->fb, &projection);
      cogl_matrix_get_inverse (&projection, &projection_inv);

      for (i = 0; i < 8; i++)
        {
          cogl_matrix_transform_point (&projection_inv,
                                       &vertices[i].x,
                                       &vertices[i].y,
                                       &vertices[i].z,
                                       &vertices[i].w);
          vertices[i].x /= vertices[i].w;
          vertices[i].y /= vertices[i].w;
          vertices[i].z /= vertices[i].w;
          vertices[i].w /= 1.0f;
        }

      draw_frustum (camera, fb, vertices);
    }
}

Component *
es_camera_new (void)
{
  Camera *camera;

  camera = g_slice_new0 (Camera);
  camera->component.update = es_camera_update;
  camera->component.draw = es_camera_draw;

  CAMERA_SET_FLAG (camera, PROJECTION_DIRTY);

  return ES_COMPONENT (camera);
}

void
es_camera_free (Camera *camera)
{
  g_slice_free (Camera, camera);
}

void
es_camera_set_near_plane (Camera *camera,
                          float   z_near)
{
  camera->z_near = z_near;
  CAMERA_SET_FLAG (camera, PROJECTION_DIRTY);
}

void
es_camera_set_far_plane (Camera *camera,
                         float   z_far)
{
  camera->z_far = z_far;
  CAMERA_SET_FLAG (camera, PROJECTION_DIRTY);
}

CoglFramebuffer *
es_camera_get_framebuffer (Camera *camera)
{
  return camera->fb;
}

void
es_camera_set_framebuffer (Camera          *camera,
                           CoglFramebuffer *fb)
{
  if (camera->fb)
    {
      cogl_object_unref (camera->fb);
      camera->fb = NULL;
    }

  if (fb)
    camera->fb = cogl_object_ref (fb);
}

EsProjection
es_camera_get_projection (Camera *camera)
{
  if (CAMERA_HAS_FLAG (camera, ORTHOGRAPHIC))
    return ES_PROJECTION_ORTHOGRAPHIC;
  else
    return ES_PROJECTION_PERSPECTIVE;
}

void
es_camera_set_projection (Camera       *camera,
                          EsProjection  projection)
{
  if (projection == ES_PROJECTION_ORTHOGRAPHIC)
    CAMERA_SET_FLAG (camera, ORTHOGRAPHIC);
  else
    CAMERA_CLEAR_FLAG (camera, ORTHOGRAPHIC);
}

void
es_camera_set_field_of_view (Camera *camera,
                             float   fov)
{
  camera->fov = fov;
  CAMERA_SET_FLAG (camera, PROJECTION_DIRTY);
}

void
es_camera_set_size_of_view (Camera *camera,
                            float   sov)
{
  camera->size = sov;
  CAMERA_SET_FLAG (camera, PROJECTION_DIRTY);
}

void
es_camera_set_background_color (Camera    *camera,
                                CoglColor *color)
{
  camera->background_color = *color;
}
