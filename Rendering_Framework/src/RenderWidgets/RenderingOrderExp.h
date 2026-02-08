#pragma once

#include <vector>

#include <Rendering_Framework/src/Rendering/RendererBase.h>
#include <Rendering_Framework/src/Scene/RViewFrustum.h>
#include <Rendering_Framework/src/Scene/RHorizonGround.h>
#include <Rendering_Framework/src/Scene/RFoliages.h>

namespace INANOA {
	class RenderingOrderExp
	{
	public:
		RenderingOrderExp();
		virtual ~RenderingOrderExp();

	public:
		bool init(const int w, const int h) ;
		void resize(const int w, const int h) ;
		void update() ;
		void render();
        void onKeyDown(char key);
        void onMouseDragLeft(int xpos, int ypos, bool mouse_down);
        void onMouseDragRight(int xpos, int ypos, bool mouse_down);

	private:
		SCENE::RViewFrustum* m_viewFrustum = nullptr;
		SCENE::EXPERIMENTAL::HorizonGround* m_horizontalGround = nullptr;
        SCENE::EXPERIMENTAL::Foliages* m_foliages = nullptr; 

		Camera* m_playerCamera = nullptr;
		Camera* m_godCamera = nullptr;

		glm::vec3 m_cameraForwardMagnitude;
		float m_cameraForwardSpeed;

		int m_frameWidth;
		int m_frameHeight;

		OPENGL::RendererBase* m_renderer = nullptr;

        GLuint m_numInstanceLoc;
        GLuint m_viewprojMatLoc;

        struct onMouseDragParams{
            int lastX;
            int lastY;
            float yaw;
            float pitch;
        };

        onMouseDragParams m_onMouseDragParams;
        int lastX;
        float rotY;
	};

}


