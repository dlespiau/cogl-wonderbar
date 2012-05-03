#ifndef __ES_ENTITY_H__
#define __ES_ENTITY_H__

#include <stdint.h>

#include <cogl/cogl.h>

typedef struct _component Component;
typedef struct _entity    Entity;

struct _component
{
  void (*update)  (Component *component, uint64_t time);
  void (*draw)    (Component *component, Entity *entity);
};

typedef enum
{
  ENTITY_FLAG_NONE,
  ENTITY_FLAG_DIRTY
} EntityFlag;

#define ENTITY_HAS_FLAG(entity,flag)    ((entity)->flags & ENTITY_FLAG_##flag)
#define ENTITY_SET_FLAG(entity,flag)    ((entity)->flags |= ENTITY_FLAG_##flag)
#define ENTITY_CLEAR_FLAG(entity,flag)  ((entity)->flags &= ~(ENTITY_FLAG_##flag))

#define entity_is_dirty(entity)         (ENTITY_HAS_FLAG (entity, DIRTY))
#define entity_set_dirty(entity)        (ENTITY_SET_FLAG (entity, DIRTY))
#define entity_clear_dirty(entity)      (ENTITY_CLEAR_FLAG (entity, DIRTY))

/* FIXME:
 *  - directly store the position in the transform matrix?
 */
struct _entity
{
  /* private fields */
  uint32_t flags;
  struct { float x, y, z; } position;
  CoglQuaternion rotation;
  CoglMatrix transform;
  GPtrArray *components;
};

#define es_entity_get_x(entity) (entity->position.x)
#define es_entity_get_y(entity) (entity->position.y)
#define es_entity_get_z(entity) (entity->position.z)

void                es_entity_init          (Entity *entity);
const CoglMatrix *  es_entity_get_transform (Entity *entity);
void                es_entity_add_component (Entity    *entity,
                                             Component *component);
void                es_entity_draw          (Entity *entity);
void                es_entity_translate     (Entity *entity,
                                             float   tx,
                                             float   tz,
                                             float   ty);
void                es_entity_rotate_x_axis (Entity *entity,
                                             float   x_angle);

#endif /* __ES_ENTITY_H__ */
