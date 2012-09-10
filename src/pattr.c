#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#ifdef _MSC_VER
#include <malloc.h>
#else
#include <alloca.h>
#endif

#include "pattr.h"
#include "psys_gl.h"

enum {
	OPT_STR,
	OPT_NUM,
	OPT_NUM_RANGE,
	OPT_VEC,
	OPT_VEC_RANGE
};

struct cfgopt {
	char *name;
	int type;
	long tm;
	char *valstr;
	vec3_t val, valrng;
};

static int init_particle_attr(struct psys_particle_attributes *pattr);
static void destroy_particle_attr(struct psys_particle_attributes *pattr);
static struct cfgopt *get_cfg_opt(const char *line);
static void release_cfg_opt(struct cfgopt *opt);
static char *stripspace(char *str);

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

	if(init_particle_attr(&attr->part_attr) == -1)
		goto err;

	attr->max_particles = -1;

	anm_set_track_default(&attr->size.value.trk, 1.0);
	anm_set_track_default(&attr->life.value.trk, 1.0);

	return 0;

err:
	psys_destroy_attr(attr);
	return -1;
}


static int init_particle_attr(struct psys_particle_attributes *pattr)
{
	if(psys_init_track3(&pattr->color) == -1) {
		return -1;
	}
	if(psys_init_track(&pattr->alpha) == -1) {
		psys_destroy_track3(&pattr->color);
		return -1;
	}
	if(psys_init_track(&pattr->size) == -1) {
		psys_destroy_track3(&pattr->color);
		psys_destroy_track(&pattr->alpha);
		return -1;
	}

	anm_set_track_default(&pattr->color.x, 1.0);
	anm_set_track_default(&pattr->color.y, 1.0);
	anm_set_track_default(&pattr->color.z, 1.0);
	anm_set_track_default(&pattr->alpha.trk, 1.0);
	anm_set_track_default(&pattr->size.trk, 1.0);
	return 0;
}


void psys_destroy_attr(struct psys_attributes *attr)
{
	psys_destroy_track3(&attr->spawn_range);
	psys_destroy_track(&attr->rate);
	psys_destroy_anm_rnd(&attr->life);
	psys_destroy_anm_rnd(&attr->size);
	psys_destroy_anm_rnd3(&attr->dir);
	psys_destroy_track3(&attr->grav);

	destroy_particle_attr(&attr->part_attr);

	if(attr->tex && unload_texture) {
		unload_texture(attr->tex, tex_cls);
	}
}

static void destroy_particle_attr(struct psys_particle_attributes *pattr)
{
	psys_destroy_track3(&pattr->color);
	psys_destroy_track(&pattr->alpha);
	psys_destroy_track(&pattr->size);
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
	int lineno = 0;
	char buf[512];
	struct cfgopt *opt = 0;

	psys_init_attr(attr);

	while(fgets(buf, sizeof buf, fp)) {

		lineno++;

		if(!(opt = get_cfg_opt(buf))) {
			continue;
		}

		if(strcmp(opt->name, "texture") == 0) {
			if(opt->type != OPT_STR) {
				goto err;
			}
			if(!(attr->tex = load_texture(opt->valstr, tex_cls))) {
				fprintf(stderr, "failed to load texture: %s\n", opt->valstr);
				goto err;
			}

			release_cfg_opt(opt);
			continue;
		} else if(opt->type == OPT_STR) {
			fprintf(stderr, "invalid particle config: '%s'\n", opt->name);
			goto err;
		}

		if(strcmp(opt->name, "spawn_range") == 0) {
			psys_set_value3(&attr->spawn_range, opt->tm, opt->val);
		} else if(strcmp(opt->name, "rate") == 0) {
			psys_set_value(&attr->rate, opt->tm, opt->val.x);
		} else if(strcmp(opt->name, "life") == 0) {
			psys_set_anm_rnd(&attr->life, opt->tm, opt->val.x, opt->valrng.x);
		} else if(strcmp(opt->name, "size") == 0) {
			psys_set_anm_rnd(&attr->size, opt->tm, opt->val.x, opt->valrng.x);
		} else if(strcmp(opt->name, "dir") == 0) {
			psys_set_anm_rnd3(&attr->dir, opt->tm, opt->val, opt->valrng);
		} else if(strcmp(opt->name, "grav") == 0) {
			psys_set_value3(&attr->grav, opt->tm, opt->val);
		} else if(strcmp(opt->name, "drag") == 0) {
			attr->drag = opt->val.x;
		} else if(strcmp(opt->name, "pcolor") == 0) {
			psys_set_value3(&attr->part_attr.color, opt->tm, opt->val);
		} else if(strcmp(opt->name, "palpha") == 0) {
			psys_set_value(&attr->part_attr.alpha, opt->tm, opt->val.x);
		} else if(strcmp(opt->name, "psize") == 0) {
			psys_set_value(&attr->part_attr.size, opt->tm, opt->val.x);
		} else {
			fprintf(stderr, "unrecognized particle config option: %s\n", opt->name);
			goto err;
		}

		release_cfg_opt(opt);
	}

	return 0;

err:
	fprintf(stderr, "Line %d: error parsing particle definition\n", lineno);
	release_cfg_opt(opt);
	return -1;
}

