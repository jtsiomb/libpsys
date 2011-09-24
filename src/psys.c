#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>
#include <vmath.h>
#include "psys_impl.h"

static int spawn(struct psys_emitter *em, struct psys_particle *p, void *cls);
static void update_particle(struct psys_emitter *em, struct psys_particle *p, float tm, float dt, void *cls);

static int init_v3track(struct v3track *v3t);
static void destroy_v3track(struct v3track *v3t);
static void set_v3value(struct v3track *v3t, anm_time_t tm, vec3_t v);
static vec3_t get_v3value(struct v3track *v3t, anm_time_t tm);

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
		psys_destroy(em);
		return -1;
	}
	if(anm_init_track(&em->rate) == -1) {
		psys_destroy(em);
		return -1;
	}
	if(anm_init_track(&em->life) == -1) {
		psys_destroy(em);
		return -1;
	}
	if(init_v3track(&em->dir) == -1) {
		psys_destroy(em);
		return -1;
	}
	if(init_v3track(&em->grav) == -1) {
		psys_destroy(em);
		return -1;
	}

	em->spawn = spawn;
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

	anm_destroy_node(&em->prs);
	anm_destroy_track(&em->rate);
	destroy_v3track(&em->dir);
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

void psys_set_rate(struct psys_emitter *em, float rate, float tm)
{
	anm_set_value(&em->rate, ANM_SEC2TM(tm), rate);
}

void psys_set_life(struct psys_emitter *em, float life, float tm)
{
	anm_set_value(&em->life, ANM_SEC2TM(tm), life);
}

void psys_set_dir(struct psys_emitter *em, vec3_t dir, float tm)
{
	set_v3value(&em->dir, ANM_SEC2TM(tm), dir);
}

void psys_set_grav(struct psys_emitter *em, vec3_t grav, float tm)
{
	set_v3value(&em->grav, ANM_SEC2TM(tm), grav);
}


void psys_clear_collision_planes(struct psys_emitter *em)
{
	struct col_plane *plane;

	plane = em->planes;
	while(plane) {
		struct col_plane *tmp = plane;
		plane = plane->next;
		free(tmp);
	}
}

int psys_add_collision_plane(struct psys_emitter *em, plane_t plane, float elast)
{
	struct col_plane *node;

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

/* --- query current state --- */
vec3_t psys_get_pos(struct psys_emitter *em)
{
	return em->cur_pos;
}

quat_t psys_get_rot(struct psys_emitter *em)
{
	return em->cur_rot;
}

float psys_get_rate(struct psys_emitter *em)
{
	return em->cur_rate;
}

float psys_get_life(struct psys_emitter *em)
{
	return em->cur_life;
}

vec3_t psys_get_dir(struct psys_emitter *em)
{
	return em->cur_dir;
}

vec3_t psys_get_grav(struct psys_emitter *em)
{
	return em->cur_grav;
}

/* --- update and render --- */

void psys_update(struct psys_emitter *em, float tm)
{
	float dt, spawn_dt;
	int i, spawn_count;
	struct psys_particle *p, pdummy;
	anm_time_t atm;

	assert(em->spawn && em->update);

	atm = ANM_SEC2TM(tm);

	em->cur_rate = anm_get_value(&em->rate, atm);
	dt = tm - em->last_update;

	/* how many particles to spawn for this interval ? */
	spawn_count = em->cur_rate * dt;

#ifndef SUB_UPDATE_POS
	em->cur_pos = anm_get_position(&em->prs, atm);
#endif
	em->cur_dir = get_v3value(&em->dir, atm);
	em->cur_life = anm_get_value(&em->life, atm);
	em->cur_grav = get_v3value(&em->grav, atm);

	spawn_dt = dt / (float)spawn_count;
	for(i=0; i<spawn_count; i++) {
#ifdef SUB_UPDATE_POS
		/* update emitter position for this spawning */
		em->cur_pos = anm_get_position(&em->prs, ANM_SEC2TM(em->last_update + spawn_dt));
#endif

		if(!(p = palloc())) {
			return;
		}
		if(em->spawn(em, p, em->spawn_cls) == -1) {
			pfree(p);
		}
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
		} else {
			p = p->next;
		}
	}
	em->plist = pdummy.next;
}

void psys_draw(struct psys_emitter *em)
{
	struct psys_particle *p;

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

static int spawn(struct psys_emitter *em, struct psys_particle *p, void *cls)
{
	p->pos = em->cur_pos;
	p->vel = em->cur_dir;
	p->size = 1.0;
	p->life = em->cur_life;

	psys_add_particle(em, p);
	return 0;
}

static void update_particle(struct psys_emitter *em, struct psys_particle *p, float tm, float dt, void *cls)
{
	vec3_t accel;

	accel.x = em->cur_grav.x - p->vel.x * em->drag;
	accel.y = em->cur_grav.y - p->vel.y * em->drag;
	accel.z = em->cur_grav.z - p->vel.z * em->drag;

	p->vel.x += accel.x * dt;
	p->vel.y += accel.y * dt;
	p->vel.z += accel.z * dt;

	p->pos.x += p->vel.x * dt;
	p->pos.y += p->vel.y * dt;
	p->pos.z += p->vel.z * dt;
}

/* --- v3track helper --- */

int init_v3track(struct v3track *v3t)
{
	if(anm_init_track(&v3t->x) == -1) {
		return -1;
	}
	if(anm_init_track(&v3t->y) == -1) {
		anm_destroy_track(&v3t->x);
		return -1;
	}
	if(anm_init_track(&v3t->z) == -1) {
		anm_destroy_track(&v3t->x);
		anm_destroy_track(&v3t->y);
		return -1;
	}
	return 0;
}

static void destroy_v3track(struct v3track *v3t)
{
	anm_destroy_track(&v3t->x);
	anm_destroy_track(&v3t->y);
	anm_destroy_track(&v3t->z);
}

static void set_v3value(struct v3track *v3t, anm_time_t tm, vec3_t v)
{
	anm_set_value(&v3t->x, tm, v.x);
	anm_set_value(&v3t->y, tm, v.y);
	anm_set_value(&v3t->z, tm, v.z);
}

static vec3_t get_v3value(struct v3track *v3t, anm_time_t tm)
{
	vec3_t v;
	v.x = anm_get_value(&v3t->x, tm);
	v.y = anm_get_value(&v3t->y, tm);
	v.z = anm_get_value(&v3t->z, tm);
	return v;
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

	if(p) {
		memset(p, 0, sizeof *p);
		/*reset_pattr(&p->attr);*/
	}
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
