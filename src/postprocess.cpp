

#include "drawscene.h"
#include "postprocess.h"

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

	inline void SetThreshold(float threshold) { Threshold = threshold; }
	inline void SetTime(float time) { Time = time; }
	inline void SetBlendFactor(float factor) { BlendFactor = factor; }

private:
	irr::u32 NumTextures;
	irr::core::vector2df PixelSize;
	float Threshold;
	float Time;
	float BlendFactor;

	irr::u32 RenderID;
	irr::u32 TexIDs[2];
	irr::u32 PixelSizeID;
};

IShaderDefaultPostProcessCallback::IShaderDefaultPostProcessCallback() : Threshold(0.5f), Time(0.0f), BlendFactor(1.0f)
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

	services->setPixelShaderConstant("BlendFactor", (irr::f32*)&BlendFactor, 1);
	services->setPixelShaderConstant("Threshold", (irr::f32*)&Threshold, 1);
	services->setPixelShaderConstant("PixelSizeX"/*PixelSizeID*/, (irr::f32*)&PixelSize.X, 1);
	services->setPixelShaderConstant("PixelSizeY", (irr::f32*)&PixelSize.Y, 1);
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

// 
PostProcess PostProcess::postProcess;
std::vector<PostProcess::ShaderCacheEntry> PostProcess::shaderCache;

void PostProcess::InitShaders()
{
	postProcess.callbackPP = new IShaderDefaultPostProcessCallback();

	//IShaderSource *shdrsrc = client.getShaderSource();
	const char* shaders[] = { 
		"iseered2.frag", 
		"blur_v.frag", 
		"blur_h.frag", 
		"bloom_prepass.frag", 
		"add2.frag", 
		"albedo.frag", 
		"blur_h_add.frag", 
		"fxaa.frag",
		NULL
	};

	int i = 0;
	while (shaders[i])
	{
		std::string vertex_path = getShaderPath("postprocess", "quad.vert");
		std::string pixel_path = getShaderPath("postprocess", shaders[i]);
		std::string vertex_program = readFile(vertex_path);
		std::string pixel_program = readFile(pixel_path);

		s32 shadermat = postProcess.driver->getGPUProgrammingServices()->addHighLevelShaderMaterial(
			vertex_program.c_str(), "main", irr::video::EVST_VS_2_0,
			pixel_program.c_str(), "main", irr::video::EPST_PS_2_0
		, postProcess.callbackPP);

		std::string name(shaders[i]);
		size_t pos = name.find(".frag");
		name.replace(pos, 5, "");
		PostProcess::ShaderCacheEntry entry;
		entry.material = shadermat;
		entry.name = name;
		postProcess.shaderCache.push_back(entry);
		//driver->getMaterialRenderer(shadermat)->grab();
		i++;
	}

	PostProcess::Effect& fail = postProcess.addNewEffect("fail");
	fail.addShader("iseered2");
	PostProcess::Effect& copy = postProcess.addNewEffect("copy");
	copy.addShader("albedo");
	// bloom
	PostProcess::Effect& bloom = postProcess.addNewEffect("bloom");
	bloom.addShader("bloom_prepass");
	bloom.addShader("blur_v");
	bloom.addShader("blur_h");
	bloom.addShader("add2");
	// blur
	PostProcess::Effect& blur = postProcess.addNewEffect("blur");
	blur.addShader("blur_v");
	blur.addShader("blur_h");
	blur.addShader("albedo");
}

void PostProcess::Init(const v2u32 &screensize, Client &client)
{
	if (!postProcess.imageScene)
	{
		init_texture(postProcess.driver, screensize, &postProcess.imageScene, "pp_source");
		// TODO: use a viewport or dynamically recreate targets
		init_texture(postProcess.driver, screensize/2, &postProcess.imagePP[0], "pp_img1");
		init_texture(postProcess.driver, screensize/2, &postProcess.imagePP[1], "pp_img2");
		
		irr::scene::SMeshBuffer* bufferPP = new irr::scene::SMeshBuffer;
		postProcess.bufferPP = bufferPP;
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

		postProcess.materialPP.ZBuffer = false;
		postProcess.materialPP.ZWriteEnable = false;
		postProcess.materialPP.TextureLayer[0].TextureWrapU = irr::video::ETC_CLAMP_TO_EDGE;
		postProcess.materialPP.TextureLayer[0].TextureWrapV = irr::video::ETC_CLAMP_TO_EDGE;
		
		InitShaders();
	}
}

