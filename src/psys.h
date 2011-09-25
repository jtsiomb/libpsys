#ifndef LIBPSYS_H_
#define LIBPSYS_H_

struct psys_emitter;
struct psys_particle;

typedef int (*psys_spawn_func_t)(struct psys_emitter*, struct psys_particle*, void*);
typedef void (*psys_update_func_t)(struct psys_emitter*, struct psys_particle*, float, float, void*);

typedef void (*psys_draw_func_t)(struct psys_emitter*, struct psys_particle*, void*);
typedef void (*psys_draw_start_func_t)(struct psys_emitter*, void*);
typedef void (*psys_draw_end_func_t)(struct psys_emitter*, void*);


struct psys_emitter *psys_create(void);
void psys_free(struct psys_emitter *em);

int psys_init(struct psys_emitter *em);
void psys_destroy(struct psys_emitter *em);

/* set properties */
void psys_set_texture(struct psys_emitter *em, unsigned int tex);

void psys_set_pos(struct psys_emitter *em, vec3_t pos, float tm);
void psys_set_rot(struct psys_emitter *em, quat_t rot, float tm);
void psys_set_pivot(struct psys_emitter *em, vec3_t pivot);

void psys_set_rate(struct psys_emitter *em, float rate, float tm);
void psys_set_life(struct psys_emitter *em, float life, float tm);
void psys_set_dir(struct psys_emitter *em, vec3_t dir, float tm);
void psys_set_grav(struct psys_emitter *em, vec3_t grav, float tm);

void psys_clear_collision_planes(struct psys_emitter *em);
int psys_add_collision_plane(struct psys_emitter *em, plane_t plane, float elast);

void psys_add_particle(struct psys_emitter *em, struct psys_particle *p);

void psys_spawn_func(struct psys_emitter *em, psys_spawn_func_t func, void *cls);
void psys_update_func(struct psys_emitter *em, psys_update_func_t func, void *cls);
void psys_draw_func(struct psys_emitter *em, psys_draw_func_t draw,
		psys_draw_start_func_t start, psys_draw_end_func_t end, void *cls);


/* query emitter state */
unsigned int psys_get_texture(struct psys_emitter *em);
vec3_t psys_get_pos(struct psys_emitter *em);
quat_t psys_get_rot(struct psys_emitter *em);
float psys_get_rate(struct psys_emitter *em);
float psys_get_life(struct psys_emitter *em);
vec3_t psys_get_dir(struct psys_emitter *em);
vec3_t psys_get_grav(struct psys_emitter *em);

/* update and render */

void psys_update(struct psys_emitter *em, float tm);
void psys_draw(struct psys_emitter *em);

#endif	/* LIBPSYS_H_ */
