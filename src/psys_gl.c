#ifndef __APPLE__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif

#include "psys_impl.h"

void psys_gl_draw_start(struct psys_emitter *em, void *cls)
{
	float xform[16];

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glGetFloatv(GL_MODELVIEW_MATRIX, xform);
	xform[0] = xform[5] = xform[10] = 1.0;
	xform[1] = xform[2] = xform[4] = xform[6] = xform[8] = xform[9] = 0.0;

	glLoadMatrixf(xform);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(em->tex) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, em->tex);
	}

	glDepthMask(0);

	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
}

void psys_gl_draw(struct psys_emitter *em, struct psys_particle *p, void *cls)
{
	float hsz = p->size / 2.0;

	glTexCoord2f(0, 0);
	glVertex3f(p->pos.x - hsz, p->pos.y - hsz, p->pos.z);

	glTexCoord2f(1, 0);
	glVertex3f(p->pos.x + hsz, p->pos.y - hsz, p->pos.z);

	glTexCoord2f(1, 1);
	glVertex3f(p->pos.x + hsz, p->pos.y + hsz, p->pos.z);

	glTexCoord2f(0, 1);
	glVertex3f(p->pos.x - hsz, p->pos.y + hsz, p->pos.z);
}

void psys_gl_draw_end(struct psys_emitter *em, void *cls)
{
	glEnd();

	glDepthMask(1);
	glPopAttrib();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
