#ifndef RAYCASTER_RENDERER_H
#define RAYCASTER_RENDERER_H

struct raycaster_renderer;

struct raycaster_renderer *renderer_create(int width, int height, double aspect, double fov, int pixelation, double wallheight);
void renderer_clear();
void renderer_set_dimensions(struct raycaster_renderer *renderer, int width, int height);
void renderer_set_fov(struct raycaster_renderer *renderer, double fov);
void renderer_set_pixelation(struct raycaster_renderer *renderer, int pixelation);
void renderer_set_wallheight(struct raycaster_renderer *renderer, double wallheight);
void renderer_draw_floorceiling(struct raycaster_renderer *renderer, const int *const map, int map_width, int map_height, double camera_x, double camera_y, double camera_r);
void renderer_draw_walls(struct raycaster_renderer *renderer, const int *const map, int map_width, int map_height, double camera_x, double camera_y, double camera_r);
void renderer_destroy(struct raycaster_renderer *renderer);

#endif
