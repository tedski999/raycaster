#ifndef RAYCASTER_ENTITY_H
#define RAYCASTER_ENTITY_H

#include "texture.h"
#include "map.h"

struct raycaster_entity;

typedef void (*entity_init_func)(struct raycaster_entity *entity);
typedef void (*entity_update_func)(struct raycaster_entity *entity, struct raycaster_map *map);
typedef void (*entity_destroy_func)(struct raycaster_entity *entity);

struct raycaster_entity *rc_entity_create(double x, double y, double z, double r, struct raycaster_texture *texture, entity_init_func init_function, entity_update_func update_function, entity_destroy_func destroy_function);
void rc_entity_set_transform(struct raycaster_entity *entity, double x, double y, double z, double r);
void rc_entity_get_transform(struct raycaster_entity *entity, double *x, double *y, double *z, double *r);
struct raycaster_texture *rc_entity_get_texture(struct raycaster_entity *entity);
void rc_entity_set_data_pointer(struct raycaster_entity *entity, void *data_pointer);
void *rc_entity_get_data_pointer(struct raycaster_entity *entity);
void rc_entity_update(struct raycaster_entity *entity, struct raycaster_map *map);
void rc_entity_destroy(struct raycaster_entity *entity);

#endif
