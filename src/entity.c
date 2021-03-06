#include "entity.h"
#include "error.h"
#include <stdlib.h>

struct raycaster_entity {
	double x, y, z, r;
	const struct raycaster_texture *texture;
	entity_update_func update_function;
	entity_destroy_func destroy_function;
	void *data_pointer;
};

struct raycaster_entity *rc_entity_create(double x, double y, double z, double r, const struct raycaster_texture *texture, entity_init_func init_function, entity_update_func update_function, entity_destroy_func destroy_function) {
	struct raycaster_entity *entity = malloc(sizeof *entity);
	RC_ASSERT(entity, "raycaster_entity memory allocation");
	*entity = (struct raycaster_entity) { x, y, z, r, texture, update_function, destroy_function };
	if (init_function)
		init_function(entity);
	return entity;
}

void rc_entity_set_transform(struct raycaster_entity *entity, double x, double y, double z, double r) {
	entity->x = x;
	entity->y = y;
	entity->z = z;
	entity->r = r;
}

void rc_entity_get_transform(const struct raycaster_entity *entity, double *x, double *y, double *z, double *r) {
	*x = entity->x;
	*y = entity->y;
	*z = entity->z;
	*r = entity->r;
}

// TODO: entities should have an array of textures, each representing the entity from an angle
const struct raycaster_texture *rc_entity_get_texture(const struct raycaster_entity *entity) {
	return entity->texture;
}

void rc_entity_set_data_pointer(struct raycaster_entity *entity, void *data_pointer) {
	entity->data_pointer = data_pointer;
}

void *rc_entity_get_data_pointer(const struct raycaster_entity *entity) {
	return entity->data_pointer;
}

void rc_entity_update(struct raycaster_entity *entity, struct raycaster_map *map) {
	if (entity->update_function)
		entity->update_function(entity, map);
}

void rc_entity_destroy(struct raycaster_entity *entity) {
	if (entity->destroy_function)
		entity->destroy_function(entity);
	free(entity);
}
