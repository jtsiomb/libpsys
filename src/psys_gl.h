#ifndef PSYS_GL_H_
#define PSYS_GL_H_

struct psys_particle;
struct psys_emitter;

void psys_gl_draw_start(struct psys_emitter *em, void *cls);
void psys_gl_draw(struct psys_emitter *em, struct psys_particle *p, void *cls);
void psys_gl_draw_end(struct psys_emitter *em, void *cls);

unsigned int psys_gl_load_texture(const char *fname, void *cls);
void psys_gl_unload_texture(unsigned int tex, void *cls);

#endif	/* PSYS_GL_H_ */
