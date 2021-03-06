#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#ifndef __APPLE__
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif

#include <imago2.h>
#include "psys.h"

void cleanup(void);
void disp(void);
void idle(void);
void reshape(int x, int y);
void keyb(unsigned char key, int x, int y);
void mouse(int bn, int state, int x, int y);
void motion(int x, int y);
void get_mouse_hit(float x, float y, float *pos);
unsigned int load_texture(const char *fname);

struct psys_emitter *ps;
unsigned int tex;

#define RATE	300.0

int main(int argc, char **argv)
{
	glutInitWindowSize(800, 600);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("libpsys example: simple");

	glutDisplayFunc(disp);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyb);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutIdleFunc(idle);

	glClearColor(0.05, 0.05, 0.05, 1);

	if(!(tex = load_texture("pimg.png"))) {
		fprintf(stderr, "failed to load the texture\n");
		return 1;
	}

	if(!(ps = psys_create())) {
		return 1;
	}
	if(psys_load_attr(&ps->attr, "simple.psys") == -1) {
		fprintf(stderr, "failed to load particle system definition\n");
		psys_free(ps);
		return 1;
	}
	/*ps->attr.tex = tex;
	ps->attr.drag = 2;
	psys_set_value3(&ps->attr.grav, 0, v3_cons(0, -4, 0));
	psys_set_anm_rnd(&ps->attr.life, 0, 2, 0);
	psys_set_value3(&ps->attr.spawn_range, 0, v3_cons(0.3, 0.3, 0.3));
	psys_set_anm_rnd3(&ps->attr.dir, 0, v3_cons(0, 0, 0), v3_cons(4, 4, 4));

	psys_set_value3(&ps->attr.part_attr.color, 0, v3_cons(1.0, 0.6, 0.4));
	psys_set_value3(&ps->attr.part_attr.color, 1000,  v3_cons(0.6, 0.3, 1.0));
	psys_set_value(&ps->attr.part_attr.alpha, 0, 1);
	psys_set_value(&ps->attr.part_attr.alpha, 700, 1);
	psys_set_value(&ps->attr.part_attr.alpha, 1000, 0);*/

	assert(glGetError() == GL_NO_ERROR);
	atexit(cleanup);

	glutMainLoop();
	return 0;
}

void cleanup(void)
{
	psys_free(ps);
}

void disp(void)
{
	unsigned int msec = glutGet(GLUT_ELAPSED_TIME);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -10);

	glClear(GL_COLOR_BUFFER_BIT);

	psys_update(ps, msec);
	psys_draw(ps);

	glutSwapBuffers();
	assert(glGetError() == GL_NO_ERROR);
}

void idle(void)
{
	glutPostRedisplay();
}

void reshape(int x, int y)
{
	glViewport(0, 0, x, y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)x / (float)y, 1.0, 1000.0);
}

void keyb(unsigned char key, int x, int y)
{
	switch(key) {
	case 27:
		exit(0);
	}
}

static int bnstate[32];
void mouse(int bn, int state, int x, int y)
{
	bnstate[bn - GLUT_LEFT_BUTTON] = state == GLUT_DOWN;
	if(bn == GLUT_LEFT_BUTTON) {
		float pos[3];
		get_mouse_hit(x, y, pos);

		psys_set_value(&ps->attr.rate, 0, state == GLUT_DOWN ? RATE : 0.0);
		psys_set_pos(ps, pos, 0);
	}
}

void motion(int x, int y)
{
	if(bnstate[0]) {
		float pos[3];
		get_mouse_hit(x, y, pos);
		psys_set_pos(ps, pos, 0);
	}
}

void get_mouse_hit(float x, float y, float *pos)
{
	double mv[16], proj[16];
	int vp[4];
	double t, nx, ny, nz, fx, fy, fz;

	glGetDoublev(GL_MODELVIEW_MATRIX, mv);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, vp);

	y = vp[3] - y;

	gluUnProject(x, y, 0, mv, proj, vp, &nx, &ny, &nz);
	gluUnProject(x, y, 1, mv, proj, vp, &fx, &fy, &fz);

	t = fabs(nz) / fabs(fz - nz);

	pos[0] = nx + (fx - nx) * t;
	pos[1] = ny + (fy - ny) * t;
	pos[2] = nz + (fz - nz) * t;
}

unsigned int load_texture(const char *fname)
{
	void *pixels;
	int xsz, ysz;
	unsigned int tex;

	if(!(pixels = img_load_pixels(fname, &xsz, &ysz, IMG_FMT_RGBA32))) {
		return 0;
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, xsz, ysz, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	img_free_pixels(pixels);

	return tex;
}
