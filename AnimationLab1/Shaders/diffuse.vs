#version 330

//This is the layout convention accross all shader files
layout(location = 0) in vec3 vertex_position;
//layout(location = 1) in vec3 vertex_normal;
//layout(location = 2) in vec2 texture_coord;
//layout(location = 3) in vec4 bone_ids;
//layout(location = 4) in vec4 bone_weights;

uniform mat4 mvpMatrix;

uniform vec4 uniformColour = vec4(1.0, 0.0, 0.0, 0.5);

out vec4 colour;

void main()
{
	vec4 Vertex = vec4(vertex_position.x, vertex_position.y, vertex_position.z, 1.0);
	gl_Position = mvpMatrix * Vertex;
	
	colour = uniformColour;
}