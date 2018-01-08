#pragma once

#include "Python.hpp"
#include "GLContext.hpp"
#include "GLMethods.hpp"
#include "Error.hpp"

typedef void (* MGLProc)();

enum MGLEnableFlag {
	MGL_NOTHING = 0,
	MGL_BLEND = 1,
	MGL_DEPTH_TEST = 2,
	MGL_CULL_FACE = 4,
	MGL_ENABLE_MASK = 7,
};

enum SHADER_SLOT_ENUM {
	VERTEX_SHADER_SLOT,
	FRAGMENT_SHADER_SLOT,
	GEOMETRY_SHADER_SLOT,
	TESS_EVALUATION_SHADER_SLOT,
	TESS_CONTROL_SHADER_SLOT,
	NUM_SHADER_SLOTS,
};

static const int SHADER_TYPE[] = {
	GL_VERTEX_SHADER,
	GL_FRAGMENT_SHADER,
	GL_GEOMETRY_SHADER,
	GL_TESS_EVALUATION_SHADER,
	GL_TESS_CONTROL_SHADER,
};

static const char * SHADER_NAME[] = {
	"vertex_shader",
	"fragment_shader",
	"geometry_shader",
	"tess_evaluation_shader",
	"tess_control_shader",
};

static const char * SHADER_NAME_UNDERLINE[] = {
	"=============",
	"===============",
	"===============",
	"======================",
	"===================",
};

struct MGLAttribute;
struct MGLBuffer;
struct MGLComputeShader;
struct MGLContext;
struct MGLFramebuffer;
struct MGLInvalidObject;
struct MGLPrimitive;
struct MGLProgram;
struct MGLRenderbuffer;
struct MGLShader;
struct MGLTexture;
struct MGLTexture3D;
struct MGLTextureCube;
struct MGLTextureFilter;
struct MGLUniform;
struct MGLUniformBlock;
struct MGLVertexArray;

struct MGLAttribute {
	PyObject_HEAD

	void * gl_attrib_ptr_proc;

	int program_obj;

	int number;
	int location;
	int type;

	int scalar_type;

	int dimension;
	int array_length;

	int rows_length;
	int row_length;
	int row_size;

	char shape;
	bool normalizable;
};

struct MGLBuffer {
	PyObject_HEAD

	MGLContext * context;

	int buffer_obj;

	Py_ssize_t size;
	bool dynamic;
};

struct MGLComputeShader {
	PyObject_HEAD

	MGLContext * context;

	PyObject * source;

	PyObject * uniforms;
	PyObject * uniform_blocks;
	PyObject * subroutines;
	PyObject * subroutine_uniforms;

	int program_obj;
	int shader_obj;
};

struct MGLContext {
	PyObject_HEAD

	MGLFramebuffer * screen;
	MGLFramebuffer * bound_framebuffer;

	GLContext gl_context;

	int version_code;

	int max_samples;
	int max_integer_samples;
	int max_color_attachments;

	int max_texture_units;
	int default_texture_unit;

	int front_face;
	bool wireframe;

	GLMethods gl;
};

struct MGLFramebuffer {
	PyObject_HEAD

	MGLContext * context;

	bool * color_mask;

	unsigned * draw_buffers;
	int draw_buffers_len;

	int framebuffer_obj;

	int viewport_x;
	int viewport_y;
	int viewport_width;
	int viewport_height;

	int width;
	int height;
	int samples;

	bool depth_mask;
};

struct MGLInvalidObject {
	PyObject_HEAD
};

struct MGLPrimitive {
	PyObject_HEAD
	PyObject * wrapper;

	int primitive;
	int geometry_primitive;
	int transform_primitive;
};

struct MGLProgram {
	PyObject_HEAD

	MGLContext * context;

	MGLPrimitive * geometry_input;
	MGLPrimitive * geometry_output;

	int program_obj;

	int num_vertex_shader_subroutines;
	int num_fragment_shader_subroutines;
	int num_geometry_shader_subroutines;
	int num_tess_evaluation_shader_subroutines;
	int num_tess_control_shader_subroutines;

	int geometry_vertices;
	int num_varyings;
};

enum MGLQueryKeys {
	SAMPLES_PASSED,
	ANY_SAMPLES_PASSED,
	TIME_ELAPSED,
	PRIMITIVES_GENERATED,
};

struct MGLQuery {
	PyObject_HEAD

	MGLContext * context;

	int query_obj[4];
};

struct MGLRenderbuffer {
	PyObject_HEAD

	MGLContext * context;

	union {
		int renderbuffer_obj;
		int texture_obj;
	};

	int width;
	int height;
	int components;

	int samples;

	bool floats;
	bool depth;
};

struct MGLShader {
	PyObject_HEAD

	MGLContext * context;

	PyObject * source;

	int shader_obj;
	int shader_type;
	int shader_slot;
};

struct MGLTexture {
	PyObject_HEAD

	MGLContext * context;

	union {
		int renderbuffer_obj;
		int texture_obj;
	};

	int width;
	int height;
	int components;

	int samples;

	bool floats;
	bool depth;

