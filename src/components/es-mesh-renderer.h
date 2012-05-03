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
