#include "model.h"
#include "LibMath/Vector.h"

// Multithread parser
void Model::parserMulti(std::string const& id, ThreadPool& pool)
{
	// open the file and set cursor at the end
	std::ifstream file("../../assets/meshes/" + id, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		// handle file not found
		std::cerr << "Failed to open file" << std::endl;
		return;
	}
	size_t fileSize = file.tellg();
	file.seekg(0);

	std::vector<char> buffer(fileSize);
	file.read(buffer.data(), fileSize);
	file.close();

	if (buffer[buffer.size() - 1] != '\n')
	{
		buffer.push_back('\n');
	}

	size_t nbChunk = buffer.size() / MIN_CHUNK_SIZE_BYTE;
	nbChunk = std::max(std::min((int)nbChunk, MAX_THREAD), 1);

	printf("Spliting %s into %zu chunks\n", id.c_str(), nbChunk);

	std::vector<Chunk> chunks = splitFileByLines(buffer, nbChunk);
	std::vector<std::promise<VerticesResult>>	promResultMesh(nbChunk);
	std::vector<std::future<VerticesResult>>	futureResultMesh(nbChunk);

	for (int i = 0; i < nbChunk; i++)
	{
		futureResultMesh[i] = promResultMesh[i].get_future();
		pool.enqueue(this, &Model::parseChunk, std::ref(buffer), chunks[i], &(promResultMesh[i]));
	}

	int nbPos = 0;
	int nbTex = 0;
	int nbNor = 0;

	std::vector<VerticesResult> futures;

	for (auto& f : futureResultMesh)
	{
		VerticesResult result = f.get();
		futures.push_back(result);
		// Merge result into your mesh/indices
		nbPos += result.m_positions.size();
		nbTex += result.m_texCoords.size();
		nbNor += result.m_normals.size();
	}

	VerticesResult global_result;
	global_result.m_positions.reserve(nbPos);
	global_result.m_texCoords.reserve(nbTex);
	global_result.m_normals.reserve(nbNor);

	for (auto& f : futures)
	{
		// Merge result into your mesh/indices
		global_result.m_positions.insert(global_result.m_positions.end(), f.m_positions.begin(), f.m_positions.end());
		global_result.m_texCoords.insert(global_result.m_texCoords.end(), f.m_texCoords.begin(), f.m_texCoords.end());
		global_result.m_normals.insert(global_result.m_normals.end(), f.m_normals.begin(), f.m_normals.end());
	}

	std::vector<std::promise<FacesResult>>	promResultIndex(nbChunk);
	std::vector<std::future<FacesResult>>		futureResultIndex(nbChunk);

	for (int i = 0; i < nbChunk; i++)
	{
		futureResultIndex[i] = promResultIndex[i].get_future();
		pool.enqueue(this, &Model::parseFace, std::ref(buffer), chunks[i], std::ref(global_result), &(promResultIndex[i]));
	}

	std::vector<FacesResult> futuresSmth;

	int meshSize = 0;
	int indexSize = 0;

	for (auto& f : futureResultIndex)
	{
		FacesResult result = f.get();
		futuresSmth.push_back(result);
		// Merge result into your mesh/indices
		meshSize += result.m_mesh.size();
		indexSize += result.m_indices.size();
	}

	m_indices.reserve(indexSize);
	m_mesh.reserve(meshSize);

	for (auto& f : futuresSmth)
	{
		size_t baseIndex = m_mesh.size();  // offset = current vertex count

		// adjust indices from this chunk
		for (uint32_t& idx : f.m_indices)
		{
			idx += static_cast<uint32_t>(baseIndex);
		}

		// Merge result into your mesh/indices
		m_indices.insert(m_indices.end(), f.m_indices.begin(), f.m_indices.end());
		m_mesh.insert(m_mesh.end(), f.m_mesh.begin(), f.m_mesh.end());
	}
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
		{
			++end;
		}

		chunks.push_back({ start,end });

		current = end + 1;
	}

	chunks.push_back({ current,buffer.size() });

	return chunks;
}


float parseFloat(const char*& ptr, const char* end)
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

		result *= std::pow(10.0f, exponent);
	}

	return neg ? -result : result;
}

int parseInt(const char* ptr, const char* end)
{
	bool neg = false;
	if (ptr < end && *ptr == '-') { neg = true; ++ptr; }

	else if (ptr < end && *ptr == '+') { neg = false; ++ptr; }
	int value = 0;
	while (ptr < end && *ptr >= '0' && *ptr <= '9')
		value = value * 10 + (*ptr++ - '0');
	return neg ? -value : value;
}

