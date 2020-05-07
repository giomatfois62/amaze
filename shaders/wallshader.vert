#version 330 core
layout (location = 0) in vec3 position;   // the position variable has attribute position 0
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color; // the color variable has attribute position 1
layout (location = 3) in vec2 texCoords; 
layout (location = 4) in mat4 model;
layout (location = 9) in float sol;
  
out vec3 vColor; // output a color to the fragment shader
out vec3 vNormal;
out vec2 vTexCoords;

//uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
	vNormal = mat3(transpose(inverse(view * model))) * normal;
	vColor = color; // set ourColor to the input color we got from the vertex data
	vTexCoords = texCoords;
}
