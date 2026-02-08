#pragma once
#include "SpatialSample.h"

#include <glad/glad.h>
#include <Rendering_Framework/src/Rendering/Camera/Camera.h>
#include <string>

#define NUM_FOLIAGE_TYPE 3
#define NUM_DRAWS 159111

namespace INANOA {
	namespace SCENE {
		namespace EXPERIMENTAL {
			class Foliages
			{
			public:
				explicit Foliages(const Camera* camera);
				virtual ~Foliages();

				Foliages(const Foliages&) = delete;
				Foliages(const Foliages&&) = delete;
				Foliages& operator=(const Foliages&) = delete;

			public:
				void init(const Camera* camera);
				void update(const Camera* camera);
				void render();

			private:
				int m_numVertex[NUM_FOLIAGE_TYPE];
				int m_numIndex[NUM_FOLIAGE_TYPE];
                int m_numInstance[NUM_FOLIAGE_TYPE];
                int m_firstIndex[NUM_FOLIAGE_TYPE];
                int m_baseVertex[NUM_FOLIAGE_TYPE];
                int m_baseInstance[NUM_FOLIAGE_TYPE];
				const float m_height = 0.0f;
                const int m_num_texture = 3;
                const int m_img_width = 1024;
                const int m_img_height = 1024;
                const int m_img_channel = 4 ;
				GLuint m_vertexBufferHandle = 0u;
				float* m_vertexBuffer = nullptr;

                GLuint wholeDataBufferHandle;
                GLuint visibleDataBufferHandle;
                GLuint cmdBufferHandle;

				GLuint m_vaoHandle = 0u;
                GLuint textureArrayHandle;

				glm::mat4 m_modelMat;

                struct DrawElementsIndirectCommand{
                    unsigned int count ;
                    unsigned int instanceCount ;
                    unsigned int firstIndex ;
                    unsigned int baseVertex ;
                    unsigned int baseInstance ;
                };

                DrawElementsIndirectCommand drawCommands[NUM_FOLIAGE_TYPE];

                GLuint diffuse_tex[3];

                const char* obj_path[NUM_FOLIAGE_TYPE] = {"models/foliages/grassB.obj", "models/foliages/bush01_lod2.obj", "models/foliages/bush05_lod2.obj"};
                const char* sample_path[NUM_FOLIAGE_TYPE] = {
                    "models/spatialSamples/poissonPoints_155304s.ss2",
                    "models/spatialSamples/poissonPoints_1010s.ss2",
                    "models/spatialSamples/poissonPoints_2797s.ss2"
                };

                GLuint grass_vao;
                GLuint vertices_vbo;
                GLuint texcoords_vbo;
                GLuint normals_vbo;
                GLuint grass_ibo;
                
                std::vector<float> vertices;
                std::vector<float> texcoords;
                std::vector<float> normals;
                std::vector<unsigned int> indices;

                INANOA::SCENE::EXPERIMENTAL::SpatialSample *poi_sample[NUM_FOLIAGE_TYPE];
			};
		}
	}
}



