#pragma once

#include "ball/Ball.h"
#include "terrain/Terrain.h"
#include "util/PerspectiveCameraController.h"
#include "lights/Lights.h"

#include <GLCore.h>
#include <GLCoreUtils.h>

class AppLayer : public GLCore::Layer {
public:
	AppLayer();
	virtual ~AppLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	virtual void update(GLCore::Timestep ts) override;
	virtual void imGuiRender() override;
private:
	PerspectiveCameraController cameraController;
	lights::LightScene lightScene;
	Ball ball;
	Terrain terrain;
};
