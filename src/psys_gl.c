#include <string.h>
#include <errno.h>
#include <assert.h>

#ifndef __APPLE__
#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif

#include <imago2.h>
#include "psys.h"
#include "psys_gl.h"

void psys_gl_draw_start(struct psys_emitter *em, void *cls)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	if(em->attr.tex) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, em->attr.tex);
	}

	glDepthMask(0);
}

void psys_gl_draw(struct psys_emitter *em, struct psys_particle *p, void *cls)
{
	float hsz = p->size / 2.0;

	float xform[16];

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef(p->pos.x, p->pos.y, p->pos.z);

	glGetFloatv(GL_MODELVIEW_MATRIX, xform);
	xform[0] = xform[5] = xform[10] = 1.0;
	xform[1] = xform[2] = xform[4] = xform[6] = xform[8] = xform[9] = 0.0;
	glLoadMatrixf(xform);

	glBegin(GL_QUADS);
	glColor4f(p->color.x, p->color.y, p->color.z, p->alpha);

	glTexCoord2f(0, 0);
	glVertex2f(-hsz, -hsz);

	glTexCoord2f(1, 0);
	glVertex2f(hsz, -hsz);

	glTexCoord2f(1, 1);
	glVertex2f(hsz, hsz);

	glTexCoord2f(0, 1);
	glVertex2f(-hsz, hsz);
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void psys_gl_draw_end(struct psys_emitter *em, void *cls)
{
	glDepthMask(1);
	glPopAttrib();
}


unsigned int psys_gl_load_texture(const char *fname, void *cls)
{
	unsigned int tex;
	void *pixels;
	int xsz, ysz;

	if(!(pixels = img_load_pixels(fname, &xsz, &ysz, IMG_FMT_RGBA32))) {
		return 0;
	}
	printf("%s: creating texture %s (%dx%d)\n", __func__, fname, xsz, ysz);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, xsz, ysz, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	assert(glGetError() == GL_NO_ERROR);

	img_free_pixels(pixels);
	return tex;
}

void psys_gl_unload_texture(unsigned int tex, void *cls)
{
	glDeleteTextures(1, &tex);
}
