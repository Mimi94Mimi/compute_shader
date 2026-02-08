#include "RenderingOrderExp.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#define NUM_DRAWS 159111

namespace INANOA {	

	// ===========================================================
	RenderingOrderExp::RenderingOrderExp(){
		this->m_cameraForwardSpeed = 0.25f;
		this->m_cameraForwardMagnitude = glm::vec3(0.0f, 0.0f, 0.0f);
		this->m_frameWidth = 64;
		this->m_frameHeight = 64;
	}
	RenderingOrderExp::~RenderingOrderExp(){}

	bool RenderingOrderExp::init(const int w, const int h) {
		INANOA::OPENGL::RendererBase* renderer = new INANOA::OPENGL::RendererBase();
		const std::string vsFile = "src\\shader\\vertexShader_ogl_450.glsl";
		const std::string fsFile = "src\\shader\\fragmentShader_ogl_450.glsl";
        const std::string resetcsFile = "src\\shader\\resetCS.glsl";
        const std::string cullingcsFile = "src\\shader\\cullingCS.glsl";
		if (renderer->init(vsFile, fsFile, resetcsFile, cullingcsFile, w, h) == false) {
			return false;
		}

		this->m_renderer = renderer;

		this->m_godCamera = new Camera(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 5.0f, 60.0f, 0.1f, 512.0f);
		this->m_godCamera->resize(w, h);

		this->m_godCamera->setViewOrg(glm::vec3(0.0f, 55.0f, 50.0f));
		this->m_godCamera->setLookCenter(glm::vec3(0.0f, 32.0f, -12.0f));
		this->m_godCamera->setDistance(70.0f);
		this->m_godCamera->update();

		this->m_playerCamera = new Camera(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 9.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 10.0, 45.0f, 1.0f, 150.0f);
		this->m_playerCamera->resize(w, h);
		this->m_playerCamera->update();

		m_renderer->setCamera(
			this->m_godCamera->projMatrix(),
			this->m_godCamera->viewMatrix(),
			this->m_godCamera->viewOrig()
		);

		// view frustum and horizontal ground
		{
			this->m_viewFrustum = new SCENE::RViewFrustum(1, nullptr);
			this->m_viewFrustum->resize(this->m_playerCamera);

			this->m_horizontalGround = new SCENE::EXPERIMENTAL::HorizonGround(2, nullptr);
			this->m_horizontalGround->resize(this->m_playerCamera);
			}

        // foliages
        this->m_foliages = new SCENE::EXPERIMENTAL::Foliages(nullptr);

		this->resize(w, h);

        m_onMouseDragParams.pitch = 0;
        m_onMouseDragParams.yaw = -90.0f;
        lastX = 0;
        rotY = -90;
		return true;
	}
	void RenderingOrderExp::resize(const int w, const int h) {
		const int HW = w * 0.5;

		this->m_playerCamera->resize(HW, h);
		this->m_godCamera->resize(HW, h);
		m_renderer->resize(w, h);
		this->m_frameWidth = w;
		this->m_frameHeight = h;

		this->m_viewFrustum->resize(this->m_playerCamera);
		this->m_horizontalGround->resize(this->m_playerCamera);
	}
	void RenderingOrderExp::update() {		
		// camera update (god)
		m_godCamera->update();

		// camera update (player)
		this->m_playerCamera->forward(this->m_cameraForwardMagnitude, true);
		this->m_playerCamera->update();

		// lock to view space
		this->m_viewFrustum->update(this->m_playerCamera);
		this->m_horizontalGround->update(this->m_playerCamera);
        this->m_foliages->update(this->m_playerCamera);
	}
	void RenderingOrderExp::render() {		
		this->m_renderer->clearRenderTarget();
		const int HW = this->m_frameWidth * 0.5;

        this->m_numInstanceLoc = glGetUniformLocation(this->m_renderer->getProgramID("culling_cs_program"), "numMaxInstance");
        this->m_viewprojMatLoc = glGetUniformLocation(this->m_renderer->getProgramID("culling_cs_program"), "viewProjMat");
        glm::mat4 viewprojMat = m_playerCamera->projMatrix() * m_playerCamera->viewMatrix();

		// =====================================================
		// god view
		this->m_renderer->setCamera(
			m_godCamera->projMatrix(),
			m_godCamera->viewMatrix(),
			m_godCamera->viewOrig()
		);
        //this->m_foliages->update(this->m_godCamera);

		this->m_renderer->setViewport(0, 0, HW, this->m_frameHeight);
		this->m_renderer->setShadingModel(OPENGL::ShadingModelType::UNLIT);
		this->m_viewFrustum->render();
		this->m_renderer->setShadingModel(OPENGL::ShadingModelType::PROCEDURAL_GRID);
		this->m_horizontalGround->render();

        this->m_renderer->useProgram("reset_cs_program");
        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        this->m_renderer->useProgram("culling_cs_program");
        glUniform1i(this->m_numInstanceLoc, NUM_DRAWS);
        glUniformMatrix4fv(this->m_viewprojMatLoc, 1, false, glm::value_ptr(viewprojMat));
        glUniform1ui(glGetUniformLocation(this->m_renderer->getProgramID("culling_cs_program"), "base"), 0);
        // start GPU process
        glDispatchCompute((NUM_DRAWS / 1024) + 1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


        this->m_renderer->useProgram("shader_program");
        this->m_renderer->setShadingModel(OPENGL::ShadingModelType::FOLIAGES);
		this->m_foliages->render();

		// =====================================================
		// player view
		this->m_renderer->clearDepth();
		this->m_renderer->setCamera(
			this->m_playerCamera->projMatrix(),
			this->m_playerCamera->viewMatrix(),
			this->m_playerCamera->viewOrig()
		);

		this->m_renderer->setViewport(HW, 0, HW, this->m_frameHeight);
		this->m_renderer->setShadingModel(OPENGL::ShadingModelType::PROCEDURAL_GRID);
		this->m_horizontalGround->render();	

        // this->m_renderer->useProgram("shader_program");
        this->m_renderer->setShadingModel(OPENGL::ShadingModelType::FOLIAGES);
		this->m_foliages->render();
	}
    void RenderingOrderExp::onKeyDown(char key){
        glm::vec3 center = this->m_playerCamera->lookCenter();
        glm::vec3 up = glm::normalize(this->m_playerCamera->upVector());
        glm::vec3 front = glm::normalize(center - this->m_playerCamera->viewOrig());
        glm::vec3 right = glm::normalize(cross(front, up));
        glm::vec3 god_up;
        switch (key)
        {
        case 'W':
        case 'w':
            this->m_playerCamera->translateLookCenterAndViewOrg(front);
            break;
        case 'S':
        case 's':
            this->m_playerCamera->translateLookCenterAndViewOrg(-front);
            break;
        case 'A':
        case 'a':
            this->m_playerCamera->translateLookCenterAndViewOrg(-right);
            break;
        case 'D':
        case 'd':
            this->m_playerCamera->translateLookCenterAndViewOrg(right);
            break;
        case 'Z':
        case 'z':
            god_up = this->m_godCamera->upVector();
            this->m_godCamera->translateLookCenterAndViewOrg(god_up);
            break;
        case 'X':
        case 'x':
            god_up = this->m_godCamera->upVector();
            this->m_godCamera->translateLookCenterAndViewOrg(-god_up);
            break;
        default:
            break;
        }
    }
    void RenderingOrderExp::onMouseDragLeft(int xpos, int ypos, bool mouse_down){
        // reference: https://learnopengl.com/Getting-started/Camera
        if (mouse_down)
        {
            this->m_onMouseDragParams.lastX = xpos;
            this->m_onMouseDragParams.lastY = ypos;
        }
    
        float xoffset = xpos - m_onMouseDragParams.lastX;
        float yoffset = m_onMouseDragParams.lastY - ypos; 
        m_onMouseDragParams.lastX = xpos;
        m_onMouseDragParams.lastY = ypos;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        m_onMouseDragParams.yaw   -= xoffset;
        m_onMouseDragParams.pitch -= yoffset;

        if(m_onMouseDragParams.pitch > 89.0f)
            m_onMouseDragParams.pitch = 89.0f;
        if(m_onMouseDragParams.pitch < -89.0f)
            m_onMouseDragParams.pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(m_onMouseDragParams.yaw)) * cos(glm::radians(m_onMouseDragParams.pitch));
        direction.y = sin(glm::radians(m_onMouseDragParams.pitch));
        direction.z = sin(glm::radians(m_onMouseDragParams.yaw)) * cos(glm::radians(m_onMouseDragParams.pitch));
        direction = glm::normalize(direction);
        this->m_godCamera->setLookCenter(this->m_godCamera->viewOrig() + direction * this->m_godCamera->distance());
    }
    void RenderingOrderExp::onMouseDragRight(int xpos, int ypos, bool mouse_down){
        if (mouse_down)
        {
            lastX = xpos;
        }
    
        float xoffset = xpos - lastX; 
        m_onMouseDragParams.lastX = xpos;

        float sensitivity = 0.001f;
        float d_rad = xoffset * sensitivity;
        

        this->m_playerCamera->rotateLookCenterAccordingToViewOrg(d_rad);
        lastX = xpos;
    }
}
