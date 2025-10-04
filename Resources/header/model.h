#pragma once

#include "resourceManager.h"
#include "LibMath/Vector.h"
#include "threadPool.h"

#include <future>
#include <thread>
#include <mutex>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// add timer for the parser to see how long it takes
#if 1
#include <chrono>
#define TEST_TIME
#endif
#if 1
#define MULTITHREADING
#endif

struct Vertex
{
	Vertex(LibMath::Vector3 pos, LibMath::Vector3 normal, LibMath::Vector2 texCoord);

	LibMath::Vector3	m_position;
	LibMath::Vector3	m_normal;
	LibMath::Vector2	m_textureUV;

	~Vertex() = default;
};

// define boundaries for a thread to read from
struct Chunk
{
	size_t	start;
	size_t	end;
};

// used for the return in thread
struct ParseResult
{
	std::vector<LibMath::Vector3>	positions;
	std::vector<LibMath::Vector3>	normals;
	std::vector<LibMath::Vector2>	texCoords;
};

struct Something
{
	std::vector<Vertex> m_mesh;
	std::vector<uint32_t> m_indices;
};

class Model : public IResource
{
public :

	class Buffer		// encapsulate the VBO
	{
	public :
		Buffer(std::vector<Vertex> const& mesh);
		Buffer() = default;

		Buffer(Buffer const&) = delete;				// delete copy to avoid double ownership
		Buffer& operator=(Buffer const&) = delete;

		Buffer(Buffer&& other) noexcept	// move 
			: m_VBO(other.m_VBO)
		{
			other.m_VBO = 0;
		}

		Buffer& operator=(Buffer&& other) noexcept
		{
			if (this != &other)
			{
				if (m_VBO != 0)
					glDeleteBuffers(1, &m_VBO);

				m_VBO = other.m_VBO;
				other.m_VBO = 0;
			}
			return *this;
		}

		~Buffer()
		{
			if (m_VBO != 0)
			{
				glDeleteBuffers(1, &m_VBO);
			}
		}
		GLuint getVBO();

	private : 
		GLuint m_VBO;
	};

	class VertexAttributes	// encapsulate the VAO
	{
	public :
		VertexAttributes(GLuint vbo, GLuint ebo);
		VertexAttributes() = default;

		VertexAttributes(const VertexAttributes&) = delete;				// delete copy to avoid double ownership
		VertexAttributes& operator=(const VertexAttributes&) = delete;

		VertexAttributes(VertexAttributes&& other) noexcept	// move
			: m_VAO(other.m_VAO)
		{
			other.m_VAO = 0;
		}

		VertexAttributes& operator=(VertexAttributes&& other) noexcept
		{
			if (this != &other)
			{
				if (m_VAO != 0)
				{
					glDeleteVertexArrays(1, &m_VAO);
				}
				m_VAO = other.m_VAO;
				other.m_VAO = 0;
			}
			return *this;
		}

		~VertexAttributes()
		{
			if (m_VAO != 0)
			{
				glDeleteVertexArrays(1, &m_VAO);
			}
		}
		GLuint getVAO();

	private :
		GLuint m_VAO;
	};

	class ElementBuffer	// encapsulate the EBO
	{
	public:
		ElementBuffer(std::vector<uint32_t> indices);
		ElementBuffer() = default;

		ElementBuffer(const ElementBuffer&) = delete;				// delete copy to avoid double ownership
		ElementBuffer& operator=(const ElementBuffer&) = delete;

		ElementBuffer(ElementBuffer&& other) noexcept		// move
			: m_EBO(other.m_EBO)
		{
			other.m_EBO = 0;
		}

		ElementBuffer& operator=(ElementBuffer&& other) noexcept
		{
			if (this != &other)
			{
				if (m_EBO != 0)
				{
					glDeleteBuffers(1, &m_EBO);
				}
				m_EBO = other.m_EBO;
				other.m_EBO = 0;
			}
			return *this;
		}

		~ElementBuffer()
		{
			if (m_EBO != 0)
			{
				glDeleteBuffers(1, &m_EBO);
			}
		}
		GLuint getEBO();

	private:
		GLuint m_EBO;
	};

	// multithread
	void parserMulti(std::string const& id, ThreadPool& pool);
	// creates nbThreads chunks of lines
	std::vector<Chunk> splitFileByLines(const std::vector<char>& buffer, unsigned int nbThreads);

	void parseChunk(std::vector<char>& buffer, Chunk chunk, std::promise<ParseResult>* promResult);

	void parseFace(std::vector<char>& file, Chunk chunk, ParseResult& result, std::promise<Something>* promResult);

	// singlethread
	void parserSingle(std::string const& id);


	Model(std::string const& id);
	~Model() = default;

	void draw();

private :

	Buffer m_VBO;
	VertexAttributes m_VAO;
	ElementBuffer m_EBO;

	std::vector<Vertex> m_mesh;
	std::vector<uint32_t> m_indices;
};