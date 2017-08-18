#include "postprocess.h"

PostProcess *PostProcess::m_pp = nullptr;

/*
class SSAORenderCallback : public IPostProcessingRenderCallback
{
public:
	SSAORenderCallback(irr::s32 materialTypeIn) : materialType(materialTypeIn) {}

	void OnPreRender(EffectHandler *effect)
	{
		video::IVideoDriver *driver = effect->getIrrlichtDevice()->getVideoDriver();
		viewProj = driver->getTransform(video::ETS_PROJECTION) *driver->getTransform(video::ETS_VIEW);
		effect->setPostProcessingEffectConstant(materialType, "mViewProj", viewProj.pointer(), 16);
	}

	void OnPostRender(EffectHandler *effect) {}

	core::matrix4 viewProj;
	s32 materialType;
};
*/

PostProcess::PostProcess(IrrlichtDevice *device, video::IVideoDriver *driver)
{
	m_pp = this;
	m_effect = new EffectHandler(device, driver->getScreenSize(), true, true, true);
}

PostProcess::~PostProcess()
{
	m_pp = nullptr;
}

void PostProcess::Init(IrrlichtDevice *device,
			video::IVideoDriver *driver,
			scene::ISceneManager *smgr)
{
	EffectHandler *effect = &(*(new PostProcess(device, driver))->m_effect);

	//effect->addPostProcessingEffectFromFile(core::stringc("client/shaders/postprocessBrightPass.glsl"));
	effect->addPostProcessingEffectFromFile(core::stringc("client/shaders/postprocess/BlurHP.glsl"));
	effect->addPostProcessingEffectFromFile(core::stringc("client/shaders/postprocess/BlurVP.glsl"));	
	effect->addPostProcessingEffectFromFile(core::stringc("client/shaders/postprocess/BloomP.glsl"));
/*
	scene::ISceneNode *m_map = smgr->getSceneNodeFromId(666);

	m_map->setMaterialFlag(video::EMF_LIGHTING, false);

	E_FILTER_TYPE filterType = (E_FILTER_TYPE)4;
	effect->addShadowToNode(m_map, filterType, ESM_BOTH);

	effect->addShadowLight(SShadowLight(512, v3f(-150, 500, -150), v3f(5, 0, 5), 
		video::SColor(255, 255, 255, 255), 20.0f, 60.0f, 30.0f * core::DEGTORAD));
*/
}
