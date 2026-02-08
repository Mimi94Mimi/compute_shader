#include "RendererBase.h"
#include "ShaderParameterBindingPoint.h"

#include <glm/gtc/type_ptr.hpp>

namespace INANOA {
	namespace OPENGL {
		RendererBase::RendererBase() {
			this->m_viewMat = glm::mat4x4(1.0f);
			this->m_projMat = glm::mat4x4(1.0f);
			this->m_viewPosition = glm::vec4(0.0f);
		}
		RendererBase::~RendererBase() {}

		bool RendererBase::init(const std::string& vsResource, const std::string& fsResource, const std::string& resetcsResource, const std::string& cullingcsResource, const int width, const int height) {
            this->m_resetCSProgram = ShaderProgram::createShaderProgramForComputeShader(resetcsResource);
			if (this->m_resetCSProgram == nullptr) {
				return false;
			}

            this->m_cullingCSProgram = ShaderProgram::createShaderProgramForComputeShader(cullingcsResource);
			if (this->m_cullingCSProgram == nullptr) {
				return false;
			}

			this->m_shaderProgram = ShaderProgram::createShaderProgram(vsResource, fsResource);
			if (this->m_shaderProgram == nullptr) {
				return false;
			}

			this->m_shaderProgram->useProgram();

			// API setting
			glEnable(GL_DEPTH_TEST);
			glLineWidth(2.0f);

			return true;
		}		

		void RendererBase::setCamera(const glm::mat4& projMat, const glm::mat4& viewMat, const glm::vec3& viewOrg) {
			this->m_projMat = projMat;
			this->m_viewMat = viewMat;
			this->m_viewPosition = glm::vec4(viewOrg, 1.0f);

			glUniformMatrix4fv(SHADER_PARAMETER_BINDING::VIEW_MAT_LOCATION, 1, false, glm::value_ptr(this->m_viewMat));
			glUniformMatrix4fv(SHADER_PARAMETER_BINDING::PROJ_MAT_LOCATION, 1, false, glm::value_ptr(this->m_projMat));
		}

		void RendererBase::resize(const int w, const int h) {
			this->m_frameWidth = w;
			this->m_frameHeight = h;
		}

		void RendererBase::clearRenderTarget() {
			static float DEPTH[1] = { 1.0f };
			static float COLOR[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

			glClearBufferfv(GL_COLOR, 0, COLOR);
			glClearBufferfv(GL_DEPTH, 0, DEPTH);
		}

        void RendererBase::useProgram(const char* program){
            if(program == "shader_program"){
                this->m_shaderProgram->useProgram();
            }
            if(program == "reset_cs_program"){
                this->m_resetCSProgram->useProgram();
            }
            if(program == "culling_cs_program"){
                this->m_cullingCSProgram->useProgram();
            }
        }

        GLuint RendererBase::getProgramID(const char* program){
            if(program == "shader_program"){
                return this->m_shaderProgram->programId();
            }
            if(program == "reset_cs_program"){
                return this->m_resetCSProgram->programId();
            }
            if(program == "culling_cs_program"){
                return this->m_cullingCSProgram->programId();
            }
        }
	}	
}
