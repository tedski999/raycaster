#ifndef RC_ENTITY_H
#define RC_ENTITY_H

struct rc_texture;
struct rc_map;
struct rc_entity;

typedef void (*entity_init_func)(struct rc_entity *entity);
typedef void (*entity_update_func)(struct rc_entity *entity, struct rc_map *map);
typedef void (*entity_destroy_func)(struct rc_entity *entity);

struct rc_entity *rc_entity_create(double x, double y, double z, double r, const struct rc_texture *texture, entity_init_func init_function, entity_update_func update_function, entity_destroy_func destroy_function);
void rc_entity_set_transform(struct rc_entity *entity, double x, double y, double z, double r);
void rc_entity_get_transform(const struct rc_entity *entity, double *x, double *y, double *z, double *r);
const struct rc_texture *rc_entity_get_texture(const struct rc_entity *entity);
void rc_entity_set_data_pointer(struct rc_entity *entity, void *data_pointer);
void *rc_entity_get_data_pointer(const struct rc_entity *entity);
void rc_entity_update(struct rc_entity *entity, struct rc_map *map);
void rc_entity_destroy(struct rc_entity *entity);

#endif
