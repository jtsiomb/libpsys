#ifndef RNDVAL_H_
#define RNDVAL_H_

#include <vmath/vmath.h>
#include "pstrack.h"

struct psys_rnd {
	float value, range;
};

struct psys_rnd3 {
	vec3_t value, range;
};

struct psys_anm_rnd {
	struct psys_track value, range;
};

struct psys_anm_rnd3 {
	struct psys_track3 value, range;
};

#define PSYS_EVAL_CUR	ANM_TIME_INVAL

int psys_init_anm_rnd(struct psys_anm_rnd *v);
void psys_destroy_anm_rnd(struct psys_anm_rnd *v);
int psys_init_anm_rnd3(struct psys_anm_rnd3 *v);
void psys_destroy_anm_rnd3(struct psys_anm_rnd3 *v);

void psys_set_rnd(struct psys_rnd *r, float val, float range);
void psys_set_rnd3(struct psys_rnd3 *r, vec3_t val, vec3_t range);

void psys_set_anm_rnd(struct psys_anm_rnd *r, anm_time_t tm, float val, float range);
void psys_set_anm_rnd3(struct psys_anm_rnd3 *r, anm_time_t tm, vec3_t val, vec3_t range);

float psys_eval_rnd(struct psys_rnd *r);
vec3_t psys_eval_rnd3(struct psys_rnd3 *r);

float psys_eval_anm_rnd(struct psys_anm_rnd *r, anm_time_t tm);
vec3_t psys_eval_anm_rnd3(struct psys_anm_rnd3 *r, anm_time_t tm);


#endif	/* RNDVAL_H_ */
