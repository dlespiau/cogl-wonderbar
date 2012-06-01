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
es_camera_draw (Component       *component,
                CoglFramebuffer *fb)
{
  Camera *camera = ES_CAMERA (component);
  float r, g, b;

  if (camera->fb != fb)
    return;

  r = cogl_color_get_red_float (&camera->background_color);
  g = cogl_color_get_green_float (&camera->background_color);
  b = cogl_color_get_blue_float (&camera->background_color);

  cogl_framebuffer_clear4f (camera->fb,
                            COGL_BUFFER_BIT_COLOR |
                            COGL_BUFFER_BIT_DEPTH | COGL_BUFFER_BIT_STENCIL,
                            r, g, b, 1);
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
