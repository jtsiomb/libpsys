#ifndef LIBPSYS_H_
#define LIBPSYS_H_

#include <vmath/vmath.h>
#include <anim/anim.h>
#include "rndval.h"
#include "pattr.h"

struct psys_particle;
struct psys_emitter;

typedef int (*psys_spawn_func_t)(struct psys_emitter*, struct psys_particle*, void*);
typedef void (*psys_update_func_t)(struct psys_emitter*, struct psys_particle*, float, float, void*);

typedef void (*psys_draw_func_t)(struct psys_emitter*, struct psys_particle*, void*);
typedef void (*psys_draw_start_func_t)(struct psys_emitter*, void*);
typedef void (*psys_draw_end_func_t)(struct psys_emitter*, void*);


struct psys_plane {
	plane_t p;
	float elasticity;
	struct psys_plane *next;
};


struct psys_emitter {
	struct anm_node prs;
	vec3_t cur_pos;

	struct psys_attributes attr;

	/* list of active particles */
	struct psys_particle *plist;
	int pcount;	/* number of active particles */

	/* list of collision planes */
	struct psys_plane *planes;

	/* custom spawn closure */
	void *spawn_cls;
	psys_spawn_func_t spawn;

	/* custom particle update closure */
	void *upd_cls;
	psys_update_func_t update;

	/* custom draw closure */
	void *draw_cls;
	psys_draw_func_t draw;
	psys_draw_start_func_t draw_start;
	psys_draw_end_func_t draw_end;

	float spawn_acc;	/* partial spawn accumulator */
	float last_update;	/* last update time (to calc dt) */
};


struct psys_particle {
	vec3_t pos, vel;
	float life, max_life;
	float base_size;

	struct psys_particle_attributes *pattr;

	/* current particle attr values calculated during update */
	vec3_t color;
	float alpha, size;

	struct psys_particle *next;
};

#ifdef __cplusplus
extern "C" {
#endif

struct psys_emitter *psys_create(void);
void psys_free(struct psys_emitter *em);

int psys_init(struct psys_emitter *em);
void psys_destroy(struct psys_emitter *em);

/* set properties */
void psys_set_pos(struct psys_emitter *em, vec3_t pos, float tm);
void psys_set_rot(struct psys_emitter *em, quat_t rot, float tm);
void psys_set_pivot(struct psys_emitter *em, vec3_t pivot);

vec3_t psys_get_pos(struct psys_emitter *em, float tm);
quat_t psys_get_rot(struct psys_emitter *em, float tm);
vec3_t psys_get_pivot(struct psys_emitter *em);

void psys_clear_collision_planes(struct psys_emitter *em);
int psys_add_collision_plane(struct psys_emitter *em, plane_t plane, float elast);

void psys_add_particle(struct psys_emitter *em, struct psys_particle *p);

void psys_spawn_func(struct psys_emitter *em, psys_spawn_func_t func, void *cls);
void psys_update_func(struct psys_emitter *em, psys_update_func_t func, void *cls);
void psys_draw_func(struct psys_emitter *em, psys_draw_func_t draw,
		psys_draw_start_func_t start, psys_draw_end_func_t end, void *cls);

/* update and render */

void psys_update(struct psys_emitter *em, float tm);
void psys_draw(struct psys_emitter *em);

#ifdef __cplusplus
}
#endif

#endif	/* LIBPSYS_H_ */
