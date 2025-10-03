#pragma once

#include <string>
#include <vector>
#include "LibMath/Matrix.h"

#include "texture.h"
#include "mesh.h"

struct Graph
{
public:

	//data
	struct SceneNode
	{
	public:
		//object, rotation and texture
		Mesh				m_data;
		Texture				m_texture;
			
		//destructor
		~SceneNode()
		{
			//std::cout << "Destroying SceneNode at " << static_cast<const void*>(this) << std::endl;
			//for (SceneNode* child : m_children)
			//{
			//	delete child;
			//}
		}

		//nodes
		SceneNode*				m_parent = nullptr;
		std::vector<std::unique_ptr<SceneNode>>	m_children;

		//constructor
		SceneNode(const Mesh& data, SceneNode* parent = nullptr)
			: m_data(data), m_parent(parent) {}

		SceneNode(SceneNode const&) = delete;				// delete copy cause unique_ptr == non copyable
		SceneNode& operator=(SceneNode const&) = delete;

		LibMath::Matrix4	getWorldTransorm();

		void				setTransform(const LibMath::Matrix4);

		void				setTexture(const Texture&);

		void				drawNodes(Camera const& camera, Shader const& shader);
	};

	std::unique_ptr<SceneNode> m_root = nullptr;

	//destructor
	~Graph()
	{
		//if (m_root)
		//{
		//	delete m_root;
		//	m_root = nullptr;
		//}
	}

	//functions
	SceneNode*	addChild(SceneNode* parent, Mesh data);

	void		startDrawing(Camera const& camera, Shader const& shader);
};