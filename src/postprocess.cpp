

#include "drawscene.h"

static video::ITexture *imageScene = NULL;
static video::ITexture *imagePP[2];
static irr::video::SMaterial materialPP;
irr::scene::SMeshBuffer* bufferPP;

irr::video::IShaderConstantSetCallBack* callbackPP;

std::vector<s32> shaderCache;
struct Effect {
	std::string name;
	std::vector<s32> shaders;
};
std::vector<Effect> effects;

// duplicated ---
std::string readFile(const std::string &path)
{
	std::ifstream is(path.c_str(), std::ios::binary);
	if (!is.is_open())
		return "";
	std::ostringstream tmp_os;
	tmp_os << is.rdbuf();
	return tmp_os.str();
}

void init_texture(video::IVideoDriver* driver, const v2u32& screensize,
		video::ITexture** texture, const char* name);
// ---

class IShaderDefaultPostProcessCallback : public irr::video::IShaderConstantSetCallBack
{
public:
	IShaderDefaultPostProcessCallback();

	virtual void OnSetConstants(irr::video::IMaterialRendererServices* services, irr::s32 userData);

	virtual void OnSetMaterial(const irr::video::SMaterial &material);

private:
	bool HaveIDs;
	irr::u32 NumTextures;
	irr::core::vector2df PixelSize;

	irr::u32 RenderID;
	irr::u32 TexIDs[3];
	irr::u32 PixelSizeID;
};

IShaderDefaultPostProcessCallback::IShaderDefaultPostProcessCallback()
	:HaveIDs(false)
{

}

void IShaderDefaultPostProcessCallback::OnSetConstants(video::IMaterialRendererServices* services, s32 userData)
{
	/*if (!HaveIDs)
	{
		RenderID = services->getPixelShaderConstantID("Render");

		for (u32 i = 0; i < 1; i++)
			TexIDs[i] = services->getPixelShaderConstantID((irr::core::stringc("Tex") + irr::core::stringc(i)).c_str());

		PixelSizeID = services->getPixelShaderConstantID("PixelSize");

		HaveIDs = true;
	}*/

	irr::s32 render = 0;
	services->setPixelShaderConstant("Render"/*RenderID*/, &render, 1);

	for (u32 i = 0; i < NumTextures; i++)
	{
		irr::s32 tex = i + 1;
		irr::core::stringc TexID = (irr::core::stringc("Tex") + irr::core::stringc(i));
		services->setPixelShaderConstant(TexID.c_str()/*TexIDs[i]*/, &tex, 1);
	}

	//some more useful stuff for post processing shaders
	services->setPixelShaderConstant("PixelSizeX"/*PixelSizeID*/, (irr::f32*)&PixelSize.X, 1);
	services->setPixelShaderConstant("PixelSizeY"/*PixelSizeID*/, (irr::f32*)&PixelSize.Y, 1);
}

void IShaderDefaultPostProcessCallback::OnSetMaterial(const video::SMaterial &material)
{
	irr::core::dimension2du tSize = material.getTexture(0)->getSize();
	PixelSize = irr::core::vector2df(1.0 / tSize.Width, 1.0 / tSize.Height);

	NumTextures = 0;
	for (u32 i = 1; i <= 4; i++)
	{
		if (material.getTexture(i))
			NumTextures++;
		else
			break;
	}
}

void init_postprocess_shaders(video::IVideoDriver *driver)
{
	callbackPP = new IShaderDefaultPostProcessCallback();

	//IShaderSource *shdrsrc = client.getShaderSource();
	const char* shaders[] = { "iseered2.frag", "blur_v.frag", "blur_h.frag", "bloom_prepass.frag", "add2.frag", "albedo.frag", "blur_h_add.frag", "fxaa.frag" };
	int shaderCount = 6;
	for (int i = 0; i < shaderCount; ++i) {
		std::string vertex_path = getShaderPath("postprocess", "quad.vert");
		std::string pixel_path = getShaderPath("postprocess", shaders[i]);
		std::string vertex_program = readFile(vertex_path);
		std::string pixel_program = readFile(pixel_path);
		s32 shadermat = driver->getGPUProgrammingServices()->addHighLevelShaderMaterial(
			vertex_program.c_str(), "main", irr::video::EVST_VS_2_0,
			pixel_program.c_str(), "main", irr::video::EPST_PS_2_0
		,callbackPP);
		shaderCache.push_back(shadermat);
		//driver->getMaterialRenderer(shadermat)->grab();
	}


	Effect effect;
	effect.name = "fail";
	effect.shaders.push_back(shaderCache[0]);
	effects.push_back(effect);
	effect.name = "copy";
	effect.shaders.clear();
	effect.shaders.push_back(shaderCache[5]);
	effects.push_back(effect);
	// bloom
	effect.shaders.clear();
	effect.name = "bloom";
	effect.shaders.push_back(shaderCache[3]);
	effect.shaders.push_back(shaderCache[1]);
	effect.shaders.push_back(shaderCache[2]);
	effect.shaders.push_back(shaderCache[4]);
	effects.push_back(effect);
	// blur
	effect.shaders.clear();
	effect.name = "blur";
	effect.shaders.push_back(shaderCache[1]);
	effect.shaders.push_back(shaderCache[2]);
	effect.shaders.push_back(shaderCache[5]);
	effects.push_back(effect);
}

