#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 u_Model;
uniform mat4 u_ModelViewProjection;
uniform mat3 u_NormalMat;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

void main()
{
	gl_Position = u_ModelViewProjection * vec4(position, 1.0);
	FragPos = (u_Model * vec4(position, 1.0)).xyz;
	Normal = mat3(transpose(inverse(u_Model))) * normal;
	
    TexCoord = aTexCoord;
}