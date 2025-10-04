#include "model.h"
#include "LibMath/Vector.h"

// Multithread parser
void Model::parserMulti(std::string const& id, ThreadPool& pool)
{
	// set the amount of thread used to parse the file
	int nbThreads = 1;

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

	std::vector<Chunk> chunks = splitFileByLines(buffer, nbThreads);
	std::vector<std::promise<ParseResult>>	promResultMesh(nbThreads);
	std::vector<std::future<ParseResult>>	futureResultMesh(nbThreads);

	for (int i = 0; i < nbThreads; i++)
	{
		futureResultMesh[i] = promResultMesh[i].get_future();
		pool.enqueue(this, &Model::parseChunk, std::ref(buffer), chunks[i], &(promResultMesh[i]));
	}

	int nbPos = 0;
	int nbTex = 0;
	int nbNor = 0;

	std::vector<ParseResult> futures;

	for (auto& f : futureResultMesh)
	{
		ParseResult result = f.get();
		futures.push_back(result);
		// Merge result into your mesh/indices
		nbPos += result.positions.size();
		nbTex += result.texCoords.size();
		nbNor += result.normals.size();
	}

	ParseResult global_result;
	global_result.positions.reserve(nbPos);
	global_result.texCoords.reserve(nbTex);
	global_result.normals.reserve(nbNor);

	for (auto& f : futures)
	{
		// Merge result into your mesh/indices
		global_result.positions.insert(global_result.positions.end(), f.positions.begin(), f.positions.end());
		global_result.texCoords.insert(global_result.texCoords.end(), f.texCoords.begin(), f.texCoords.end());
		global_result.normals.insert(global_result.normals.end(), f.normals.begin(), f.normals.end());
	}

	chunks = splitFileByLines(buffer, nbThreads);

	std::vector<std::promise<Something>>	promResultIndex(nbThreads);
	std::vector<std::future<Something>>		futureResultIndex(nbThreads);

	for (int i = 0; i < nbThreads; i++)
	{
		futureResultIndex[i] = promResultIndex[i].get_future();
		pool.enqueue(this, &Model::parseFace, std::ref(buffer), chunks[i], std::ref(global_result), &(promResultIndex[i]));
	}

	std::vector<Something> futuresSmth;

	int meshSize = 0;
	int indexSize = 0;

	for (auto& f : futureResultIndex)
	{
		Something result = f.get();
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

	m_VBO = Buffer(m_mesh);
	m_EBO = ElementBuffer(m_indices);
	m_VAO = VertexAttributes(m_VBO.getVBO(), m_EBO.getEBO());
}

std::vector<Chunk> Model::splitFileByLines(const std::vector<char>& buffer, unsigned int nbThreads)
{
	size_t chunkSize = buffer.size() / nbThreads;
	std::vector<Chunk> chunks;

	size_t current = 0;

	for (int i = 0; i < nbThreads - 1; i++)
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


// read a chunk of the file
void Model::parseChunk(std::vector<char>& file, Chunk chunk, std::promise<ParseResult>* promResult)
{
	std::vector<LibMath::Vector3> positions, normals;
	std::vector<LibMath::Vector2> texCoords;

	// temp variable to store vertices values
	LibMath::Vector3 position, normal;
	LibMath::Vector2 texCoord;

	int index = 0;

	// Map to avoid duplicate vertices: maps a string like "1/1/1" to the final vertex index
	std::unordered_map<std::string, uint32_t> vertexMap;

	size_t i = chunk.start;

	std::vector<Vertex> mesh;
	std::vector<uint32_t> indices;

	while (i < chunk.end)
	{
		size_t endLine = i;
		while (file[endLine] != '\n' && endLine < chunk.end)
		{
			endLine++;
		}

		std::string			line(file.begin() + i, file.begin() + endLine);
		std::stringstream	iss(std::string(file.begin() + i, file.begin() + endLine));
		std::string			word;

		iss >> word;
		char* c_word = word.data();

		if (c_word[0] == 'v')
		{
			if (c_word[1] == 'n')
			{
				iss >> normal.m_x >> normal.m_y >> normal.m_z;

				normals.push_back(normal);
			}
			else if (c_word[1] == 't')
			{
				iss >> texCoord.m_x >> texCoord.m_y;

				texCoords.push_back(texCoord);
			}
			else
			{
				iss >> position.m_x >> position.m_y >> position.m_z;

				positions.push_back(position);
			}
		}		

		i = 1 + endLine;
	}

	promResult->set_value({positions, normals, texCoords});
}


// parse the faces
void Model::parseFace(std::vector<char>& file, Chunk chunk, ParseResult& result, std::promise<Something>* promResult)
{
	// Map to avoid duplicate vertices: maps a string like "1/1/1" to the final vertex index
	std::unordered_map<std::string, uint32_t> vertexMap;

	size_t i = chunk.start;

	std::vector<Vertex> mesh;
	std::vector<uint32_t> indices;

	// temp variable to store vertices values
	LibMath::Vector3 position, normal;
	LibMath::Vector2 texCoord;

	int index = 0;

	while (i < chunk.end)
	{
		size_t endLine = i;
		while (file[endLine] != '\n' && endLine < chunk.end)
		{
			endLine++;
		}

		std::string			line(file.begin() + i, file.begin() + endLine);
		std::stringstream	iss(std::string(file.begin() + i, file.begin() + endLine));
		std::string			word;

		iss >> word;
		char* c_word = word.data();

		if (c_word[0] == 'f')
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
						index = index < 0 ? static_cast<int>(result.positions.size()) + index : index - 1;
						if (index >= 0 && index < result.positions.size())
							position = result.positions[index];
					}


					if (!vtStr.empty())
					{
						index = stoi(vtStr);
						index = index < 0 ? static_cast<int>(result.texCoords.size()) + index : index - 1;
						if (index >= 0 && index < result.texCoords.size())
							texCoord = result.texCoords[index];
					}


					if (!vnStr.empty())
					{
						index = stoi(vnStr);
						index = index < 0 ? static_cast<int>(result.normals.size()) + index : index - 1;
						if (index >= 0 && index < result.normals.size())
							normal = result.normals[index];
					}

					mesh.push_back(Vertex(position, normal, texCoord));

					indices.push_back(mesh.size() - 1);

					vertexMap[token] = mesh.size() - 1;
				}
				else
				{
					// Reuse the cached index for duplicate vertex
					indices.push_back(vertexMap[token]);
				}
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

		if (strcmp(c_word, "v") == 0)
		{
			iss >> position.m_x >> position.m_y >> position.m_z;

			positions.push_back(position);
		}
		else if (strcmp(c_word, "vn") == 0)
		{
			iss >> normal.m_x >> normal.m_y >> normal.m_z;

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

	m_VBO = Buffer(m_mesh);
	m_EBO = ElementBuffer(m_indices);
	m_VAO = VertexAttributes(m_VBO.getVBO(), m_EBO.getEBO());
}

Model::Model(std::string const& id)
{
	ThreadPool pool;

#ifdef TEST_TIME
	auto start = std::chrono::high_resolution_clock::now();
#endif


#ifdef MULTITHREADING
	parserMulti(id, pool);
#else
	parserSingle(id);
#endif


#ifdef TEST_TIME
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = end - start;
	printf("Parsed %s in %fs\n", id.c_str(), elapsed.count());
#endif
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

GLuint Model::Buffer::getVBO()
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

GLuint Model::VertexAttributes::getVAO()
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

GLuint Model::ElementBuffer::getEBO()
{
	return m_EBO;
}
