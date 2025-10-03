#include "graph.h"
//#include "memoryLeak.h"
Graph::SceneNode* Graph::addChild(SceneNode* parent, Mesh data)
{
	std::unique_ptr<SceneNode> newNode = std::make_unique<SceneNode>(data, parent);
	SceneNode* newNodeRaw = newNode.get(); // Save raw pointer for return

	if (parent == nullptr)
	{
		// Assign ownership to root
		m_root = std::move(newNode);
	}
	else
	{
		// Add to parent's children
		parent->m_children.push_back(std::move(newNode));
	}

	return newNodeRaw; // Safe to return non-owning raw pointer

	////set the node as the root if it has no parent
	//if (parent == nullptr)
	//{
	//	m_root = new SceneNode{ data, nullptr };
	//	return m_root;
	//}

	////create a node for the parent
	//SceneNode* child = new SceneNode{ data, parent };
	//parent->m_children.push_back(child);


	//return child;
}

void Graph::startDrawing(Camera const& camera, Shader const& shader)
{
	//draw from the root
	if (m_root) { m_root->drawNodes(camera, shader); }
}

LibMath::Matrix4 Graph::SceneNode::getWorldTransorm()
{
	//check if the node have a aprent
	if (m_parent == nullptr) { return m_data.getTransform(); }

	//multiply the node transform by its parent transform -> then repeat the process with the parent node
	return m_parent->getWorldTransorm() * m_data.getTransform();
}

void Graph::SceneNode::setTransform(const LibMath::Matrix4 trans)
{
	m_data.setTransform(trans);
}

void Graph::SceneNode::setTexture(const Texture& tex)
{
	m_texture = tex;
}

void Graph::SceneNode::drawNodes(Camera const& camera, Shader const& shader)
{
	//set texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture.getID());

	//get world transform
	LibMath::Matrix4 worldTransform = getWorldTransorm();

	//draw the current node
	m_data.draw(camera, shader, worldTransform);

	for (size_t i = 0; i < m_children.size(); i++)
	{
		m_children[i]->drawNodes(camera, shader);
	}
}

