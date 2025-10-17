#include "model.h"
#include "LibMath/Vector.h"


float	fastPow10(int exp)
{
	static const float pow10Table[] = {
		1e-10f, 1e-9f, 1e-8f, 1e-7f, 1e-6f, 1e-5f, 1e-4f, 1e-3f, 1e-2f, 1e-1f,
		1e0f, 1e1f, 1e2f, 1e3f, 1e4f, 1e5f, 1e6f, 1e7f, 1e8f, 1e9f, 1e10f
	};
	exp = std::clamp(exp, -10, 10);
	return pow10Table[exp + 10];
}

float	parseFloat(const char*& ptr, const char* end)
{
	float result = 0.0f;
	bool neg = false;
	if (ptr < end && (*ptr == '-' || *ptr == '+'))
	{
		neg = (*ptr == '-');
		++ptr;
	}

	// integer part
	while (ptr < end && *ptr >= '0' && *ptr <= '9')
	{
		result = result * 10 + (*ptr++ - '0');
	}

	// fractional part
	if (ptr < end && *ptr == '.')
	{
		++ptr;
		float frac = 0.0f, div = 1.0f;
		while (ptr < end && *ptr >= '0' && *ptr <= '9')
		{
			frac = frac * 10 + (*ptr++ - '0');
			div *= 10;
		}
		result += frac / div;
	}

	// Scientific notation
	if (ptr < end && (*ptr == 'e' || *ptr == 'E')) 
	{
		++ptr;
		// sign
		bool expNeg = false;
		if (ptr < end && (*ptr == '-' || *ptr == '+')) 
		{
			expNeg = (*ptr == '-');
			++ptr;
		}

		int exponent = 0;
		while (ptr < end && *ptr >= '0' && *ptr <= '9')
		{
			exponent = exponent * 10 + (*ptr++ - '0');
		}

		if (expNeg)
		{
			exponent = -exponent;
		}

		result *= fastPow10(exponent);
	}

	return neg ? -result : result;
}

int		parseInt(const char* ptr, const char* end)
{
	bool neg = false;
	if (ptr < end && *ptr == '-')
	{
		neg = true; ++ptr;
	}
	else if (ptr < end && *ptr == '+')
	{
		neg = false; ++ptr;
	}
	int value = 0;

	while (ptr < end)
	{
		unsigned char c = *ptr - '0';
		if (c > 9) break;
		value = value * 10 + c;
		++ptr;
	}
	return neg ? -value : value;
}

LibMath::Vector3	parseVec3(const char* ptr, const char* end)
{
	LibMath::Vector3 position;
	while (*ptr != ' ')
	{
		++ptr;
	}

	while (*ptr == ' ') 
	{
		++ptr;
	}
	position.m_x = parseFloat(ptr, end);
	while (*ptr == ' ')
	{
		++ptr;
	}
	position.m_y = parseFloat(ptr, end);
	while (*ptr == ' ')
	{
		++ptr;
	}
	position.m_z = parseFloat(ptr, end);
	return position;
}

LibMath::Vector2	parseVec2(const char* ptr, const char* end)
{
	LibMath::Vector2 position;
	while (*ptr != ' ')
	{
		++ptr;
	}

	while (*ptr == ' ')
	{
		++ptr;
	}
	position.m_x = parseFloat(ptr, end);
	while (*ptr == ' ')
	{
		++ptr;
	}
	position.m_y = parseFloat(ptr, end);
	return position;
}

