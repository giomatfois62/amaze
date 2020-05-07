#version 330 core

in vec3 vColor;
in vec3 vNormal;
in vec2 vTexCoords;

uniform sampler2D texture_diffuse1;

out vec4 FragColor;

float clamp(float x, float mi, float ma)
{
    return max(min(x, ma), mi);
}

  
void main()
{
        vec3 lightDir = vec3(0.0, 0.0, 1.0);
        float intensity = max(dot(vNormal, lightDir), 0.0);

	//FragColor = vec4(intensity * vColor * texture(texture_diffuse1, vTexCoords).xyz, 1.0);
	FragColor = vec4(intensity * vColor, 1.0);
	//FragColor = vec4(r, g, b, 1.0);
}
