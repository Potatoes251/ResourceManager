#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(): m_textureID(0){}

bool Texture::setTexture(std::string const& textureFile)
{
	//generate and bind texture
	glGenTextures(1, &m_textureID);
	if (m_textureID == 0)
	{
		std::cerr << "Failed to generate texture ID.\n";
		return false;
	}

	glBindTexture(GL_TEXTURE_2D, m_textureID);

	//texture Wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//texture Filtering - Mipmaps
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	//flip the texture
	stbi_set_flip_vertically_on_load(true);

	//load file
	int width, height, nbChannels;
	std::string position = "../../assets/textures/" + textureFile;
	unsigned char* data = stbi_load(position.c_str(), &width, &height, &nbChannels, 0);

	//set color type
	if (data)
	{
		GLenum format;
		if (nbChannels == 1)
		{
			format = GL_RED;
		}
		else if (nbChannels == 3)
		{
			format = GL_RGB;
		}
		else if (nbChannels == 4)
		{
			format = GL_RGBA;
		}
		else
		{
			format = GL_RGB;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);

		return true;
	}

	else
	{
		std::cerr << "Failed to load texture: " << position << std::endl;
		return false;
	}
}

GLuint Texture::getID() const
{
	return m_textureID;
}
