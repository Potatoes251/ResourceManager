#include "mesh.h"

Mesh::Mesh(Model* model, LibMath::Matrix4 transform)
{
	m_model = model;
	m_transform = transform;
}

void Mesh::draw(Camera const& camera, Shader const& shader)
{
	LibMath::Matrix4 MVP = camera.getVPMatrix() * m_transform;

	LibMath::Matrix3x3 normalMatrix = m_transform.getMinor(3,3);
	normalMatrix.inverse();
	normalMatrix.transpose();

	//send it to the shader
	shader.setUniform("u_MVP", MVP);
	shader.setUniform("u_NormalMat", normalMatrix);

	m_model->draw();
}

void Mesh::draw(Camera const& camera, Shader const& shader, LibMath::Matrix4 trans)
{
	LibMath::Matrix4 MVP = camera.getVPMatrix() * trans;

	LibMath::Matrix3x3 normalMatrix = trans.getMinor(3, 3);
	normalMatrix.inverse();
	normalMatrix.transpose();

	//send it to the shader
	shader.setUniform("u_MVP", MVP);
	shader.setUniform("u_NormalMat", normalMatrix);

	m_model->draw();
}

LibMath::Matrix4 Mesh::getTransform() const
{
	return m_transform;
}

void Mesh::setTransform(const LibMath::Matrix4& trans)
{
	m_transform = trans;
}
