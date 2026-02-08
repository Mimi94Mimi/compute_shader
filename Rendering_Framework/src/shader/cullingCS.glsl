layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in ;

struct DrawCommand{
    uint count ;
    uint instanceCount ;
    uint firstIndex ;
    uint baseVertex ;
    uint baseInstance ;
};

struct InstanceProperties{
    vec4 offset_texture ;
};

/* the SSBO for storing draw commands */
layout (std430, binding=3) buffer DrawCommandsBlock{
    DrawCommand commands[] ;
};

/*the buffer for storing “whole” instance position and other necessary information*/
layout (std430, binding=1) buffer RawInstanceData
{
    InstanceProperties rawInstanceProps[] ;
};
/*the buffer for storing “visible” instance position*/
layout (std430, binding=2) buffer CurrValidInstanceData
{
    InstanceProperties currValidInstanceProps[] ;
};

uniform int numMaxInstance;
uniform mat4 viewProjMat;
uniform int base;

void main() {
    int write_start[3] = {0, 155304, 155304 + 1010};
    uint idx = gl_GlobalInvocationID.x;

    // discarding invalid array-access
    if(idx >= numMaxInstance){ return; }
    vec4 prop = rawInstanceProps[idx].offset_texture;
    vec3 offset = vec3(prop.xyz);
    // translate the position to clip space
    vec4 clipSpaceV = viewProjMat * vec4(offset, 1.0) ;
    clipSpaceV = clipSpaceV / clipSpaceV.w ;
    // determine if it is culled
    bool notCulled = (clipSpaceV.x > -1.0) && (clipSpaceV.x < 1.0) && (clipSpaceV.y > -1.0) &&
    (clipSpaceV.y <= 1.0) && (clipSpaceV.z > -1.0) && (clipSpaceV.z < 1.0) ;
    // notCulled = true; // debug
    unsigned int type;
    uint UNIQUE_IDX;
    if(notCulled){
        type = 0;
        if (idx >= write_start[1]) type = 1;
        if (idx >= write_start[2]) type = 2;
        UNIQUE_IDX = atomicAdd(commands[type].instanceCount, 1);

        currValidInstanceProps[write_start[type] + UNIQUE_IDX] = rawInstanceProps[idx] ;
        return;
    }
}