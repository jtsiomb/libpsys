#ifndef RNDVAL_H_
#define RNDVAL_H_

#include <vmath.h>
#include <anim/anim.h>
#include "anm_track3.h"

struct psys_rnd {
	float value, range;
};

struct psys_rnd3 {
	vec3_t value, range;
};

struct psys_anm_rnd {
	struct anm_track value, range;
	/* current cache */
	anm_time_t cur_tm;
	struct psys_rnd cur;
};

struct psys_anm_rnd3 {
	struct anm_track3 value, range;
	/* current cache */
	anm_time_t cur_tm;
	struct psys_rnd3 cur;
};


int psys_init_anm_rnd(struct psys_anm_rnd *v);
void psys_destroy_anm_rnd(struct psys_anm_rnd *v);
int psys_init_anm_rnd3(struct psys_anm_rnd3 *v);
void psys_destroy_anm_rnd3(struct psys_anm_rnd3 *v);

float psys_eval_rnd(struct psys_rnd *r);
vec3_t psys_eval_rnd3(struct psys_rnd3 *r);

float psys_eval_anm_rnd(struct psys_anm_rnd *r, anm_time_t tm);
vec3_t psys_eval_anm_rnd3(struct psys_anm_rnd3 *r, anm_time_t tm);


#endif	/* RNDVAL_H_ */
