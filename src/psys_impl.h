#ifndef PSYS_IMPL_H_
#define PSYS_IMPL_H_

#include <anim/anim.h>
#include "psys.h"

struct v3track {
	struct anm_track x, y, z;
};

struct col_plane {
	plane_t p;
	float elasticity;
	struct col_plane *next;
};

struct psys_particle;

struct psys_emitter {
	float last_update;

	struct anm_node prs;

	unsigned int tex;

	struct anm_track rate;
	struct anm_track life;
	struct v3track dir;
	struct v3track grav;

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
	vec3_t cur_pos;
	quat_t cur_rot;
	float cur_rate, cur_life;
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

void psys_gl_draw_start(struct psys_emitter *em, void *cls);
void psys_gl_draw(struct psys_emitter *em, struct psys_particle *p, void *cls);
void psys_gl_draw_end(struct psys_emitter *em, void *cls);

#endif	/* PSYS_IMPL_H_ */
