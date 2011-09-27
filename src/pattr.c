#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "pattr.h"
#include "psys_gl.h"

static void *tex_cls;
static unsigned int (*load_texture)(const char*, void*) = psys_gl_load_texture;
static void (*unload_texture)(unsigned int, void*) = psys_gl_unload_texture;

void psys_texture_loader(unsigned int (*load)(const char*, void*), void (*unload)(unsigned int, void*), void *cls)
{
	load_texture = load;
	unload_texture = unload;
	tex_cls = cls;
}

int psys_init_attr(struct psys_attributes *attr)
{
	memset(attr, 0, sizeof *attr);

	if(psys_init_track3(&attr->spawn_range) == -1)
		goto err;
	if(psys_init_track(&attr->rate) == -1)
		goto err;
	if(psys_init_anm_rnd(&attr->life) == -1)
		goto err;
	if(psys_init_anm_rnd(&attr->size) == -1)
		goto err;
	if(psys_init_anm_rnd3(&attr->dir) == -1)
		goto err;
	if(psys_init_track3(&attr->grav) == -1)
		goto err;

	attr->max_particles = -1;
	return 0;

err:
	psys_destroy_attr(attr);
	return -1;
}

void psys_destroy_attr(struct psys_attributes *attr)
{
	psys_destroy_track3(&attr->spawn_range);
	psys_destroy_track(&attr->rate);
	psys_destroy_anm_rnd(&attr->life);
	psys_destroy_anm_rnd(&attr->size);
	psys_destroy_anm_rnd3(&attr->dir);
	psys_destroy_track3(&attr->grav);

	if(attr->tex && unload_texture) {
		unload_texture(attr->tex, tex_cls);
	}
}

void psys_eval_attr(struct psys_attributes *attr, anm_time_t tm)
{
	psys_eval_track3(&attr->spawn_range, tm);
	psys_eval_track(&attr->rate, tm);
	psys_eval_anm_rnd(&attr->life, tm);
	psys_eval_anm_rnd(&attr->size, tm);
	psys_eval_anm_rnd3(&attr->dir, tm);
	psys_eval_track3(&attr->grav, tm);
}

int psys_load_attr(struct psys_attributes *attr, const char *fname)
{
	FILE *fp;
	int res;

	if(!(fp = fopen(fname, "r"))) {
		fprintf(stderr, "%s: failed to read file: %s: %s\n", __func__, fname, strerror(errno));
		return -1;
	}
	res = psys_load_attr_stream(attr, fp);
	fclose(fp);
	return res;
}

int psys_load_attr_stream(struct psys_attributes *attr, FILE *fp)
{
	return -1;	/* TODO */
}


int psys_save_attr(struct psys_attributes *attr, const char *fname)
{
	FILE *fp;
	int res;

	if(!(fp = fopen(fname, "w"))) {
		fprintf(stderr, "%s: failed to write file: %s: %s\n", __func__, fname, strerror(errno));
		return -1;
	}
	res = psys_save_attr_stream(attr, fp);
	fclose(fp);
	return res;
}

int psys_save_attr_stream(struct psys_attributes *attr, FILE *fp)
{
	return -1;	/* TODO */
}
