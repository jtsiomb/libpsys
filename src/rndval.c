/*
libpsys - reusable particle system library.
Copyright (C) 2011-2014  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdlib.h>
#include "rndval.h"

int psys_init_anm_rnd(struct psys_anm_rnd *r)
{
	if(psys_init_track(&r->value) == -1) {
		return -1;
	}
	if(psys_init_track(&r->range) == -1) {
		psys_destroy_track(&r->value);
		return -1;
	}
	return 0;
}

void psys_destroy_anm_rnd(struct psys_anm_rnd *r)
{
	psys_destroy_track(&r->value);
	psys_destroy_track(&r->range);
}

int psys_init_anm_rnd3(struct psys_anm_rnd3 *r)
{
	if(psys_init_track3(&r->value) == -1) {
		return -1;
	}
	if(psys_init_track3(&r->range) == -1) {
		psys_destroy_track3(&r->value);
		return -1;
	}
	return 0;
}

void psys_destroy_anm_rnd3(struct psys_anm_rnd3 *r)
{
	psys_destroy_track3(&r->value);
	psys_destroy_track3(&r->range);
}

void psys_copy_anm_rnd(struct psys_anm_rnd *dest, const struct psys_anm_rnd *src)
{
	psys_copy_track(&dest->value, &src->value);
	psys_copy_track(&dest->range, &src->range);
}

void psys_copy_anm_rnd3(struct psys_anm_rnd3 *dest, const struct psys_anm_rnd3 *src)
{
	psys_copy_track3(&dest->value, &src->value);
	psys_copy_track3(&dest->range, &src->range);
}

void psys_set_rnd(struct psys_rnd *r, float val, float range)
{
	r->value = val;
	r->range = range;
}

void psys_set_rnd3(struct psys_rnd3 *r, vec3_t val, vec3_t range)
{
	r->value = val;
	r->range = range;
}

void psys_set_anm_rnd(struct psys_anm_rnd *r, anm_time_t tm, float val, float range)
{
	psys_set_value(&r->value, tm, val);
	psys_set_value(&r->range, tm, range);
}

void psys_set_anm_rnd3(struct psys_anm_rnd3 *r, anm_time_t tm, vec3_t val, vec3_t range)
{
	psys_set_value3(&r->value, tm, val);
	psys_set_value3(&r->range, tm, range);
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
	struct psys_rnd tmp;
	if(tm == ANM_TIME_INVAL) {
		tmp.value = psys_get_cur_value(&r->value);
		tmp.range = psys_get_cur_value(&r->range);
	} else {
		tmp.value = psys_get_value(&r->value, tm);
		tmp.range = psys_get_value(&r->range, tm);
	}
	return psys_eval_rnd(&tmp);
}

vec3_t psys_eval_anm_rnd3(struct psys_anm_rnd3 *r, anm_time_t tm)
{
	struct psys_rnd3 tmp;
	if(tm == ANM_TIME_INVAL) {
		tmp.value = psys_get_cur_value3(&r->value);
		tmp.range = psys_get_cur_value3(&r->range);
	} else {
		tmp.value = psys_get_value3(&r->value, tm);
		tmp.range = psys_get_value3(&r->range, tm);
	}
	return psys_eval_rnd3(&tmp);
}
