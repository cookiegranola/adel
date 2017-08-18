#pragma once

#include "client/renderingengine.h"
#include "client/xeffects/src/XEffects.h"

class PostProcess {
public:
	PostProcess(IrrlichtDevice *device, video::IVideoDriver *driver);
	~PostProcess();

	static void Init(
		IrrlichtDevice *device,
		video::IVideoDriver *driver,
		scene::ISceneManager *smgr);

	static EffectHandler *getEffect()
	{
		return m_pp->m_effect;
	}

	static scene::ISceneNode *getMap()
	{
		return m_pp->m_map;
	}

private:
	scene::ISceneNode *m_map = nullptr;
	EffectHandler *m_effect = nullptr;
	static PostProcess *m_pp;
};