// read a chunk of the file
void Model::parseChunk(std::vector<char>& file, Chunk chunk, std::promise<VerticesResult>* promResult)
{
	std::vector<LibMath::Vector3> positions;
	std::vector<LibMath::Vector3> normals;
	std::vector<LibMath::Vector2> texCoords;

	// temp variable to store vertices values
	LibMath::Vector3 position, normal;
	LibMath::Vector2 texCoord;

	int index = 0;

	// Map to avoid duplicate vertices: maps a string like "1/1/1" to the final vertex index
	std::unordered_map<std::string, uint32_t> vertexMap;

	size_t i = chunk.m_start;

	std::vector<Vertex> mesh;
	std::vector<uint32_t> indices;

	while (i < chunk.m_end)
	{
		size_t endLine = i;
		while (endLine < chunk.m_end && file[endLine] != '\n')
		{
			endLine++;
		}

		std::string_view	line(&file[i], endLine - i);

		// Skip empty lines
		if (!line.empty())
		{
			// Check the first character(s)
			char first = line[0];
			char second = line.size() > 1 ? line[1] : '\0';

			const char* ptr = line.data();
			const char* end = ptr + line.size();

			if (first == 'v' && second == ' ') 
			{
				while (*ptr != ' ') ++ptr; ++ptr;
				position.m_x = parseFloat(ptr, end);
				while (*ptr == ' ') ++ptr;
				position.m_y = parseFloat(ptr, end);
				while (*ptr == ' ') ++ptr;
				position.m_z = parseFloat(ptr, end);
				positions.push_back(position);
			}
			else if (first == 'v' && second == 'n') 
			{
				while (*ptr != ' ') ++ptr; ++ptr;
				normal.m_x = parseFloat(ptr, end);
				while (*ptr == ' ') ++ptr;
				normal.m_y = parseFloat(ptr, end);
				while (*ptr == ' ') ++ptr;
				normal.m_z = parseFloat(ptr, end);
				normals.push_back(normal);
			}
			else if (first == 'v' && second == 't') 
			{
				while (*ptr != ' ') ++ptr; ++ptr;
				texCoord.m_x = parseFloat(ptr, end);
				while (*ptr == ' ') ++ptr;
				texCoord.m_y = parseFloat(ptr, end);
				texCoords.push_back(texCoord);
			}
		}

		i = 1 + endLine;
	}

	promResult->set_value({positions, normals, texCoords});
}


// parse the faces
void Model::parseFace(std::vector<char>& file, Chunk chunk, VerticesResult& result, std::promise<FacesResult>* promResult)
{
	// Map to avoid duplicate vertices: maps a string like "1/1/1" to the final vertex index
	std::unordered_map<uint64_t, uint32_t> vertexMap;

	size_t i = chunk.m_start;

	std::vector<Vertex> mesh;
	std::vector<uint32_t> indices;

	// temp variable to store vertices values
	LibMath::Vector3 position, normal;
	LibMath::Vector2 texCoord;

	while (i < chunk.m_end)
	{
		size_t endLine = i;
		while (file[endLine] != '\n' && endLine < chunk.m_end)
		{
			endLine++;
		}

        const char* ptr = &file[i];
        const char* end = &file[endLine];

        // Skip empty lines or non-face lines
        if (ptr < end && *ptr == 'f')
        {
            ptr += 2; // skip 'f' and a space

            for (int vi = 0; vi < 3; ++vi)
            {
                int vIdx = 0, vtIdx = 0, vnIdx = 0;

                // v
                while (ptr < end && *ptr == ' ') ++ptr;
                const char* start = ptr;
                while (ptr < end && *ptr != '/' && *ptr != ' ')
				{
					++ptr;
				}
                if (ptr > start) vIdx = parseInt(start, ptr);

                // vt
                if (ptr < end && *ptr == '/') 
				{
					++ptr; 
					start = ptr;
				}
                while (ptr < end && *ptr != '/' && *ptr != ' ') 
				{
					++ptr;
				}
                if (ptr > start) vtIdx = parseInt(start, ptr);

                // vn
				if (ptr < end && *ptr == '/')
				{
					++ptr;
					start = ptr;
				}
                while (ptr < end && *ptr != ' ') 
				{
					++ptr;
				}
                if (ptr > start) vnIdx = parseInt(start, ptr);

				// Pack indices into a 64-bit key
                //uint64_t key = ((uint64_t)vIdx << 40) | ((uint64_t)vtIdx << 20) | (uint64_t)vnIdx;

                //auto it = vertexMap.find(key);
                //if (it == vertexMap.end())
                {
                    vIdx = vIdx < 0 ? static_cast<int>(result.m_positions.size()) + vIdx : vIdx - 1;;
                    vtIdx = vtIdx < 0 ? static_cast<int>(result.m_texCoords.size()) + vtIdx : vtIdx - 1;;
                    vnIdx = vnIdx < 0 ? static_cast<int>(result.m_normals.size()) + vnIdx : vnIdx - 1;;

                    if (vIdx >= 0) position = result.m_positions[vIdx];
                    if (vtIdx >= 0) texCoord = result.m_texCoords[vtIdx];
                    if (vnIdx >= 0) normal = result.m_normals[vnIdx];

                    mesh.push_back(Vertex(position, normal, texCoord));
                    indices.push_back(mesh.size() - 1);
                    //vertexMap[key] = mesh.size() - 1;
                }
                //else
                //{
                //    indices.push_back(it->second);
                //}
            }
		}


		i = 1 + endLine;
	}

	promResult->set_value({ mesh, indices });
}



