#ifndef RC_RENDERER_H
#define RC_RENDERER_H

struct rc_renderer;
struct rc_window;
struct rc_map;
struct rc_entity;
struct rc_texture;

struct rc_renderer *rc_renderer_create(const struct rc_window *window, double aspect, int resolution, double fov, struct rc_texture **wall_textures);
void rc_renderer_set_dimensions(const struct rc_renderer *renderer, int width, int height);
void rc_renderer_set_fov(struct rc_renderer *renderer, double fov);
void rc_renderer_set_resolution(struct rc_renderer *renderer, int resolution);
void rc_renderer_set_wall_textures(struct rc_renderer *renderer, struct rc_texture **wall_textures);
void rc_renderer_draw(struct rc_renderer *renderer, const struct rc_map *map, struct rc_entity **entities, int entities_count, const struct rc_entity *camera);
void rc_renderer_destroy(struct rc_renderer *renderer);

#endif
