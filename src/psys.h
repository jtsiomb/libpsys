#ifndef LIBPSYS_H_
#define LIBPSYS_H_

#include <vmath.h>
#include <anim/anim.h>
#include "rndval.h"

struct anm_track_vec3 {
	struct anm_track x, y, z;
};

struct psys_plane {
	plane_t p;
	float elasticity;
	struct psys_plane *next;
};

struct psys_particle;

struct psys_emitter {
	float last_update;

	unsigned int tex;

	struct anm_node prs;
	struct anm_track_vec3 pos_range;

	struct anm_track rate;
	struct anm_track life, life_range;
	struct anm_track size, size_range;
	struct anm_track_vec3 dir, dir_range;
	struct anm_track_vec3 grav;

	float drag;	/* I don't think this needs to animate */

	/* list of active particles */
	struct psys_particle *plist;

	/* list of collision planes */
	struct col_plane *planes;

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

	/* calculated on update */
	vec3_t cur_pos, cur_pos_range;
	quat_t cur_rot;
	float cur_rate, cur_life;
	float cur_size, cur_size_range;
	vec3_t cur_dir;
	vec3_t cur_grav;

	/* partial spawn accumulator */
	float spawn_acc;
};


struct psys_particle {
	vec3_t pos, vel;
	float life, size;

	struct psys_particle *next;
};


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

void psys_set_pos(struct psys_emitter *em, vec3_t pos, vec3_t range, float tm);
void psys_set_rot(struct psys_emitter *em, quat_t rot, float tm);
void psys_set_pivot(struct psys_emitter *em, vec3_t pivot);

void psys_set_rate(struct psys_emitter *em, float rate, float tm);
void psys_set_life(struct psys_emitter *em, float life, float range, float tm);
void psys_set_size(struct psys_emitter *em, float size, float range, float tm);
void psys_set_dir(struct psys_emitter *em, vec3_t dir, vec3_t range, float tm);
void psys_set_grav(struct psys_emitter *em, vec3_t grav, float tm);

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

#endif	/* LIBPSYS_H_ */
