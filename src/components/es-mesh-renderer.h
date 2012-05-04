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

#ifndef __MESH_RENDERER_H__
#define __MESH_RENDERER_H__

#include <stdint.h>

#include <cogl/cogl.h>

#include "entity.h"
#include "mash-data-loader.h"

#define ES_MESH_RENDERER(p) ((MeshRenderer *)(p))

typedef struct _MeshRenderer MeshRenderer;

struct _MeshRenderer
{
  Component component;
  CoglPrimitive *primitive;
  MashData *mesh_data;
  CoglPipeline *pipeline;
};

Component *     es_mesh_renderer_new_from_file      (const char   *file,
                                                     CoglPipeline *pipeline);
Component *     es_mesh_renderer_new_from_template  (const char   *name,
                                                     CoglPipeline *pipeline);

void            es_mesh_renderer_free               (MeshRenderer *renderer);

void            es_mesh_renderer_set_pipeline       (MeshRenderer *renderer,
                                                     CoglPipeline *pipeline);

#endif /* __MESH_RENDERER_H__ */
