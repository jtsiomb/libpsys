#include <stdio.h>
#include <stdlib.h>

#ifndef __APPLE__
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif

#include <vmath.h>
#include "psys.h"

void disp(void);
void idle(void);
void reshape(int x, int y);
void keyb(unsigned char key, int x, int y);
void mouse(int bn, int state, int x, int y);
void motion(int x, int y);
vec3_t get_mouse_hit(float x, float y);

struct psys_emitter *ps;

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

	glEnable(GL_CULL_FACE);

	if(!(ps = psys_create())) {
		return 1;
	}
	psys_set_grav(ps, v3_cons(0, -1, 0), 0);
	psys_set_life(ps, 2, 0);

	glutMainLoop();
	return 0;
}

void disp(void)
{
	static unsigned int prev_msec;
	unsigned int msec = glutGet(GLUT_ELAPSED_TIME);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -10);

	glClear(GL_COLOR_BUFFER_BIT);

	psys_update(ps, (msec - prev_msec) / 1000.0);
	psys_draw(ps);

	glutSwapBuffers();
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
		psys_set_rate(ps, state == GLUT_DOWN ? 1.0 : 0.0, 0);
		psys_set_pos(ps, get_mouse_hit(x, y), 0);
	}
}

void motion(int x, int y)
{
	if(bnstate[0]) {
		psys_set_pos(ps, get_mouse_hit(x, y), 0);
	}
}

vec3_t get_mouse_hit(float x, float y)
{
	double mv[16], proj[16];
	int vp[4];
	double res_x, res_y, res_z;
	float t;
	vec3_t res, pnear, pfar;

	glGetDoublev(GL_MODELVIEW_MATRIX, mv);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, vp);

	y = vp[3] - y;

	gluUnProject(x, y, 0, mv, proj, vp, &res_x, &res_y, &res_z);
	pnear.x = res_x;
	pnear.y = res_y;
	pnear.z = res_z;

	gluUnProject(x, y, 1, mv, proj, vp, &res_x, &res_y, &res_z);
	pfar.x = res_x;
	pfar.y = res_y;
	pfar.z = res_z;

	t = fabs(pnear.z) / fabs(pfar.z - pnear.z);
	res = v3_add(pnear, v3_scale(v3_sub(pfar, pnear), t));

	return res;
}
