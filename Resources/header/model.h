#pragma once

#include "resourceManager.h"
#include "LibMath/Vector.h"

#include <future>
#include <thread>
#include <mutex>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define MIN_CHUNK_SIZE_BYTE 250'000
#define MAX_THREAD 4

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
	size_t	m_start;
	size_t	m_end;
};

// used for the return of vertices in thread
struct VerticesResult
{
	std::vector<LibMath::Vector3>	m_positions;
	std::vector<LibMath::Vector3>	m_normals;
	std::vector<LibMath::Vector2>	m_texCoords;
};

// used for the return of faces in thread
struct FacesResult
{
	std::vector<Vertex> m_mesh;
	std::vector<uint32_t> m_indices;
};

struct FaceIndex
{
	int vIdx;
	int vtIdx;
	int vnIdx;
};

inline float		fastPow10(int exp);
inline float		parseFloat(const char*& ptr, const char* end);
inline int			parseInt(const char* ptr, const char* end);
LibMath::Vector3	parseVec3(const char* ptr, const char* end);
LibMath::Vector2	parseVec2(const char* ptr, const char* end);
FaceIndex			parseIndices(const char* ptr, const char* end, VerticesResult& result);

class Model : public IResource
{
public :

	// encapsulate the VBO
	class Buffer
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
		GLuint getVBO() const;

	private : 
		GLuint m_VBO;
	};

	// encapsulate the VAO
	class VertexAttributes
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
		GLuint getVAO() const;

	private :
		GLuint m_VAO;
	};

	// encapsulate the EBO
	class ElementBuffer
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
		GLuint getEBO() const;

	private:
		GLuint m_EBO;
	};

	Model(std::string const& id, ThreadPool& pool);
	Model(std::string const& id);
	~Model() = default;

	void	parserMultithread(std::string const& id, ThreadPool& pool);

	std::vector<Chunk>	splitFileByLines(const std::vector<char>& file, size_t nbChunk);

	void	parseChunkVertices(std::vector<char>& file, Chunk chunk, std::promise<VerticesResult>* promResult);

	void	parseChunkFaces(std::vector<char>& file, Chunk chunk, VerticesResult& result, std::promise<FacesResult>* promResult);

	void	parserSinglethread(std::string const& id);
	void	uploadToGPU();
	void	draw();

private :
	
	Buffer					m_VBO;
	VertexAttributes		m_VAO;
	ElementBuffer			m_EBO;

	std::vector<Vertex>		m_mesh;
	std::vector<uint32_t>	m_indices;
};