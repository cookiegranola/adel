

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

	irr::video::ITexture *imageScene;
	irr::video::ITexture *imagePP[2];
	
	irr::video::SMaterial materialPP;
	irr::scene::SMeshBuffer* bufferPP;

	class IShaderDefaultPostProcessCallback* callbackPP;

	std::vector<s32> effectChain;
	
	PostProcess() : driver(NULL), imageScene(NULL), bufferPP(NULL), callbackPP(NULL) {
		imagePP[0] = NULL; imagePP[1] = NULL;
	}

	static PostProcess postProcess;

	static void SetThreshold(float threshold);
	static void SetBlendingFactor(float factor);

	static void SetDriver(irr::video::IVideoDriver *irr_driver) { postProcess.driver = irr_driver; }
	static void InitShaders(/*todo: pass list*/);
	static void Init(const v2u32 &screensize, Client &client);
	static void Begin(video::SColor color);
	static void ApplyEffect(const char* name);
	static void End();
	static void Clean();
};

#endif // POSTPROCESS_H_
