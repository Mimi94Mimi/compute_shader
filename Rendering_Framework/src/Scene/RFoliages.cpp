#include "RFoliages.h"
#include "SpatialSample.h"
#include "../texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <Rendering_Framework/src/Rendering/ShaderParameterBindingPoint.h>
#include <iostream>

#include "../../externals/include/assimp/cimport.h"
#include "../../externals/include/assimp/scene.h"
#include "../../externals/include/assimp/postprocess.h"

#define NUM_FOLIAGE_TYPE 3
#define NUM_DRAWS 159111

using namespace std;

namespace INANOA
{
    namespace SCENE
    {
        namespace EXPERIMENTAL
        {
            Foliages::Foliages(const Camera *camera)
            {
                this->init(camera);
            }
            Foliages::~Foliages() {}

            void Foliages::init(const Camera *camera)
            {
                // load texture
                const int NUM_TEXTURE = this->m_num_texture;
                const int IMG_WIDTH = this->m_img_width;
                const int IMG_HEIGHT = this->m_img_height;
                const int IMG_CHANNEL = this->m_img_channel;
                glGenTextures(1, &textureArrayHandle);
                glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayHandle);

                glTexStorage3D(GL_TEXTURE_2D_ARRAY, 11, GL_RGBA8, IMG_WIDTH, IMG_HEIGHT, NUM_TEXTURE);

                glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

                //start for loop
                for(int type = 0; type < NUM_FOLIAGE_TYPE; type++)
                {
                    const aiScene *scene = aiImportFile(obj_path[type], aiProcessPreset_TargetRealtime_MaxQuality);
                    
                    aiMaterial *material = scene->mMaterials[0];
                    aiString texturePath;
                    if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS)
                    {
                        string texture_path = "textures/";
                        texture_path += texturePath.C_Str();
                        TextureData tex = loadImg(texture_path.c_str());
                        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, type, IMG_WIDTH, IMG_HEIGHT, 1, GL_RGBA,
                                        GL_UNSIGNED_BYTE, tex.data);

                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
                        glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    }

                    aiMesh *mesh = scene->mMeshes[0];
                    this->m_baseVertex[type] = vertices.size() / 3;
                    for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
                    {
                        this->vertices.push_back(mesh->mVertices[v].x);
                        this->vertices.push_back(mesh->mVertices[v].y);
                        this->vertices.push_back(mesh->mVertices[v].z);
                        this->texcoords.push_back(mesh->mTextureCoords[0][v][0]);
                        this->texcoords.push_back(mesh->mTextureCoords[0][v][1]);
                        this->texcoords.push_back(type);
                        this->normals.push_back(mesh->mNormals[v].x);
                        this->normals.push_back(mesh->mNormals[v].y);
                        this->normals.push_back(mesh->mNormals[v].z);
                    }
                    this->m_numVertex[type] = mesh->mNumVertices;

                    this->m_firstIndex[type] = indices.size();
                    for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
                    {
                        aiFace face = mesh->mFaces[f];
                        for (unsigned int j = 0; j < face.mNumIndices; j++)
                        {
                            this->indices.push_back(face.mIndices[j]);
                        }
                    }

                    this->m_numIndex[type] = mesh->mNumFaces * 3;
                }

                glGenVertexArrays(1, &grass_vao);
                glBindVertexArray(grass_vao);

                glGenBuffers(1, &vertices_vbo);
                glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(0);