void PostProcess::Begin(video::SColor color)
{
	if (postProcess.imageScene) {
		postProcess.driver->setRenderTarget(postProcess.imageScene, true, true, color);
	}
}

void PostProcess::ApplyEffect(const char* name)
{
	if (!postProcess.imageScene)
		return;

	int currentEffect = 0;
	for (size_t fx = 0; fx < postProcess.effectDB.size(); ++fx) {
		if (postProcess.effectDB[fx].name == name) {
			currentEffect = fx;
			break;
		}
	}

	PostProcess::Effect& effect = postProcess.effectDB[currentEffect];
	int shaderCount = effect.shaders.size();
	for (size_t i = 0; i < shaderCount; ++i) {
		postProcess.effectChain.push_back(effect.shaders[i]);
	}
}

void PostProcess::End()
{
	if (!postProcess.imageScene)
		return;

	// maybe not required because post process shaders don't require matrices
	postProcess.driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
	postProcess.driver->setTransform(video::ETS_VIEW, core::IdentityMatrix);
	postProcess.driver->setTransform(video::ETS_PROJECTION, core::IdentityMatrix);

	video::SMaterial oldmaterial = postProcess.driver->getMaterial2D();

	std::vector<s32>& shaders = postProcess.effectChain;
	u32 shaderCount = shaders.size();
	s32 finalShader = shaders[shaderCount - 1];

	if (shaderCount > 1)
	{
		int currentPP = 0;
		int shadermat = shaders[0];
		postProcess.materialPP.MaterialType = (irr::video::E_MATERIAL_TYPE)shadermat;
		postProcess.driver->setRenderTarget(postProcess.imagePP[currentPP], true, false);
		postProcess.materialPP.setTexture(0, postProcess.imageScene);
		postProcess.driver->setMaterial(postProcess.materialPP);
		postProcess.driver->drawMeshBuffer(postProcess.bufferPP);
		
		for (u32 i = 1; i < shaderCount; ++i) {
			int shadermat = shaders[i];
			postProcess.materialPP.MaterialType = (irr::video::E_MATERIAL_TYPE)shadermat;
			video::ITexture* target = (i == (shaderCount - 1)) ? NULL : postProcess.imagePP[currentPP ^ 1];
			postProcess.driver->setRenderTarget(target, true, false);
			postProcess.materialPP.setTexture(0, postProcess.imagePP[currentPP]);
			// for now force texture 1 to be the original scene
			postProcess.materialPP.setTexture(1, postProcess.imageScene);
			postProcess.driver->setMaterial(postProcess.materialPP);
			postProcess.driver->drawMeshBuffer(postProcess.bufferPP);

			currentPP ^= 1;
		}
	}
	else
	{
		int shadermat = finalShader;
		postProcess.materialPP.MaterialType = (irr::video::E_MATERIAL_TYPE)shadermat;

		postProcess.driver->setRenderTarget(0, false, false);
		postProcess.materialPP.setTexture(0, postProcess.imageScene);
		postProcess.driver->setMaterial(postProcess.materialPP);
		postProcess.driver->drawMeshBuffer(postProcess.bufferPP);
	}

	postProcess.driver->setMaterial(oldmaterial);

	postProcess.effectChain.clear();
}

void PostProcess::Clean()
{
	if (postProcess.imageScene) {
		//driver->setRenderTarget(0);
		delete postProcess.bufferPP;
		postProcess.bufferPP = NULL;
		postProcess.driver->removeTexture(postProcess.imagePP[1]);
		postProcess.driver->removeTexture(postProcess.imagePP[0]);
		postProcess.driver->removeTexture(postProcess.imageScene);
		postProcess.imagePP[1] = NULL;
		postProcess.imagePP[0] = NULL;
		postProcess.imageScene = NULL;
	}
}

void PostProcess::SetThreshold(float threshold) { postProcess.callbackPP->SetThreshold(threshold); }
void PostProcess::SetBlendingFactor(float factor) { postProcess.callbackPP->SetBlendFactor(factor); }
