

#ifndef POSTPROCESS_H_
#define POSTPROCESS_H_

#include "irrlichttypes_extrabloated.h"

struct PostProcess
{
	struct ShaderCacheEntry {
		std::string name;
		s32 material;
	};
	static std::vector<ShaderCacheEntry> shaderCache;

	s32 getShader(const char* name) {
		for (size_t i = 0; i < PostProcess::shaderCache.size(); i++) {
			const ShaderCacheEntry& entry = PostProcess::shaderCache[i];
			if (entry.name == name) {
				return entry.material;
			}
		}
		return -1;
	}

	struct Effect 
	{
		std::string name;
		std::vector<s32> shaders;

		bool addShader(const char* name) {
			for (size_t i = 0; i < PostProcess::shaderCache.size(); i++) {
				const ShaderCacheEntry& entry = PostProcess::shaderCache[i];
				if (entry.name == name) {
					shaders.push_back(entry.material);
					return true;
				}
			}
			return false;
		}
	};
	std::vector<Effect> effectDB;

	Effect& addNewEffect(const char* name) { 
		Effect effect = { name };
		size_t index = effectDB.size();
		effectDB.push_back(effect);
		return effectDB[index];
	}

	irr::video::IVideoDriver *driver;
	v2u32 screenSize;
	irr::scene::ICameraSceneNode* lightCamera;

#if 0
	irr::video::ITexture *imageScene;
	irr::video::ITexture *imagePP[2];
#else
	class COpenGLRenderTarget *imageScene;
	class COpenGLRenderTarget *imagePP[2];
	class COpenGLRenderTarget *lightScene;
	class COpenGLRenderTarget *tempScene;
#endif

	static irr::video::SMaterial materialPP;
	irr::scene::SMeshBuffer* bufferPP;

	class IShaderDefaultPostProcessCallback* callbackPP;

	std::vector<s32> effectChain;
	
	PostProcess() : driver(NULL), imageScene(NULL), bufferPP(NULL), callbackPP(NULL) {
		imagePP[0] = NULL; imagePP[1] = NULL;
	}

	static s32 shadowShader;

	static PostProcess postProcess;
	
	static void SetSunPosition(float x, float y);
	static void SetThreshold(float threshold);
	static void SetBlendingFactor(float factor);
	static void SetExposure(float factor);

	static void SetDriver(irr::video::IVideoDriver *irr_driver) { postProcess.driver = irr_driver; }
	static void InitShaders(/*todo: pass list*/);
	static void Init(const v2u32 &screensize, Client &client);
	static void Begin(video::SColor color);
	static void ApplyEffect(const char* name);
	static void End();
	static void Clean();

	static bool BeginOffScreen(video::SColor color);
	static void EndOffScreen();
	static bool BeginShadowPass();
	static void EndShadowPass(video::ITexture** depthTexture, bool debugDisplay = false);
};

#endif // POSTPROCESS_H_
