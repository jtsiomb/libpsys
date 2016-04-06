/*
libpsys - reusable particle system library.
Copyright (C) 2011-2015  John Tsiombikas <nuclear@member.fsf.org>

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
#include <math.h>
#include <float.h>
#include <assert.h>
#include "psys.h"
#include "psys_gl.h"
#include <pthread.h>

static int spawn_particle(struct psys_emitter *em, struct psys_particle *p);
static void update_particle(struct psys_emitter *em, struct psys_particle *p, float tm, float dt, void *cls);

/* particle pool */
static struct psys_particle *ppool;
static int ppool_size;
static pthread_mutex_t pool_lock = PTHREAD_MUTEX_INITIALIZER;

static struct psys_particle *palloc(void);
static void pfree(struct psys_particle *p);

/* --- constructors and shit --- */

struct psys_emitter *psys_create(void)
{
	struct psys_emitter *em;

	if(!(em = malloc(sizeof *em))) {
		return 0;
	}
	if(psys_init(em) == -1) {
		free(em);
		return 0;
	}
	return em;
}

void psys_free(struct psys_emitter *em)
{
	psys_destroy(em);
	free(em);
}

int psys_init(struct psys_emitter *em)
{
	memset(em, 0, sizeof *em);

	if(anm_init_node(&em->prs) == -1) {
		return -1;
	}
	if(psys_init_attr(&em->attr) == -1) {
		anm_destroy_node(&em->prs);
		return -1;
	}

	em->spawn = 0;	/* no custom spawning, just the defaults */
	em->update = update_particle;

	em->draw = psys_gl_draw;
	em->draw_start = psys_gl_draw_start;
	em->draw_end = psys_gl_draw_end;
	return 0;
}

void psys_destroy(struct psys_emitter *em)
{
	struct psys_particle *part;

	part = em->plist;
	while(part) {
		struct psys_particle *tmp = part;
		part = part->next;
		pfree(tmp);
	}

	psys_destroy_attr(&em->attr);
}

void psys_set_pos(struct psys_emitter *em, vec3_t pos, float tm)
{
	anm_set_position(&em->prs, pos, ANM_SEC2TM(tm));
}

void psys_set_rot(struct psys_emitter *em, quat_t rot, float tm)
{
	anm_set_rotation(&em->prs, rot, ANM_SEC2TM(tm));
}

void psys_set_pivot(struct psys_emitter *em, vec3_t pivot)
{
	anm_set_pivot(&em->prs, pivot);
}

vec3_t psys_get_pos(struct psys_emitter *em, float tm)
{
	return anm_get_node_position(&em->prs, ANM_SEC2TM(tm));
}

quat_t psys_get_rot(struct psys_emitter *em, float tm)
{
	return anm_get_node_rotation(&em->prs, ANM_SEC2TM(tm));
}

vec3_t psys_get_pivot(struct psys_emitter *em)
{
	return anm_get_pivot(&em->prs);
}

void psys_clear_collision_planes(struct psys_emitter *em)
{
	struct psys_plane *plane;

	plane = em->planes;
	while(plane) {
		struct psys_plane *tmp = plane;
		plane = plane->next;
		free(tmp);
	}
}

int psys_add_collision_plane(struct psys_emitter *em, plane_t plane, float elast)
{
	struct psys_plane *node;

	if(!(node = malloc(sizeof *node))) {
		return -1;
	}
	node->p = plane;
	node->elasticity = elast;
	node->next = em->planes;
	em->planes = node;
	return 0;
}

void psys_add_particle(struct psys_emitter *em, struct psys_particle *p)
{
	p->next = em->plist;
	em->plist = p;

	em->pcount++;
}

void psys_spawn_func(struct psys_emitter *em, psys_spawn_func_t func, void *cls)
{
	em->spawn = func;
	em->spawn_cls = cls;
}

void psys_update_func(struct psys_emitter *em, psys_update_func_t func, void *cls)
{
	em->update = func;
	em->upd_cls = cls;
}

void psys_draw_func(struct psys_emitter *em, psys_draw_func_t draw,
		psys_draw_start_func_t start, psys_draw_end_func_t end, void *cls)
{
	em->draw = draw;
	em->draw_start = start;
	em->draw_end = end;
	em->draw_cls = cls;
}

/* --- update and render --- */

