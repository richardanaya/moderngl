#include "ModernGL.hpp"

#include "OpenGL.hpp"

const char * categoryNames[] = {
	"VertexShader",
	"FragmentShader",
	"GeometryShader",
	"TessEvaluationShader",
	"TessControlShader",
};

PyObject * NewProgram(PyObject * self, PyObject * args) {
	PyObject * shaders;

	if (!PyArg_ParseTuple(args, "O:NewProgram", &shaders)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("shaders", shaders, PyList_Type);

	int count = (int)PyList_Size(shaders);

	int category[NUM_SHADER_CATEGORIES] = {};
	for (int i = 0; i < count; ++i) {
		Shader * shader = (Shader *)PyList_GetItem(shaders, i);
		CHECK_AND_REPORT_ELEMENT_TYPE_ERROR("shaders", shader, ShaderType, i);
		++category[shader->category];
	}

	for (int i = 0; i < NUM_SHADER_CATEGORIES; ++i) {
		if (category[i] > 1) {
			PyErr_Format(ModuleError, "NewProgram() duplicate %s", categoryNames[i]);
			return 0;
		}
	}
	
	int program = OpenGL::glCreateProgram();

	for (int i = 0; i < count; ++i) {
		Shader * shader = (Shader *)PyList_GetItem(shaders, i);
		OpenGL::glAttachShader(program, shader->shader);
	}

	int linked = OpenGL::GL_FALSE;
	OpenGL::glLinkProgram(program);
	OpenGL::glGetProgramiv(program, OpenGL::GL_LINK_STATUS, &linked);

	if (!linked) {
		int logSize = 0;
		static const char * logTitle = "NewProgram() linking failed\n";
		static int logTitleSize = strlen(logTitle);
		memcpy(compilerLog, logTitle, logTitleSize);
		OpenGL::glGetProgramInfoLog(program, maxCompilerLog - logTitleSize, &logSize, compilerLog + logTitleSize);
		compilerLog[logTitleSize + logSize] = 0;
		OpenGL::glDeleteProgram(program);
		program = 0;
	} else {
		compilerLog[0] = 0;
		OpenGL::glUseProgram(program);
		activeProgram = program;
	}

	if (!program) {
		PyErr_SetString(ModuleError, compilerLog);
		return 0;
	}
	
	return CreateProgramType(program);
}

PyObject * DeleteProgram(PyObject * self, PyObject * args) {
	Program * program;

	if (!PyArg_ParseTuple(args, "O:DeleteProgram", &program)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("program", program, ProgramType);

	int shaders[8] = {};
	int numShaders = 0;

	OpenGL::glGetAttachedShaders(program->program, 8, &numShaders, (OpenGL::GLuint *)shaders);
	for (int i = 0; i < numShaders; ++i) {
		OpenGL::glDeleteShader(shaders[i]);
	}
	OpenGL::glDeleteProgram(program->program);
	Py_RETURN_NONE;
}

PyObject * UseProgram(PyObject * self, PyObject * args) {
	Program * program;

	if (!PyArg_ParseTuple(args, "O:UseProgram", &program)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("program", program, ProgramType);

	OpenGL::glUseProgram(program->program);
	activeProgram = program->program;
	Py_RETURN_NONE;
}

PyObject * GetProgramInterface(PyObject * self, PyObject * args) {
	Program * program;

	if (!PyArg_ParseTuple(args, "O:GetProgramInterface", &program)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("program", program, ProgramType);
	
	PyObject * dict = PyDict_New();

	int attributes = 0;
	OpenGL::glGetProgramiv(program->program, OpenGL::GL_ACTIVE_ATTRIBUTES, &attributes);
	for (int i = 0; i < attributes; ++i) {
		char name[64 + 1];
		int size;
		int length;
		unsigned type;
		OpenGL::glGetActiveAttrib(program->program, i, 64, &length, &size, &type, name);
		int location = OpenGL::glGetAttribLocation(program->program, name);
		name[length] = 0;
		if (location >= 0) {
			PyDict_SetItemString(dict, name, CreateAttributeLocationType(location, program->program));
		}
	}

	int uniforms = 0;
	OpenGL::glGetProgramiv(program->program, OpenGL::GL_ACTIVE_UNIFORMS, &uniforms);
	for (int i = 0; i < uniforms; ++i) {
		char name[64 + 1];
		int size;
		int length;
		unsigned type;
		OpenGL::glGetActiveUniform(program->program, i, 64, &length, &size, &type, name);
		int location = OpenGL::glGetUniformLocation(program->program, name);
		name[length] = 0;
		if (location >= 0) {
			PyDict_SetItemString(dict, name, CreateUniformLocationType(location, program->program));
		}
	}

	int uniformBlocks = 0;
	OpenGL::glGetProgramiv(program->program, OpenGL::GL_ACTIVE_UNIFORM_BLOCKS, &uniformBlocks);
	for (int i = 0; i < uniformBlocks; ++i) {
		char name[64 + 1];
		int length;
		int location = -1;
		OpenGL::glGetActiveUniformBlockName(program->program, i, 64, &length, name);
		OpenGL::glGetActiveUniformBlockiv(program->program, i, OpenGL::GL_UNIFORM_BLOCK_BINDING, &location);
		name[length] = 0;
		if (location >= 0) {
			PyDict_SetItemString(dict, name, CreateUniformBufferLocationType(location, program->program));
		}
	}

	return dict;
}

PyObject * GetAttributeLocation(PyObject * self, PyObject * args, PyObject * kwargs) {
	Program * program;
	const char * name;
	bool strict = false;

	static const char * kwlist[] = {"program", "name", "strict", 0};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Os|p:GetAttributeLocation", (char **)kwlist, &program, &name, &strict)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("program", program, ProgramType);

	int location = OpenGL::glGetAttribLocation(program->program, name);
	if (strict && location < 0) {
		PyErr_Format(ModuleError, "GetAttributeLocation() `%s` not found", name);
		return 0;
	}

	return CreateAttributeLocationType(location, program->program);
}

PyObject * GetUniformLocation(PyObject * self, PyObject * args, PyObject * kwargs) {
	Program * program;
	const char * name;
	bool strict = false;

	static const char * kwlist[] = {"program", "name", "strict", 0};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Os|p:GetUniformLocation", (char **)kwlist, &program, &name, &strict)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("program", program, ProgramType);

	int location = OpenGL::glGetUniformLocation(program->program, name);
	if (strict && location < 0) {
		PyErr_Format(ModuleError, "GetUniformLocation() `%s` not found", name);
		return 0;
	}

	return CreateUniformLocationType(location, program->program);
}

PyObject * GetUniformBufferLocation(PyObject * self, PyObject * args, PyObject * kwargs) {
	Program * program;
	const char * name;
	bool strict = false;

	static const char * kwlist[] = {"program", "name", "strict", 0};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Os|p:GetUniformBufferLocation", (char **)kwlist, &program, &name, &strict)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("program", program, ProgramType);

	int location = OpenGL::glGetUniformBlockIndex(program->program, name);
	if (strict && location < 0) {
		PyErr_Format(ModuleError, "GetUniformBufferLocation() `%s` not found", name);
		return 0;
	}

	return CreateUniformBufferLocationType(location, program->program);
}

PyObject * Uniform1f(PyObject * self, PyObject * args) {
	UniformLocation * location;
	float v0;

	if (!PyArg_ParseTuple(args, "Of:Uniform1f", &location, &v0)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("location", location, UniformLocationType);

	if (location->program != activeProgram) {
		PyErr_SetString(ModuleError, "Uniform1f() the selected uniform does not belong to the active program");
		return 0;
	}

	OpenGL::glUniform1f(location->location, v0);
	Py_RETURN_NONE;
}

PyObject * Uniform2f(PyObject * self, PyObject * args) {
	UniformLocation * location;
	float v0;
	float v1;

	if (!PyArg_ParseTuple(args, "Off:Uniform2f", &location, &v0, &v1)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("location", location, UniformLocationType);

	if (location->program != activeProgram) {
		PyErr_SetString(ModuleError, "Uniform2f() the selected uniform does not belong to the active program");
		return 0;
	}

	OpenGL::glUniform2f(location->location, v0, v1);
	Py_RETURN_NONE;
}

PyObject * Uniform3f(PyObject * self, PyObject * args) {
	UniformLocation * location;
	float v0;
	float v1;
	float v2;

	if (!PyArg_ParseTuple(args, "Offf:Uniform3f", &location, &v0, &v1, &v2)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("location", location, UniformLocationType);

	if (location->program != activeProgram) {
		PyErr_SetString(ModuleError, "Uniform3f() the selected uniform does not belong to the active program");
		return 0;
	}

	OpenGL::glUniform3f(location->location, v0, v1, v2);
	Py_RETURN_NONE;
}

PyObject * Uniform4f(PyObject * self, PyObject * args) {
	UniformLocation * location;
	float v0;
	float v1;
	float v2;
	float v3;

	if (!PyArg_ParseTuple(args, "Offff:Uniform4f", &location, &v0, &v1, &v2, &v3)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("location", location, UniformLocationType);

	if (location->program != activeProgram) {
		PyErr_SetString(ModuleError, "Uniform4f() the selected uniform does not belong to the active program");
		return 0;
	}

	OpenGL::glUniform4f(location->location, v0, v1, v2, v3);
	Py_RETURN_NONE;
}

PyObject * Uniform1i(PyObject * self, PyObject * args) {
	UniformLocation * location;
	int v0;

	if (!PyArg_ParseTuple(args, "Oi:Uniform1i", &location, &v0)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("location", location, UniformLocationType);

	if (location->program != activeProgram) {
		PyErr_SetString(ModuleError, "Uniform1i() the selected uniform does not belong to the active program");
		return 0;
	}

	OpenGL::glUniform1i(location->location, v0);
	Py_RETURN_NONE;
}

PyObject * Uniform2i(PyObject * self, PyObject * args) {
	UniformLocation * location;
	int v0;
	int v1;

	if (!PyArg_ParseTuple(args, "Oii:Uniform2i", &location, &v0, &v1)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("location", location, UniformLocationType);

	if (location->program != activeProgram) {
		PyErr_SetString(ModuleError, "Uniform2i() the selected uniform does not belong to the active program");
		return 0;
	}

	OpenGL::glUniform2i(location->location, v0, v1);
	Py_RETURN_NONE;
}

PyObject * Uniform3i(PyObject * self, PyObject * args) {
	UniformLocation * location;
	int v0;
	int v1;
	int v2;

	if (!PyArg_ParseTuple(args, "Oiii:Uniform3i", &location, &v0, &v1, &v2)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("location", location, UniformLocationType);

	if (location->program != activeProgram) {
		PyErr_SetString(ModuleError, "Uniform3i() the selected uniform does not belong to the active program");
		return 0;
	}

	OpenGL::glUniform3i(location->location, v0, v1, v2);
	Py_RETURN_NONE;
}

PyObject * Uniform4i(PyObject * self, PyObject * args) {
	UniformLocation * location;
	int v0;
	int v1;
	int v2;
	int v3;

	if (!PyArg_ParseTuple(args, "Oiiii:Uniform4i", &location, &v0, &v1, &v2, &v3)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("location", location, UniformLocationType);

	if (location->program != activeProgram) {
		PyErr_SetString(ModuleError, "Uniform4i() the selected uniform does not belong to the active program");
		return 0;
	}

	OpenGL::glUniform4i(location->location, v0, v1, v2, v3);
	Py_RETURN_NONE;
}

PyObject * UniformMatrix(PyObject * self, PyObject * args) {
	UniformLocation * location;
	PyObject * matrix;

	if (!PyArg_ParseTuple(args, "OO:UniformMatrix", &location, &matrix)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("location", location, UniformLocationType);
	CHECK_AND_REPORT_ARG_TYPE_ERROR("matrix", matrix, PyList_Type);

	if (location->program != activeProgram) {
		PyErr_SetString(ModuleError, "UniformMatrix() the selected uniform does not belong to the active program");
		return 0;
	}

	float matrix_data[16];

	int count = (int)PyList_Size(matrix);
	
	if (count != 16) {
		PyErr_Format(ModuleError, "UniformMatrix() matrix length must be 16, not %d", count);
		return 0;
	}

	for (int i = 0; i < count; ++i) {
		PyObject * item = PyList_GET_ITEM(matrix, i);
		CHECK_AND_REPORT_ELEMENT_TYPE_ERROR("matrix", item, PyFloat_Type, i);
		matrix_data[i] = (float)PyFloat_AsDouble(item);
	}

	OpenGL::glUniformMatrix4fv(location->location, 1, false, matrix_data);
	Py_RETURN_NONE;
}

PyObject * UniformTransposeMatrix(PyObject * self, PyObject * args) {
	UniformLocation * location;
	PyObject * matrix;

	if (!PyArg_ParseTuple(args, "OO:UniformTransposeMatrix", &location, &matrix)) {
		return 0;
	}

	CHECK_AND_REPORT_ARG_TYPE_ERROR("location", location, UniformLocationType);
	CHECK_AND_REPORT_ARG_TYPE_ERROR("matrix", matrix, PyList_Type);

	if (location->program != activeProgram) {
		PyErr_SetString(ModuleError, "UniformTransposeMatrix() the selected uniform does not belong to the active program");
		return 0;
	}

	float matrix_data[16];

	int count = (int)PyList_Size(matrix);
	
	if (count != 16) {
		PyErr_Format(ModuleError, "UniformTransposeMatrix() matrix length must be 16, not %d", count);
		return 0;
	}

	for (int i = 0; i < count; ++i) {
		PyObject * item = PyList_GET_ITEM(matrix, i);
		CHECK_AND_REPORT_ELEMENT_TYPE_ERROR("matrix", item, PyFloat_Type, i);
		matrix_data[i] = (float)PyFloat_AsDouble(item);
	}

	OpenGL::glUniformMatrix4fv(location->location, 1, true, matrix_data);
	Py_RETURN_NONE;
}