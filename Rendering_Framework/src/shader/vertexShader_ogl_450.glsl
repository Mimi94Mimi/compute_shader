#version 450 core

layout(location=0) in vec3 v_vertex;
layout(location=1) in vec3 texcoord;
layout(location=2) in vec3 normal;
layout(location=3) in vec4 offset_texture;

out vec3 f_worldVertex;
out vec3 f_viewVertex;
out vec3 f_texcoord;
out vec3 f_normal;
out float texture_id;

layout(location = 0) uniform mat4 modelMat ;
layout(location = 1) uniform mat4 viewMat ;
layout(location = 2) uniform mat4 projMat ;
layout (location = 5) uniform int shadingModelId;

struct DrawCommand{
    uint count ;
    uint instanceCount ;
    uint firstIndex ;
    uint baseVertex ;
    uint baseInstance ;
};

void main(){
    vec4 worldVertex = modelMat * vec4(v_vertex, 1.0) + vec4(offset_texture.xyz, 0);
    vec4 worldNormal = modelMat * vec4(normal, 0.0);
    
	vec4 viewVertex = viewMat * worldVertex;
    vec4 viewNormal = viewMat * worldNormal;
	
	f_worldVertex = worldVertex.xyz;
	f_viewVertex = viewVertex.xyz;
    f_normal = viewNormal.xyz;
	
	gl_Position = projMat * viewVertex;

    f_texcoord = texcoord;

    texture_id = offset_texture[3];

}