void init_postprocess(video::IVideoDriver *driver, const v2u32 &screensize, Client &client)
{
	if (!imageScene) {
		init_texture(driver, screensize, &imageScene, "pp_source");
		// TODO: use a viewport or dynamically recreate targets
		init_texture(driver, screensize/2, &imagePP[0], "pp_img1");
		init_texture(driver, screensize/2, &imagePP[1], "pp_img2");
		bufferPP = new irr::scene::SMeshBuffer;
		bufferPP->Vertices.push_back(irr::video::S3DVertex(
			irr::core::vector3df(-1.0, 1.0, 0.0),
			irr::core::vector3df(0.0, 0.0, 1.0), irr::video::SColor(255, 255, 255, 255),
			irr::core::vector2df(0.0, 0.0)));
		bufferPP->Vertices.push_back(irr::video::S3DVertex(
			irr::core::vector3df(1.0, 1.0, 0.0),
			irr::core::vector3df(0.0, 0.0, 1.0), irr::video::SColor(255, 255, 255, 255),
			irr::core::vector2df(1.0, 0.0)));
		bufferPP->Vertices.push_back(irr::video::S3DVertex(
			irr::core::vector3df(1.0, -1.0, 0.0),
			irr::core::vector3df(0.0, 0.0, 1.0), irr::video::SColor(255, 255, 255, 255),
			irr::core::vector2df(1.0, 1.0)));
		bufferPP->Vertices.push_back(irr::video::S3DVertex(
			irr::core::vector3df(-1.0, -1.0, 0.0),
			irr::core::vector3df(0.0, 0.0, 1.0), irr::video::SColor(255, 255, 255, 255),
			irr::core::vector2df(0.0, 1.0)));

		bufferPP->Indices.push_back(0);
		bufferPP->Indices.push_back(1);
		bufferPP->Indices.push_back(2);
		bufferPP->Indices.push_back(3);
		bufferPP->Indices.push_back(0);
		bufferPP->Indices.push_back(2);

		materialPP.ZBuffer = false;
		materialPP.ZWriteEnable = false;
		materialPP.TextureLayer[0].TextureWrapU = irr::video::ETC_CLAMP_TO_EDGE;
		materialPP.TextureLayer[0].TextureWrapV = irr::video::ETC_CLAMP_TO_EDGE;
	}

	init_postprocess_shaders(driver);
}

void begin_postprocess(video::IVideoDriver *driver, video::SColor color)
{
	if (imageScene) {
		driver->setRenderTarget(imageScene, true, true, color);
	}
}

void apply_effect(video::IVideoDriver *driver, const char* name)
{
	if (!imageScene)
		return;

	int currentEffect = 0;
	for (int fx = 0; fx < effects.size(); ++fx) {
		if (effects[fx].name == name) {
			currentEffect = fx;
			break;
		}
	}

	Effect& effect = effects[currentEffect];
	int shaderCount = effect.shaders.size();

	driver->setTransform(video::ETS_VIEW, core::IdentityMatrix);
	driver->setTransform(video::ETS_PROJECTION, core::IdentityMatrix);
	video::SMaterial oldmaterial = driver->getMaterial2D();

	s32 finalShader = effect.shaders[shaderCount - 1];

	if (shaderCount > 1)
	{
		int currentPP = 0;
		int shadermat = effect.shaders[0];
		materialPP.MaterialType = (irr::video::E_MATERIAL_TYPE)shadermat;
		driver->setRenderTarget(imagePP[currentPP], true, false);		
		materialPP.setTexture(0, imageScene);
		driver->setMaterial(materialPP);
		driver->drawMeshBuffer(bufferPP);
		
		for (int i = 1; i < shaderCount; ++i) {
			int shadermat = effect.shaders[i];
			materialPP.MaterialType = (irr::video::E_MATERIAL_TYPE)shadermat;
			video::ITexture* target = (i == (shaderCount - 1)) ? NULL : imagePP[currentPP ^ 1];
			driver->setRenderTarget(target, true, false);
			materialPP.setTexture(0, imagePP[currentPP]);
			// for now force texture 1 to be the original scene
			materialPP.setTexture(1, imageScene);
			driver->setMaterial(materialPP);
			driver->drawMeshBuffer(bufferPP);

			currentPP ^= 1;
		}
	}
	else
	{
		int shadermat = finalShader;
		materialPP.MaterialType = (irr::video::E_MATERIAL_TYPE)shadermat;

		driver->setRenderTarget(0, false, false);
		materialPP.setTexture(0, imageScene);
		driver->setMaterial(materialPP);
		driver->drawMeshBuffer(bufferPP);
	}
	driver->setMaterial(oldmaterial);
}

void clean_postprocess(video::IVideoDriver *driver)
{
	if (imageScene) {
		//driver->setRenderTarget(0);
		delete bufferPP;
		bufferPP = NULL;
		driver->removeTexture(imagePP[1]);
		driver->removeTexture(imagePP[0]);
		driver->removeTexture(imageScene);
		imagePP[1] = NULL;
		imagePP[0] = NULL;
		imageScene = NULL;
	}
}
