#include "shader.h"


#include <glad/glad.h>
#include <GLFW/glfw3.h>


Shader::Shader()
{
	m_shaderProgram = glCreateProgram();

	m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
}

bool Shader::setVertexShader(std::string const& shaderFile) const
{
	std::ifstream file("../../ModernOpenGL/shaders/" + shaderFile);	// open the file
	if (!file.is_open())
	{
		std::cerr << "Failed to open file" << std::endl;	// handle file not found
		return false;
	}

	std::stringstream shaderStream;
	shaderStream << file.rdbuf();

	std::string shaderString = shaderStream.str();

	char const* shaderSrc = shaderString.c_str();

	glShaderSource(m_vertexShader, 1, &shaderSrc, NULL);

	glCompileShader(m_vertexShader);

	GLint result;
	GLchar infolog[1024];
	glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(m_vertexShader, sizeof(infolog), NULL, infolog);
		std::cout << "Error! Vertex shader failed to compile." << infolog << std::endl;
		return false;
	}

	return true;
}

bool Shader::setFragmentShader(std::string const& shaderFile) const
{
	std::ifstream file("../../ModernOpenGL/shaders/" + shaderFile);	// open the file
	if (!file.is_open())
	{
		std::cerr << "Failed to open file" << std::endl;	// handle file not found
		return false;
	}

	std::stringstream shaderStream;
	shaderStream << file.rdbuf();

	std::string shaderString = shaderStream.str();

	char const* shaderSrc = shaderString.c_str();

	glShaderSource(m_fragmentShader, 1, &shaderSrc, NULL);

	glCompileShader(m_fragmentShader);

	GLint result;
	GLchar infolog[1024];
	glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(m_fragmentShader, sizeof(infolog), NULL, infolog);
		std::cout << "Error! Fragment shader failed to compile." << infolog << std::endl;
		return false;
	}

	return true;
}

bool Shader::link() const
{
	glAttachShader(m_shaderProgram, m_vertexShader);
	glAttachShader(m_shaderProgram, m_fragmentShader);

	glLinkProgram(m_shaderProgram);

	GLint result;
	GLchar infolog[1024];

	glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(m_shaderProgram, sizeof(infolog), NULL, infolog);
		std::cout << "Error! shader failed to link." << infolog << std::endl;
		return false;
	}

	glDeleteShader(m_vertexShader);
	glDeleteShader(m_fragmentShader);


	return true;
}

void Shader::use() const
{
	glUseProgram(m_shaderProgram);
}

void Shader::setUniform(char const* name, LibMath::Matrix4 val) const
{
	GLint location = glGetUniformLocation(m_shaderProgram, name);
	glUniformMatrix4fv(location, 1, GL_FALSE, val.data());
}

void Shader::setUniform(char const* name, LibMath::Matrix3x3 val) const
{
	GLint location = glGetUniformLocation(m_shaderProgram, name);
	glUniformMatrix4fv(location, 1, GL_FALSE, val.data());
}

void Shader::setUniform(char const* name, LibMath::Vector4* val)
{
	GLint location = glGetUniformLocation(m_shaderProgram, name);
	glUniform4fv(location, 1, (GLfloat*)val);
}

void Shader::setUniform(char const* name, LibMath::Vector3 val)
{
	GLint location = glGetUniformLocation(m_shaderProgram, name);
	glUniform3fv(location, 1, &val[0]);
}

void Shader::setUniform(char const* name, LibMath::Vector2* val)
{
	GLint location = glGetUniformLocation(m_shaderProgram, name);
	glUniform2fv(location, 1, (GLfloat*)val);
}

void Shader::setUniform(char const* name, int* val)
{
	GLint location = glGetUniformLocation(m_shaderProgram, name);
	glUniform1i(location, *val);
}

void Shader::setUniform(char const* name, float* val)
{
	GLint location = glGetUniformLocation(m_shaderProgram, name);
	glUniform1f(location, *val);
}

void Shader::setUniform(char const* name, size_t* val)
{
	GLint location = glGetUniformLocation(m_shaderProgram, name);
	glUniform1f(location, *val);
}

void Shader::setUniformTex(char const* name, int* val)
{
	GLint location = glGetUniformLocation(m_shaderProgram, name);
	if (location == -1)
	{
		std::cerr << "Uniform '" << name << "' not found or unused in shader program.\n";
		return;
	}
	glUniform1i(location, *val);
}