void psys_update(struct psys_emitter *em, float tm)
{
	float dt, spawn_dt, spawn_tm;
	int i, spawn_count;
	struct psys_particle *p, pdummy;
	anm_time_t atm = ANM_SEC2TM(tm);

	if(!em->update) {
		fprintf(stderr, "psys_update called without an update callback\n");
		abort();
	}

	dt = tm - em->last_update;
	if(dt <= 0.0) {
		return;
	}

	psys_eval_attr(&em->attr, atm);

	/* how many particles to spawn for this interval ? */
	em->spawn_acc += psys_get_cur_value(&em->attr.rate) * dt;
	if(em->spawn_acc >= 1.0) {
		spawn_count = em->spawn_acc;
		em->spawn_acc = fmod(em->spawn_acc, 1.0);
	} else {
		spawn_count = 0;
	}

	if(spawn_count) {
		spawn_dt = dt / (float)spawn_count;
	}
	spawn_tm = em->last_update;
	for(i=0; i<spawn_count; i++) {
		if(em->attr.max_particles >= 0 && em->pcount >= em->attr.max_particles) {
			break;
		}

		/* update emitter position for this spawning */
		em->cur_pos = anm_get_position(&em->prs, ANM_SEC2TM(spawn_tm));

		if(!(p = palloc())) {
			return;
		}
		if(spawn_particle(em, p) == -1) {
			pfree(p);
		}
		spawn_tm += spawn_dt;
	}

	/* update all particles */
	p = em->plist;
	while(p) {
		em->update(em, p, tm, dt, em->upd_cls);
		p = p->next;
	}

	/* cleanup dead particles */
	pdummy.next = em->plist;
	p = &pdummy;
	while(p->next) {
		if(p->next->life <= 0) {
			struct psys_particle *tmp = p->next;
			p->next = p->next->next;
			pfree(tmp);
			em->pcount--;
		} else {
			p = p->next;
		}
	}
	em->plist = pdummy.next;

	em->last_update = tm;
}

void psys_draw(const struct psys_emitter *em)
{
	struct psys_particle *p;

	assert(em->draw);

	if(em->draw_start) {
		em->draw_start(em, em->draw_cls);
	}

	p = em->plist;
	while(p) {
		em->draw(em, p, em->draw_cls);
		p = p->next;
	}

	if(em->draw_end) {
		em->draw_end(em, em->draw_cls);
	}
}

static int spawn_particle(struct psys_emitter *em, struct psys_particle *p)
{
	struct psys_rnd3 rpos;
	rpos.value = em->cur_pos;
	rpos.range = psys_get_cur_value3(&em->attr.spawn_range);

	p->pos = psys_eval_rnd3(&rpos);
	p->vel = psys_eval_anm_rnd3(&em->attr.dir, PSYS_EVAL_CUR);
	p->base_size = psys_eval_anm_rnd(&em->attr.size, PSYS_EVAL_CUR);
	p->max_life = p->life = psys_eval_anm_rnd(&em->attr.life, PSYS_EVAL_CUR);

	p->pattr = &em->attr.part_attr;

	if(em->spawn && em->spawn(em, p, em->spawn_cls) == -1) {
		return -1;
	}

	psys_add_particle(em, p);
	return 0;
}

static void update_particle(struct psys_emitter *em, struct psys_particle *p, float tm, float dt, void *cls)
{
	vec3_t accel, grav;
	anm_time_t t;

	grav = psys_get_cur_value3(&em->attr.grav);

	accel.x = grav.x - p->vel.x * em->attr.drag;
	accel.y = grav.y - p->vel.y * em->attr.drag;
	accel.z = grav.z - p->vel.z * em->attr.drag;

	p->vel.x += accel.x * dt;
	p->vel.y += accel.y * dt;
	p->vel.z += accel.z * dt;

	p->pos.x += p->vel.x * dt;
	p->pos.y += p->vel.y * dt;
	p->pos.z += p->vel.z * dt;

	/* update particle attributes */
	t = (anm_time_t)(1000.0 * (p->max_life - p->life) / p->max_life);

	p->color = psys_get_value3(&p->pattr->color, t);
	p->alpha = psys_get_value(&p->pattr->alpha, t);
	p->size = p->base_size * psys_get_value(&p->pattr->size, t);

	p->life -= dt;
}

/* --- particle allocation pool --- */

static struct psys_particle *palloc(void)
{
	struct psys_particle *p;

	pthread_mutex_lock(&pool_lock);
	if(ppool) {
		p = ppool;
		ppool = ppool->next;
		ppool_size--;
	} else {
		p = malloc(sizeof *p);
	}
	pthread_mutex_unlock(&pool_lock);

	return p;
}

static void pfree(struct psys_particle *p)
{
	pthread_mutex_lock(&pool_lock);
	p->next = ppool;
	ppool = p;
	ppool_size++;
	pthread_mutex_unlock(&pool_lock);
}