void Model::parserMultithread(std::string const& id, ThreadPool& pool)
{
	std::ifstream file("../../assets/meshes/" + id, std::ios::binary | std::ios::ate); // set cursor at the end
	if (!file.is_open())
	{
		std::cerr << "Failed to open file" << std::endl;
		return;
	}
	size_t fileSize = file.tellg();
	file.seekg(0);

	std::vector<char> buffer(fileSize);
	file.read(buffer.data(), fileSize);
	file.close();

	size_t nbChunk = buffer.size() / MIN_CHUNK_SIZE_BYTE;
	nbChunk = std::max(std::min((int)nbChunk, MAX_THREAD), 1);

	printf("Spliting %s into %zu chunks\n", id.c_str(), nbChunk);

	std::vector<Chunk> chunks = splitFileByLines(buffer, nbChunk);

	std::vector<std::promise<VerticesResult>>	promResultMesh(nbChunk);
	std::vector<std::future<VerticesResult>>	futureResultMesh(nbChunk);

	for (int i = 0; i < nbChunk; i++)
	{
		futureResultMesh[i] = promResultMesh[i].get_future();
		pool.enqueue(this, &Model::parseChunkVertices, std::ref(buffer), chunks[i], &(promResultMesh[i]));
	}
#ifdef TEST_TIME
	auto start = std::chrono::high_resolution_clock::now();
#endif
	VerticesResult global_result;
	for (auto& f : futureResultMesh)
	{
		VerticesResult result = f.get();

		global_result.m_positions.insert(global_result.m_positions.end(), result.m_positions.begin(), result.m_positions.end());
		global_result.m_texCoords.insert(global_result.m_texCoords.end(), result.m_texCoords.begin(), result.m_texCoords.end());
		global_result.m_normals.insert(global_result.m_normals.end(), result.m_normals.begin(), result.m_normals.end());
	}
#ifdef TEST_TIME
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	printf("v, vt, vn reading : %lldms\n", elapsed.count());
	start = std::chrono::high_resolution_clock::now();
#endif
	std::vector<std::promise<FacesResult>>		promResultIndex(nbChunk);
	std::vector<std::future<FacesResult>>		futureResultIndex(nbChunk);

	for (int i = 0; i < nbChunk; i++)
	{
		futureResultIndex[i] = promResultIndex[i].get_future();
		pool.enqueue(this, &Model::parseChunkFaces, std::ref(buffer), chunks[i], std::ref(global_result), &(promResultIndex[i]));
	}
	for (auto& f : futureResultIndex)
	{
		FacesResult result = f.get();

		size_t baseIndex = m_mesh.size();  // offset indices by the number of indices already there

		for (uint32_t& idx : result.m_indices)
		{
			idx += static_cast<uint32_t>(baseIndex);
		}
		m_indices.insert(m_indices.end(), result.m_indices.begin(), result.m_indices.end());
		m_mesh.insert(m_mesh.end(), result.m_mesh.begin(), result.m_mesh.end());
	}
#ifdef TEST_TIME
	end = std::chrono::high_resolution_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	printf("Face parsing : %lldms\n", elapsed.count());
#endif
}

std::vector<Chunk> Model::splitFileByLines(const std::vector<char>& buffer, size_t nbChunk)
{
	size_t chunkSize = buffer.size() / nbChunk;
	std::vector<Chunk> chunks;

	size_t current = 0;

	for (int i = 0; i < nbChunk - 1; i++)
	{
		size_t start = current;

		size_t end = start + chunkSize;

		while (end < buffer.size() && buffer[end] != '\n')
			++end;

		chunks.push_back({ start,end });

		current = end + 1;
	}

	chunks.push_back({ current,buffer.size() });

	return chunks;
}

void	Model::parseChunkVertices(std::vector<char>& file, Chunk chunk, std::promise<VerticesResult>* promResult)
{
	std::vector<LibMath::Vector3>	positions, normals;
	std::vector<LibMath::Vector2>	texCoords;

	size_t	i = chunk.m_start;

	while (i < chunk.m_end)
	{
		size_t	endLine = i;
		while (endLine < chunk.m_end && file[endLine] != '\n')
		{
			endLine++;
		}
		std::string_view	line(&file[i], endLine - i);

		// Skip empty lines
		if (line.empty())
		{
			i = ++endLine;
			continue;
		}
		// Check the first character(s)
		char	first = line[0];
		char	second = line.size() > 1 ? line[1] : '\0';

		const char*		ptr = line.data();
		const char*		end = ptr + line.size();

		if (first == 'v' && second == ' ') 
		{
			positions.push_back(parseVec3(ptr, end));
		}
		else if (first == 'v' && second == 'n') 
		{
			normals.push_back(parseVec3(ptr, end));
		}
		else if (first == 'v' && second == 't') 
		{
			texCoords.push_back(parseVec2(ptr, end));
		}
		i = ++endLine;
	}
	promResult->set_value({positions, normals, texCoords});
}

