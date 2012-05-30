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

#ifndef __ES_LIGHT_H__
#define __ES_LIGHT_H__

#include "es-entity.h"

typedef struct _Light Light;

#define ES_LIGHT(p) ((Light *)(p))

#define LIGHT_HAS_FLAG(clip,flag)    \
    ((clip)->flags & LIGHT_FLAG_##flag)

#define LIGHT_SET_FLAG(clip,flag)    \
    ((clip)->flags |= LIGHT_FLAG_##flag)

#define LIGHT_CLEAR_FLAG(clip,flag)  \
    ((clip)->flags &= ~(LIGHT_FLAG_##flag))

struct _Light
{
  Component component;
  uint32_t flags;
  CoglColor ambient;
  CoglColor diffuse;
  CoglColor specular;
};

Component * es_light_new             (void);

void        es_light_free            (Light *light);

void        es_light_set_ambient     (Light     *light,
                                      CoglColor *ambient);
void        es_light_set_diffuse     (Light     *light,
                                      CoglColor *diffuse);
void        es_light_set_specular     (Light     *light,
                                      CoglColor *specular);

#endif /* __ES_LIGHT_H__ */
