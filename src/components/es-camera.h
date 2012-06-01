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

#ifndef __ES_CAMERA_H__
#define __ES_CAMERA_H__

#include "es-entity.h"

typedef struct _Camera Camera;

#define ES_CAMERA(p) ((Camera *)(p))

typedef enum
{
  CAMERA_FLAG_NONE             = 0,
  CAMERA_FLAG_ORTHOGRAPHIC     = 1 << 0,
  CAMERA_FLAG_VIEW_DIRTY       = 1 << 1,
  CAMERA_FLAG_PROJECTION_DIRTY = 1 << 2,
} EsCameraFlag;

typedef enum
{
  ES_PROJECTION_PERSPECTIVE,
  ES_PROJECTION_ORTHOGRAPHIC
} EsProjection;


#define CAMERA_HAS_FLAG(camera,flag)  ((camera)->flags & CAMERA_FLAG_##flag)
#define CAMERA_SET_FLAG(camera,flag)  ((camera)->flags |= CAMERA_FLAG_##flag)
#define CAMERA_CLEAR_FLAG(camera,flag)((camera)->flags &= ~(CAMERA_FLAG_##flag))

struct _Camera
{
  Component component;
  uint32_t flags;
  CoglFramebuffer *fb;		/* framebuffer to draw to */
  CoglColor background_color;	/* clear color */
  float fov;                    /* perspective */
  float size;                   /* orthographic */
  float z_near, z_far;
};

Component *       es_camera_new                   (void);
void              es_camera_free		  (Camera *camera);
CoglFramebuffer * es_camera_get_framebuffer       (Camera *camera);
void              es_camera_set_framebuffer       (Camera          *camera,
                                                   CoglFramebuffer *fb);
void	          es_camera_set_near_plane        (Camera *camera,
                                                   float   z_near);
void	          es_camera_set_far_plane         (Camera *camera,
                                                   float    z_far);
EsProjection      es_camera_get_projection        (Camera *camera);
void	          es_camera_set_projection        (Camera       *camera,
                                                   EsProjection  projection);
void	          es_camera_set_field_of_view     (Camera  *camera,
                                                   float    fov);
void              es_camera_set_size_of_view      (Camera *camera,
                                                   float   sov);
void	          es_camera_set_background_color  (Camera    *camera,
                                                   CoglColor *color);

#endif /* __ES_CAMERA_H__ */