void	Model::parseChunkFaces(std::vector<char>& file, Chunk chunk, VerticesResult& result, std::promise<FacesResult>* promResult)
{
	size_t i = chunk.m_start;

	std::vector<Vertex> mesh;
	std::vector<uint32_t> indices;

	const int avgLineLengthEstimate = 30;
	mesh.reserve((chunk.m_end - chunk.m_start) / avgLineLengthEstimate);
	indices.reserve(mesh.capacity() * 3);

	LibMath::Vector3 position, normal;
	LibMath::Vector2 texCoord;

	while (i < chunk.m_end)
	{
		size_t endLine = i;
		while (endLine < chunk.m_end && file[endLine] != '\n')
			endLine++;

		const char* ptr = &file[i];
		const char* end = &file[endLine];

		if (ptr >= end || *ptr != 'f')
		{
			i = ++endLine;
			continue;
		}
		ptr++; // skip 'f'

		// --- Per-face state ---
		int firstIdx = -1;
		int prevIdx = -1;
		int currIdx = -1;
		int vertexCount = 0;

		while (ptr < end && *ptr != '\n' && *ptr != '\r')
		{
			// skip spaces
			while (ptr < end && *ptr == ' ')
				++ptr;
			if (ptr >= end) break;

			int vIdx = 0, vtIdx = 0, vnIdx = 0;

			// v
			const char* start = ptr;
			while (ptr < end && *ptr != '/' && *ptr != ' ' && *ptr != '\n' && *ptr != '\r')
				++ptr;
			if (ptr > start)
				vIdx = parseInt(start, ptr);

			// vt
			if (ptr < end && *ptr == '/')
				++ptr;
			start = ptr;
			while (ptr < end && *ptr != '/' && *ptr != ' ' && *ptr != '\n' && *ptr != '\r')
				++ptr;
			if (ptr > start)
				vtIdx = parseInt(start, ptr);

			// vn
			if (ptr < end && *ptr == '/')
				++ptr;
			start = ptr;
			while (ptr < end && *ptr != ' ' && *ptr != '\n' && *ptr != '\r')
				++ptr;
			if (ptr > start)
				vnIdx = parseInt(start, ptr);

			// convert to 0-based indices
			vIdx = vIdx < 0 ? (int)result.m_positions.size() + vIdx : vIdx - 1;
			vtIdx = vtIdx < 0 ? (int)result.m_texCoords.size() + vtIdx : vtIdx - 1;
			vnIdx = vnIdx < 0 ? (int)result.m_normals.size() + vnIdx : vnIdx - 1;

			if (vIdx >= 0) position = result.m_positions[vIdx];
			if (vtIdx >= 0) texCoord = result.m_texCoords[vtIdx];
			if (vnIdx >= 0) normal = result.m_normals[vnIdx];

			mesh.push_back(Vertex(position, normal, texCoord));
			currIdx = static_cast<int>(mesh.size() - 1);

			// triangulate on the fly
			if (vertexCount == 0)
			{
				firstIdx = currIdx;
			}
			else if (vertexCount == 1)
			{
				prevIdx = currIdx;
			}
			else
			{
				indices.push_back(firstIdx);
				indices.push_back(prevIdx);
				indices.push_back(currIdx);
				prevIdx = currIdx;
			}
			vertexCount++;
		}

		i = ++endLine;
	}

	promResult->set_value({ mesh, indices });
}



