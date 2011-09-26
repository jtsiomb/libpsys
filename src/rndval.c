#include <stdlib.h>
#include "rndval.h"

int psys_init_anm_rnd(struct psys_anm_rnd *r)
{
	if(anm_init_track(&r->value) == -1) {
		return -1;
	}
	if(anm_init_track(&r->range) == -1) {
		anm_destroy_track(&r->value);
		return -1;
	}
	return 0;
}

void psys_destroy_anm_rnd(struct psys_anm_rnd *r)
{
	anm_destroy_track(&r->value);
	anm_destroy_track(&r->range);
}

int psys_init_anm_rnd3(struct psys_anm_rnd3 *r)
{
	if(anm_init_track3(&r->value) == -1) {
		return -1;
	}
	if(anm_init_track3(&r->range) == -1) {
		anm_destroy_track3(&r->value);
		return -1;
	}
	return 0;
}

void psys_destroy_anm_rnd3(struct psys_anm_rnd3 *r)
{
	anm_destroy_track3(&r->value);
	anm_destroy_track3(&r->range);
}


float psys_eval_rnd(struct psys_rnd *r)
{
	return r->value + r->range * (float)rand() / (float)RAND_MAX - 0.5 * r->range;
}

vec3_t psys_eval_rnd3(struct psys_rnd3 *r)
{
	vec3_t res;
	res.x = r->value.x + r->range.x * (float)rand() / (float)RAND_MAX - 0.5 * r->range.x;
	res.y = r->value.y + r->range.y * (float)rand() / (float)RAND_MAX - 0.5 * r->range.y;
	res.z = r->value.z + r->range.z * (float)rand() / (float)RAND_MAX - 0.5 * r->range.z;
	return res;
}


float psys_eval_anm_rnd(struct psys_anm_rnd *r, anm_time_t tm)
{
	if(r->cur_tm != tm) {
		r->cur.value = anm_get_value(&r->value, tm);
		r->cur.range = anm_get_value(&r->range, tm);
		r->cur_tm = tm;
	}
	return psys_eval_rnd(&r->cur);
}

vec3_t psys_eval_anm_rnd3(struct psys_anm_rnd3 *r, anm_time_t tm)
{
	if(r->cur_tm != tm) {
		r->cur.value = anm_get_value3(&r->value, tm);
		r->cur.range = anm_get_value3(&r->range, tm);
		r->cur_tm = tm;
	}
	return psys_eval_rnd3(&r->cur);
}
