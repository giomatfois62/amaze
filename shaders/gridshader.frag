#version 330 core

in vec3 vColor;
in vec3 vNormal;
in vec2 vTexCoords;
flat in float vSol;

uniform sampler2D texture_diffuse1;

out vec4 FragColor;

void main()
{
	vec3 color = vColor; // not visited cell = 5

	if (vSol < 2)
		color = vec3(1,0,0); // current cell = 1
	else if (vSol < 3)
		color = vec3(0,1,0); // visited cell = 2
	else if (vSol < 4)
		color = vec3(0,0,1); // end cell = 3

        //FragColor = vec4(intensity * vColor * texture(texture_diffuse1, vTexCoords).xyz, 1.0);
	FragColor = vec4(color, 1.0);
}