static struct cfgopt *get_cfg_opt(const char *line)
{
	char *buf, *tmp;
	struct cfgopt *opt;

	line = stripspace((char*)line);
	if(line[0] == '#' || !line[0]) {
		return 0;	/* skip empty lines and comments */
	}

	if(!(opt = malloc(sizeof *opt))) {
		return 0;
	}

	if(!(opt->valstr = strchr(line, '='))) {
		release_cfg_opt(opt);
		return 0;
	}
	*opt->valstr++ = 0;
	opt->valstr = stripspace(opt->valstr);

	/* allocate a working buffer on the stack that could fit the current line */
	buf = alloca(strlen(line) + 1);
	strcpy(buf, line);
	buf = stripspace(buf);

	/* parse the keyframe time specifier if it exists */
	if((tmp = strchr(buf, '('))) {
		char *endp;
		float tval;

		*tmp++ = 0;
		opt->name = strdup(buf);

		tval = strtod(tmp + 1, &endp);
		if(endp == tmp + 1) { /* nada ... */
			opt->tm = 0;
		} else if(*endp == 's') {	/* seconds suffix */
			opt->tm = (long)(tval * 1000.0f);
		} else {
			opt->tm = (long)tval;
		}
	} else {
		opt->name = strdup(buf);
		opt->tm = 0;
	}

	if(sscanf(opt->valstr, "[%f %f %f] ~ [%f %f %f]", &opt->val.x, &opt->val.y, &opt->val.z,
				&opt->valrng.x, &opt->valrng.y, &opt->valrng.z) == 6) {
		/* value is a vector range */
		opt->type = OPT_VEC_RANGE;

	} else if(sscanf(opt->valstr, "%f ~ %f", &opt->val.x, &opt->valrng.x) == 2) {
		/* value is a number range */
		opt->type = OPT_NUM_RANGE;
		opt->val.y = opt->val.z = opt->val.x;
		opt->valrng.y = opt->valrng.z = opt->valrng.x;

	} else if(sscanf(opt->valstr, "[%f %f %f]", &opt->val.x, &opt->val.y, &opt->val.z) == 3) {
		/* value is a vector */
		opt->type = OPT_VEC;
		opt->valrng.x = opt->valrng.y = opt->valrng.z = 0.0f;

	} else if(sscanf(opt->valstr, "%f", &opt->val.x) == 1) {
		/* value is a number */
		opt->type = OPT_NUM;
		opt->val.y = opt->val.z = opt->val.x;
		opt->valrng.x = opt->valrng.y = opt->valrng.z = 0.0f;

	} else if(sscanf(opt->valstr, "\"%s\"", buf) == 1) {
		/* just a string... strip the quotes */
		if(buf[strlen(buf) - 1] == '\"') {
			buf[strlen(buf) - 1] = 0;
		}
		opt->type = OPT_STR;
		opt->valstr = strdup(buf);
	} else {
		/* fuck it ... */
		release_cfg_opt(opt);
		return 0;
	}

	return opt;
}

static void release_cfg_opt(struct cfgopt *opt)
{
	if(opt) {
		free(opt->name);
	}
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


static char *stripspace(char *str)
{
	char *end;

	while(*str && isspace(*str)) {
		str++;
	}

	end = str + strlen(str) - 1;
	while(end >= str && isspace(*end)) {
		*end-- = 0;
	}
	return str;
}