void	Model::parserSinglethread(std::string const& id)
{
	// Open the file in binary mode and move cursor to end
	std::ifstream file("../../assets/meshes/" + id, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << id << std::endl;
		return;
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	file.seekg(0);

	std::vector<char> buffer(fileSize);
	file.read(buffer.data(), fileSize);
	file.close();

	if (buffer.empty())
	{
		std::cerr << "File is empty: " << id << std::endl;
		return;
	}

	if (buffer.back() != '\n')
		buffer.push_back('\n');

	// Storage for parsed data
	std::vector<LibMath::Vector3> positions, normals;
	std::vector<LibMath::Vector2> texCoords;

	LibMath::Vector3 position, normal;
	LibMath::Vector2 texCoord;

	std::unordered_map<uint64_t, uint32_t> vertexMap;

	size_t i = 0;

	while (i < buffer.size())
	{
		size_t endLine = i;
		while (endLine < buffer.size() && buffer[endLine] != '\n')
			++endLine;

		std::string_view line(&buffer[i], endLine - i);
		if (line.empty() || line[0] == '#')
		{
			i = ++endLine;
			continue;
		}
		char first = line[0];
		char second = line.size() > 1 ? line[1] : '\0';
		const char* ptr = line.data();
		const char* end = ptr + line.size();

		if (first == 'v' && second == ' ')
		{
			positions.push_back(parseVec3(ptr, end));
		}
		else if (first == 'v' && second == 't')
		{
			texCoords.push_back(parseVec2(ptr, end));
		}
		else if (first == 'v' && second == 'n')
		{
			normals.push_back(parseVec3(ptr, end));
		}
		// ---- Parse Faces ----
		else if (first == 'f' && second == ' ')
		{
			ptr += 2; // skip 'f '

			for (int vi = 0; vi < 3; ++vi)
			{
				int vIdx = 0, vtIdx = 0, vnIdx = 0;

				while (ptr < end && *ptr == ' ')
					++ptr;
				if (ptr >= end) break;

				const char* start = ptr;
				while (ptr < end && *ptr != '/' && *ptr != ' ')
					++ptr;
				if (ptr > start)
					vIdx = parseInt(start, ptr);

				// vt
				if (ptr < end && *ptr == '/')
					++ptr;
				start = ptr;
				while (ptr < end && *ptr != '/' && *ptr != ' ')
					++ptr;
				if (ptr > start)
					vtIdx = parseInt(start, ptr);

				// vn
				if (ptr < end && *ptr == '/')
					++ptr;
				start = ptr;
				while (ptr < end && *ptr != ' ' && ptr < end)
					++ptr;
				if (ptr > start)
					vnIdx = parseInt(start, ptr);

				// Pack into 64-bit key for deduplication
				uint64_t key = ((uint64_t)vIdx << 40) | ((uint64_t)vtIdx << 20) | (uint64_t)vnIdx;

				auto it = vertexMap.find(key);
				if (it == vertexMap.end())
				{
					vIdx = vIdx < 0 ? (int)positions.size() + vIdx : vIdx - 1;
					vtIdx = vtIdx < 0 ? (int)texCoords.size() + vtIdx : vtIdx - 1;
					vnIdx = vnIdx < 0 ? (int)normals.size() + vnIdx : vnIdx - 1;

					if (vIdx >= 0 && vIdx < (int)positions.size()) position = positions[vIdx];
					if (vtIdx >= 0 && vtIdx < (int)texCoords.size()) texCoord = texCoords[vtIdx];
					if (vnIdx >= 0 && vnIdx < (int)normals.size()) normal = normals[vnIdx];

					m_mesh.push_back(Vertex(position, normal, texCoord));
					uint32_t idx = static_cast<uint32_t>(m_mesh.size() - 1);
					m_indices.push_back(idx);
					vertexMap[key] = idx;
				}
				else
				{
					m_indices.push_back(it->second);
				}
			}
		}

		i = ++endLine;
	}
}

void	Model::uploadToGPU()
{
	m_VBO = Buffer(m_mesh);
	m_EBO = ElementBuffer(m_indices);
	m_VAO = VertexAttributes(m_VBO.getVBO(), m_EBO.getEBO());
}


Model::Model(std::string const& id, ThreadPool& pool)
{
	parserMultithread(id, pool);
}

Model::Model(std::string const& id)
{
	parserSinglethread(id);
}

void Model::draw()
{
	glPolygonMode(GL_FRONT, GL_FILL);
	glBindVertexArray(m_VAO.getVAO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO.getEBO());
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
}

Vertex::Vertex(LibMath::Vector3 pos, LibMath::Vector3 normal, LibMath::Vector2 texCoord)
{
	m_position = pos;
	m_normal = normal;
	m_textureUV = texCoord;
}

Model::Buffer::Buffer(std::vector<Vertex> const& mesh)
{
	glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(Vertex), mesh.data(), GL_STATIC_DRAW);
}

GLuint Model::Buffer::getVBO() const
{
	return m_VBO;
}

Model::VertexAttributes::VertexAttributes(GLuint vbo, GLuint ebo)
{
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	// mesh :  pos_x, pos_y, pos_z, nor_x, nor_y, nor_z, tex_u, tex_v

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);	// pos
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));	// nor
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));	// tex
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

GLuint Model::VertexAttributes::getVAO() const
{
	return m_VAO;
}

Model::ElementBuffer::ElementBuffer(std::vector<uint32_t> indices)
{
	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
}

GLuint Model::ElementBuffer::getEBO() const
{
	return m_EBO;
}