                glGenBuffers(1, &texcoords_vbo);
                glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo);
                glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(float), texcoords.data(), GL_STATIC_DRAW);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(1);

                glGenBuffers(1, &normals_vbo);
                glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
                glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(2);

                glGenBuffers(1, &grass_ibo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grass_ibo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);


                // create a DYNAMIC VBO (only vertex is used)
                using namespace INANOA::SCENE::EXPERIMENTAL;
                for(int type = 0; type < NUM_FOLIAGE_TYPE; type++)
                {
                    poi_sample[type] = SpatialSample::importBinaryFile(sample_path[type]);
                    m_numInstance[type] = poi_sample[type]->numSample();
                }

                int offset_buffer_size = m_numInstance[0] + m_numInstance[1] + m_numInstance[2];
                float* offsets = new float[offset_buffer_size * 4]{0.0f};
                float* visibleOffsets = new float[offset_buffer_size * 4]{0.0f};

                int offset_start = 0;
                for(int type = 0; type < NUM_FOLIAGE_TYPE; type++)
                {
                    for(int idx = 0 ; idx < m_numInstance[type] ; idx++){
                        offsets[(offset_start + idx) * 4 + 0] = poi_sample[type]->position(idx)[0];
                        offsets[(offset_start + idx) * 4 + 1] = poi_sample[type]->position(idx)[1];
                        offsets[(offset_start + idx) * 4 + 2] = poi_sample[type]->position(idx)[2];
                        offsets[(offset_start + idx) * 4 + 3] = type;
                    }
                    m_baseInstance[type] = offset_start;
                    offset_start += m_numInstance[type];
                }


                for(int type = 0; type < NUM_FOLIAGE_TYPE; type++)
                {
                    drawCommands[type].count = m_numIndex[type];
                    drawCommands[type].instanceCount = m_numInstance[type];
                    drawCommands[type].firstIndex = m_firstIndex[type];
                    drawCommands[type].baseVertex = m_baseVertex[type];
                    drawCommands[type].baseInstance = m_baseInstance[type];
                }
                
                // prepare a SSBO for storing whole instance data
                glGenBuffers(1, &this->wholeDataBufferHandle);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->wholeDataBufferHandle);
                glBufferStorage(GL_SHADER_STORAGE_BUFFER, offset_buffer_size * 4 * sizeof(float), offsets,
                GL_MAP_READ_BIT) ;
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, this->wholeDataBufferHandle);

                // prepare a SSBO for storing visible instance data of current frame
                glGenBuffers(1, &this->visibleDataBufferHandle);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->visibleDataBufferHandle);
                glBufferStorage(GL_SHADER_STORAGE_BUFFER, offset_buffer_size * 4 * sizeof(float), nullptr,
                GL_MAP_READ_BIT) ;
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, this->visibleDataBufferHandle);

                //glBindBuffer(GL_ARRAY_BUFFER, this->wholeDataBufferHandle);
                glBindBuffer(GL_ARRAY_BUFFER, this->visibleDataBufferHandle);
                glVertexAttribPointer(3, 4, GL_FLOAT, false, 0, 0);
                glEnableVertexAttribArray(3);
                glVertexAttribDivisor(3, 1);

                // prepare a SSBO for storing rendering parameters
                glGenBuffers(1, &this->cmdBufferHandle) ;
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->cmdBufferHandle);
                glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(drawCommands),
                drawCommands, GL_MAP_READ_BIT);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, this->cmdBufferHandle);

                this->m_vaoHandle = grass_vao;
            }

            void Foliages::render()
            {   
                // bind vao
                glBindVertexArray(this->m_vaoHandle);
                //bind cmd buffer
                glBindBuffer(GL_DRAW_INDIRECT_BUFFER, this->cmdBufferHandle);
                // submit model matrix
                glUniformMatrix4fv(SHADER_PARAMETER_BINDING::MODEL_MAT_LOCATION, 1, false, glm::value_ptr(this->m_modelMat));
                // render
                const int indicesPtr = 0;
                glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, 3, 0);
            }

            void Foliages::update(const Camera *camera)
            {
                const glm::vec3 viewPos = camera->viewOrig();
                const glm::mat4 viewMat = camera->viewMatrix();

                glm::mat4 tMat = glm::translate(glm::vec3(viewPos.x, this->m_height, viewPos.z));
                glm::mat4 viewT = glm::transpose(viewMat);
                glm::vec3 forward = -1.0f * glm::vec3(viewT[2].x, 0.0, viewT[2].z);
                glm::vec3 y(0.0, 1.0, 0.0);
                glm::vec3 x = glm::normalize(glm::cross(y, forward));

                glm::mat4 rMat;
                rMat[0] = glm::vec4(x, 0.0);
                rMat[1] = glm::vec4(y, 0.0);
                rMat[2] = glm::vec4(forward, 0.0);
                rMat[3] = glm::vec4(0.0, 0.0, 0.0, 1.0);

                this->m_modelMat = glm::mat4(1.0);
            }
        }
    }
}