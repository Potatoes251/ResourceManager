#pragma once

#include "camera.h"
#include "shader.h"
#include "model.h"
#include "LibMath/Matrix.h"

class Mesh
{
public :

	Mesh(Model* model, LibMath::Matrix4 transform);

	void draw(Camera const& camera, Shader const& shader);

	void draw(Camera const& camera, Shader const& shader, LibMath::Matrix4);



	LibMath::Matrix4	getTransform() const;

	void				setTransform(const LibMath::Matrix4);

private :
	Model* m_model;
	LibMath::Matrix4 m_transform;
};