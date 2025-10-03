#pragma once

#include "resourceManager.h"

#include "LibMath/Matrix.h"
#include "LibMath/Vector.h"
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Shader : public IResource
{
public :

	Shader();
	~Shader()
	{
		if (m_vertexShader != 0)
		{
			glDeleteShader(m_vertexShader);
			m_vertexShader = 0;
		}
		if (m_fragmentShader != 0)
		{
			glDeleteShader(m_fragmentShader);
			m_fragmentShader = 0;
		}
		if (m_shaderProgram != 0)
		{
			glDeleteProgram(m_shaderProgram);
			m_shaderProgram = 0;
		}
	}

	bool setVertexShader(std::string const& shader) const;		// load a vertex shader

	bool setFragmentShader(std::string const& shader) const;	// load a fragment shader

	bool link() const;									// link into a shader program

	void use() const;

	void setUniform(char const* name, LibMath::Matrix4 val) const;
	void setUniform(char const* name, LibMath::Matrix3x3 val) const;
	void setUniform(char const* name, LibMath::Vector4* val);
	void setUniform(char const* name, LibMath::Vector3 val);
	void setUniform(char const* name, LibMath::Vector2* val);
	void setUniform(char const* name, int* val);
	void setUniform(char const* name, float* val);
	void setUniform(char const* name, size_t* val);

	void setUniformTex(char const* name, int* val);

private :

	GLuint m_shaderProgram;

	GLuint m_vertexShader;
	GLuint m_fragmentShader;
};