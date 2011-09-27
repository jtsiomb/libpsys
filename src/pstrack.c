#include "pstrack.h"

int psys_init_track(struct psys_track *track)
{
	track->cache_tm = ANM_TIME_INVAL;

	if(anm_init_track(&track->trk) == -1) {
		return -1;
	}
	return 0;
}

void psys_destroy_track(struct psys_track *track)
{
	anm_destroy_track(&track->trk);
}

int psys_init_track3(struct psys_track3 *track)
{
	track->cache_tm = ANM_TIME_INVAL;

	if(anm_init_track(&track->x) == -1) {
		return -1;
	}
	if(anm_init_track(&track->y) == -1) {
		anm_destroy_track(&track->x);
		return -1;
	}
	if(anm_init_track(&track->z) == -1) {
		anm_destroy_track(&track->x);
		anm_destroy_track(&track->z);
		return -1;
	}
	return 0;
}

void psys_destroy_track3(struct psys_track3 *track)
{
	anm_destroy_track(&track->x);
	anm_destroy_track(&track->y);
	anm_destroy_track(&track->z);
}

void psys_eval_track(struct psys_track *track, anm_time_t tm)
{
	if(track->cache_tm != tm) {
		track->cache_tm = tm;
		track->cache_val = anm_get_value(&track->trk, tm);
	}
}

void psys_set_value(struct psys_track *track, anm_time_t tm, float v)
{
	anm_set_value(&track->trk, tm, v);
	track->cache_tm = ANM_TIME_INVAL;
}

float psys_get_value(struct psys_track *track, anm_time_t tm)
{
	psys_eval_track(track, tm);
	return track->cache_val;
}

float psys_get_cur_value(struct psys_track *track)
{
	return track->cache_val;
}


void psys_eval_track3(struct psys_track3 *track, anm_time_t tm)
{
	if(track->cache_tm != tm) {
		track->cache_tm = tm;
		track->cache_vec.x = anm_get_value(&track->x, tm);
		track->cache_vec.y = anm_get_value(&track->y, tm);
		track->cache_vec.z = anm_get_value(&track->z, tm);
	}
}

void psys_set_value3(struct psys_track3 *track, anm_time_t tm, vec3_t v)
{
	anm_set_value(&track->x, tm, v.x);
	anm_set_value(&track->y, tm, v.y);
	anm_set_value(&track->z, tm, v.z);
	track->cache_tm = ANM_TIME_INVAL;
}

vec3_t psys_get_value3(struct psys_track3 *track, anm_time_t tm)
{
	psys_eval_track3(track, tm);
	return track->cache_vec;
}

vec3_t psys_get_cur_value3(struct psys_track3 *track)
{
	return track->cache_vec;
}