	MGLTextureFilter * filter;
	bool repeat_x;
	bool repeat_y;
};

struct MGLTexture3D {
	PyObject_HEAD

	MGLContext * context;

	int texture_obj;

	int width;
	int height;
	int depth;

	int components;

	int samples;

	bool floats;

	MGLTextureFilter * filter;

	bool repeat_x;
	bool repeat_y;
	bool repeat_z;
};

struct MGLTextureCube {
	PyObject_HEAD

	MGLContext * context;

	int texture_obj;

	int width;
	int height;
	int depth;

	int components;

	bool floats;
};

struct MGLTextureFilter {
	PyObject_HEAD

	PyObject * wrapper;

	int min_filter;
	int mag_filter;
};

struct MGLUniform {
	PyObject_HEAD

	MGLProc value_getter;
	MGLProc value_setter;
	MGLProc gl_value_reader_proc;
	MGLProc gl_value_writer_proc;

	int program_obj;

	int number;
	int location;
	int type;

	int dimension;
	int element_size;
	int array_length;

	bool matrix;
};

struct MGLUniformBlock {
	PyObject_HEAD

	const GLMethods * gl;

	int program_obj;

	int index;
	int size;
};

struct MGLVertexArray {
	PyObject_HEAD

	MGLContext * context;

	MGLProgram * program;
	MGLBuffer * index_buffer;

	unsigned * subroutines;
	int num_subroutines;

	int vertex_array_obj;
	int num_vertices;
};

void MGLAttribute_Invalidate(MGLAttribute * attribute);
void MGLBuffer_Invalidate(MGLBuffer * buffer);
void MGLComputeShader_Invalidate(MGLComputeShader * program);
void MGLContext_Invalidate(MGLContext * context);
void MGLFramebuffer_Invalidate(MGLFramebuffer * framebuffer);
void MGLProgram_Invalidate(MGLProgram * program);
void MGLRenderbuffer_Invalidate(MGLRenderbuffer * renderbuffer);
void MGLShader_Invalidate(MGLShader * shader);
void MGLTexture3D_Invalidate(MGLTexture3D * texture);
void MGLTextureCube_Invalidate(MGLTextureCube * texture);
void MGLTexture_Invalidate(MGLTexture * texture);
void MGLUniform_Invalidate(MGLUniform * uniform);
void MGLVertexArray_Invalidate(MGLVertexArray * vertex_array);

void MGLAttribute_Complete(MGLAttribute * attribute, const GLMethods & gl);
void MGLUniform_Complete(MGLUniform * self, const GLMethods & gl);
void MGLUniformBlock_Complete(MGLUniformBlock * uniform_block, const GLMethods & gl);
void MGLVertexArray_Complete(MGLVertexArray * vertex_array);

void MGLComputeShader_Compile(MGLComputeShader * compute_shader);
void MGLProgram_Compile(MGLProgram * program, PyObject * outputs);
void MGLShader_Compile(MGLShader * shader);
void MGLContext_Initialize(MGLContext * self);

extern PyTypeObject MGLAttribute_Type;
extern PyTypeObject MGLBuffer_Type;
extern PyTypeObject MGLComputeShader_Type;
extern PyTypeObject MGLContext_Type;
extern PyTypeObject MGLFramebuffer_Type;
extern PyTypeObject MGLInvalidObject_Type;
extern PyTypeObject MGLPrimitive_Type;
extern PyTypeObject MGLProgram_Type;
extern PyTypeObject MGLQuery_Type;
extern PyTypeObject MGLRenderbuffer_Type;
extern PyTypeObject MGLShader_Type;
extern PyTypeObject MGLTexture3D_Type;
extern PyTypeObject MGLTextureCube_Type;
extern PyTypeObject MGLTextureFilter_Type;
extern PyTypeObject MGLTexture_Type;
extern PyTypeObject MGLUniformBlock_Type;
extern PyTypeObject MGLUniform_Type;
extern PyTypeObject MGLVertexArray_Type;

extern MGLPrimitive * MGL_TRIANGLES;
extern MGLPrimitive * MGL_TRIANGLE_STRIP;
extern MGLPrimitive * MGL_TRIANGLE_FAN;
extern MGLPrimitive * MGL_LINES;
extern MGLPrimitive * MGL_LINE_STRIP;
extern MGLPrimitive * MGL_LINE_LOOP;
extern MGLPrimitive * MGL_POINTS;
extern MGLPrimitive * MGL_LINE_STRIP_ADJACENCY;
extern MGLPrimitive * MGL_LINES_ADJACENCY;
extern MGLPrimitive * MGL_TRIANGLE_STRIP_ADJACENCY;
extern MGLPrimitive * MGL_TRIANGLES_ADJACENCY;
extern MGLPrimitive * MGL_NO_PRIMITIVE;

extern MGLTextureFilter * MGL_LINEAR;
extern MGLTextureFilter * MGL_NEAREST;
extern MGLTextureFilter * MGL_LINEAR_MIPMAP;
extern MGLTextureFilter * MGL_NEAREST_MIPMAP;

extern PyObject * MGLError_TypePtr;