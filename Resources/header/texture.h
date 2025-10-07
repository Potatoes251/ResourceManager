#pragma once

#include "resourceManager.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Texture : public IResource
{
public:
	//constructor and destructor
	Texture(std::string const& textureFile);

	~Texture();

	//functions
	bool	setTexture(std::string const& textureFile);

	GLuint	getID() const;

private:
	GLuint	m_textureID = 0;
};

