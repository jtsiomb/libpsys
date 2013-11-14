#ifndef PSTRACK_H_
#define PSTRACK_H_

#include <vmath/vmath.h>
#include <anim/anim.h>

struct psys_track {
	struct anm_track trk;

	anm_time_t cache_tm;
	float cache_val;
};

struct psys_track3 {
	struct anm_track x, y, z;

	anm_time_t cache_tm;
	vec3_t cache_vec;
};

#ifdef __cplusplus
extern "C" {
#endif

int psys_init_track(struct psys_track *track);
void psys_destroy_track(struct psys_track *track);

int psys_init_track3(struct psys_track3 *track);
void psys_destroy_track3(struct psys_track3 *track);

/* XXX dest must have been initialized first */
void psys_copy_track(struct psys_track *dest, const struct psys_track *src);
void psys_copy_track3(struct psys_track3 *dest, const struct psys_track3 *src);

void psys_eval_track(struct psys_track *track, anm_time_t tm);
void psys_set_value(struct psys_track *track, anm_time_t tm, float v);
float psys_get_value(struct psys_track *track, anm_time_t tm);
float psys_get_cur_value(struct psys_track *track);

void psys_eval_track3(struct psys_track3 *track, anm_time_t tm);
void psys_set_value3(struct psys_track3 *track, anm_time_t tm, vec3_t v);
vec3_t psys_get_value3(struct psys_track3 *track, anm_time_t tm);
vec3_t psys_get_cur_value3(struct psys_track3 *track);

#ifdef __cplusplus
}
#endif

#endif	/* PSTRACK_H_ */
