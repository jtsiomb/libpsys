#ifndef PATTR_H_
#define PATTR_H_

#include "pstrack.h"
#include "rndval.h"

/* the particle attributes vary from 0 to 1 during its lifetime */
struct psys_particle_attributes {
	struct psys_track3 color;
	struct psys_track alpha;
	struct psys_track size;
};

struct psys_attributes {
	unsigned int tex;	/* OpenGL texture to use for the billboard */

	struct psys_track3 spawn_range;	/* radius of emmiter */
	struct psys_track rate;			/* spawn rate particles per sec */
	struct psys_anm_rnd life;		/* particle life in seconds */
	struct psys_anm_rnd size;		/* base particle size */
	struct psys_anm_rnd3 dir;		/* particle shoot direction */

	struct psys_track3 grav;		/* external force (usually gravity) */
	float drag;	/* I don't think this needs to animate */

	/* particle attributes */
	struct psys_particle_attributes part_attr;

	/* limits */
	int max_particles;
};

#ifdef __cplusplus
extern "C" {
#endif

void psys_texture_loader(unsigned int (*load)(const char*, void*), void (*unload)(unsigned int, void*), void *cls);

struct psys_attributes *psys_create_attr(void);
void psys_free_attr(struct psys_attributes *attr);

int psys_init_attr(struct psys_attributes *attr);
void psys_destroy_attr(struct psys_attributes *attr);

/* copies particle system attributes src to dest
 * XXX: dest must have been initialized first
 */
void psys_copy_attr(struct psys_attributes *dest, const struct psys_attributes *src);

void psys_eval_attr(struct psys_attributes *attr, anm_time_t tm);

int psys_load_attr(struct psys_attributes *attr, const char *fname);
int psys_load_attr_stream(struct psys_attributes *attr, FILE *fp);

int psys_save_attr(const struct psys_attributes *attr, const char *fname);
int psys_save_attr_stream(const struct psys_attributes *attr, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif	/* PATTR_H_ */