// Old Parser
void Model::parserSingle(std::string const& id)
{
	std::ifstream file("../../assets/meshes/" + id);	// open the file
	if (!file.is_open()) 
	{
		std::cerr << "Failed to open file" << std::endl;	// handle file not found
		return;
	}

	// buffer vectors to stores extracted values from "v", "vt" and "vn"
	std::vector<LibMath::Vector3> positions, normals;
	std::vector<LibMath::Vector2> texCoords;


	std::string line, word;
	char* c_word;

	// temp variable to store vertices values
	LibMath::Vector3 position, normal;
	LibMath::Vector2 texCoord;
	int index = 0;

	// Map to avoid duplicate vertices: maps a string like "1/1/1" to the final vertex index
	std::unordered_map<std::string, uint32_t> vertexMap;

	while (std::getline(file, line))	// go through the file line per line
	{
		std::stringstream	iss(line);
		iss >> word;
		c_word = word.data();

		// Check the first character(s)
		char first = line[0];
		char second = line.size() > 1 ? line[1] : '\0';

		const char* ptr = line.data();
		const char* end = ptr + line.size();

		if (first == 'v' && second == ' ')
		{
			while (*ptr != ' ') ++ptr; ++ptr;
			position.m_x = parseFloat(ptr, end);
			while (*ptr == ' ') ++ptr;
			position.m_y = parseFloat(ptr, end);
			while (*ptr == ' ') ++ptr;
			position.m_z = parseFloat(ptr, end);
			positions.push_back(position);
		}
		else if (first == 'v' && second == 'n')
		{
			while (*ptr != ' ') ++ptr; ++ptr;
			normal.m_x = parseFloat(ptr, end);
			while (*ptr == ' ') ++ptr;
			normal.m_y = parseFloat(ptr, end);
			while (*ptr == ' ') ++ptr;
			normal.m_z = parseFloat(ptr, end);
			normals.push_back(normal);
		}
		else if (strcmp(c_word, "vt") == 0)
		{
			iss >> texCoord.m_x >> texCoord.m_y;

			texCoords.push_back(texCoord);
		}
		else if (strcmp(c_word, "f") == 0)
		{
			char del = '/';
			for (int i = 0; i < 3; ++i)
			{

				std::string token;
				iss >> token; // Read one vertex string like "1/1/1"

				if (vertexMap.count(token) == 0)
				{
					std::stringstream tokenStream(token);
					std::string vStr, vtStr, vnStr;

					getline(tokenStream, vStr, del);
					getline(tokenStream, vtStr, del);
					getline(tokenStream, vnStr, del);

					if (!vStr.empty())
					{
						index = stoi(vStr);
						index = index < 0 ? static_cast<int>(positions.size()) + index : index - 1;
						if (index >= 0 && index < positions.size())
							position = positions[index];
					}


					if (!vtStr.empty())
					{
						index = stoi(vtStr);
						index = index < 0 ? static_cast<int>(texCoords.size()) + index : index - 1;
						if (index >= 0 && index < texCoords.size())
							texCoord = texCoords[index];
					}


					if (!vnStr.empty())
					{
						index = stoi(vnStr);
						index = index < 0 ? static_cast<int>(normals.size()) + index : index - 1;
						if (index >= 0 && index < normals.size())
							normal = normals[index];
					}

					m_mesh.push_back(Vertex(position, normal, texCoord));

					m_indices.push_back(m_mesh.size() - 1);

					vertexMap[token] = m_mesh.size() - 1;
				}
				else
				{
					// Reuse the cached index for duplicate vertex
					m_indices.push_back(vertexMap[token]);
				}
			}
		}

	}

	file.close();
}

void Model::uploadToGPU()
{
	m_VBO = Buffer(m_mesh);
	m_EBO = ElementBuffer(m_indices);
	m_VAO = VertexAttributes(m_VBO.getVBO(), m_EBO.getEBO());
}


Model::Model(std::string const& id, ThreadPool& pool)
{
	parserMulti(id, pool);
}

Model::Model(std::string const& id)
{
	parserSingle(id);

}

void Model::draw()
{
	glPolygonMode(GL_FRONT, GL_FILL);
	glBindVertexArray(m_VAO.getVAO());
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
		indices.data(), GL_STATIC_DRAW);
}

GLuint Model::ElementBuffer::getEBO() const
{
	return m_EBO;
}
