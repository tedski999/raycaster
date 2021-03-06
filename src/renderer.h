#ifndef RAYCASTER_RENDERER_H
#define RAYCASTER_RENDERER_H

struct raycaster_renderer;
struct raycaster_window;
struct raycaster_map;
struct raycaster_entity;
struct raycaster_texture;

struct raycaster_renderer *rc_renderer_create(const struct raycaster_window *window, double aspect, int resolution, double fov, struct raycaster_texture **wall_textures);
void rc_renderer_set_dimensions(const struct raycaster_renderer *renderer, int width, int height);
void rc_renderer_set_fov(struct raycaster_renderer *renderer, double fov);
void rc_renderer_set_resolution(struct raycaster_renderer *renderer, int resolution);
void rc_renderer_set_wall_textures(struct raycaster_renderer *renderer, struct raycaster_texture **wall_textures);
void rc_renderer_draw(struct raycaster_renderer *renderer, const struct raycaster_map *map, struct raycaster_entity **entities, int entities_count, const struct raycaster_entity *camera);
void rc_renderer_destroy(struct raycaster_renderer *renderer);

#endif
