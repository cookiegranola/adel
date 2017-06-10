

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
#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_OPENGL_

#if defined(_IRR_OPENGL_USE_EXTPOINTER_)
#define GL_GLEXT_LEGACY 1
#else
#define GL_GLEXT_PROTOTYPES 1
#endif
#ifdef _IRR_WINDOWS_API_
// include windows headers for HWND
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define GL_GLEXT_LEGACY 1
#include <GL/gl.h>
#include "glext.h"
#ifdef _MSC_VER
#pragma comment(lib, "OpenGL32.lib")
#endif
#elif defined(_IRR_COMPILE_WITH_X11_DEVICE_)
#define GL_GLEXT_LEGACY 1
#define GLX_GLEXT_LEGACY 1
#include <GL/gl.h>
#include "glext.h"
#include <GL/glx.h>
#elif defined(_IRR_OSX_PLATFORM_)
#include <OpenGL/gl.h>
#elif defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
#define NO_SDL_GLEXT
#include <SDL/SDL_video.h>
#include <SDL/SDL_opengl.h>
#else
#if defined(_IRR_OSX_PLATFORM_)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif

#if !defined(_IRR_OSX_PLATFORM_)
#ifdef _IRR_WINDOWS_API_
PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif
// ARB framebuffer object
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
// EXT framebuffer object
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;

#ifdef _IRR_OPENGL_USE_EXTPOINTER_
#ifdef _IRR_WINDOWS_API_
#define IRR_OGL_LOAD_EXTENSION(x) wglGetProcAddress(reinterpret_cast<const char*>(x))
#elif defined(_IRR_COMPILE_WITH_SDL_DEVICE_) && !defined(_IRR_COMPILE_WITH_X11_DEVICE_)
#define IRR_OGL_LOAD_EXTENSION(x) SDL_GL_GetProcAddress(reinterpret_cast<const char*>(x))
#else
// Accessing the correct function is quite complex
// All libraries should support the ARB version, however
// since GLX 1.4 the non-ARB version is the official one
// So we have to check the runtime environment and
// choose the proper symbol
// In case you still have problems please enable the
// next line by uncommenting it
#define _IRR_GETPROCADDRESS_WORKAROUND_

#ifndef _IRR_GETPROCADDRESS_WORKAROUND_
__GLXextFuncPtr(*IRR_OGL_LOAD_EXTENSION_FUNCP)(const GLubyte*) = 0;
#ifdef GLX_VERSION_1_4
int major = 0, minor = 0;
if (glXGetCurrentDisplay())
glXQueryVersion(glXGetCurrentDisplay(), &major, &minor);
if ((major>1) || (minor>3))
IRR_OGL_LOAD_EXTENSION_FUNCP = glXGetProcAddress;
else
#endif
IRR_OGL_LOAD_EXTENSION_FUNCP = glXGetProcAddressARB;
#define IRR_OGL_LOAD_EXTENSION(X) IRR_OGL_LOAD_EXTENSION_FUNCP(reinterpret_cast<const GLubyte*>(X))
#else
#define IRR_OGL_LOAD_EXTENSION(X) glXGetProcAddressARB(reinterpret_cast<const GLubyte*>(X))
#endif // workaround
#endif // Windows, SDL, or Linux
#endif
#endif

//! OpenGL FBO Render Target.
class COpenGLRenderSurface : public irr::video::ITexture
{
public:

	//! FrameBufferObject constructor
	COpenGLRenderSurface(const core::dimension2d<u32>& size, const io::path& name, const bool isDepth = false,
		irr::video::ECOLOR_FORMAT format = irr::video::ECF_UNKNOWN);

	//! destructor
	virtual ~COpenGLRenderSurface();

	//! lock function
	virtual void* lock(irr::video::E_TEXTURE_LOCK_MODE mode = irr::video::ETLM_READ_WRITE, u32 mipmapLevel = 0) { return NULL; }

	//! unlock function
	virtual void unlock() {}

	//! Returns original size of the texture (image).
	virtual const core::dimension2d<u32>& getOriginalSize() const { return ImageSize; }

	//! Returns size of the texture.
	virtual const core::dimension2d<u32>& getSize() const { return TextureSize; }

	//! returns driver type of texture (=the driver, that created it)
	virtual irr::video::E_DRIVER_TYPE getDriverType() const { return irr::video::EDT_OPENGL; }

	//! returns color format of texture
	virtual irr::video::ECOLOR_FORMAT getColorFormat() const { return ColorFormat; }

	//! returns pitch of texture (in bytes)
	virtual u32 getPitch() const { return 0; }

	//! return open gl texture name
	GLuint getOpenGLTextureName() const { return TextureName; }

	//! return whether this texture has mipmaps
	virtual bool hasMipMaps() const { return false; }

	//! Regenerates the mip map levels of the texture.
	/** Useful after locking and modifying the texture
	\param mipmapData Pointer to raw mipmap data, including all necessary mip levels, in the same format as the main texture image. If not set the mipmaps are derived from the main image. */
	virtual void regenerateMipMapLevels(void* mipmapData = 0) {}

	//! Is it a render target?
	virtual bool isRenderTarget() const { return true; }

	//! Is it a FrameBufferObject?
	virtual bool isFrameBufferObject() const { return true; }

	// warning: this structure MUST be binary compatible with COpenGLTexture!
	core::dimension2d<u32> ImageSize;
	core::dimension2d<u32> TextureSize;
	irr::video::ECOLOR_FORMAT ColorFormat;
	irr::video::IVideoDriver* Driver;
	irr::video::IImage* Image;
	irr::video::IImage* MipImage;
	
	GLuint TextureName;
	GLint InternalFormat;
	GLenum PixelFormat;
	GLenum PixelType;

	u8 MipLevelStored;
	bool HasMipMaps;
	bool MipmapLegacyMode;
	bool IsRenderTarget;
	bool AutomaticMipmapUpdate;
	bool ReadOnlyLock;
	bool KeepImage;

protected:
	//! protected constructor with basic setup, no GL texture name created, for derived classes
	COpenGLRenderSurface(const io::path& name) : ITexture(name), 
		ColorFormat(irr::video::ECF_A8R8G8B8),
		TextureName(0), InternalFormat(GL_RGBA8), PixelFormat(GL_BGRA_EXT),
		PixelType(GL_UNSIGNED_BYTE) {
	}
};

COpenGLRenderSurface::COpenGLRenderSurface(const core::dimension2d<u32>& size, const io::path& name, const bool isDepth, irr::video::ECOLOR_FORMAT format) : ITexture(name)
{
	ImageSize = size;
	TextureSize = size;
	InternalFormat = GL_RGBA;
	PixelFormat = GL_RGBA; // GL_BGRA_EXT ?
	PixelType = GL_UNSIGNED_BYTE;
	
	// generate color texture
	glGenTextures(1, &TextureName);
	glBindTexture(GL_TEXTURE_2D, TextureName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (!isDepth) {
		glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, ImageSize.Width,
			ImageSize.Height, 0, PixelFormat, PixelType, 0);
	}
	else {
		// generate depth texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8_EXT/*GL_DEPTH_COMPONENT24*/, ImageSize.Width,
			ImageSize.Height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

COpenGLRenderSurface::~COpenGLRenderSurface()
{
	if (TextureName) {
		glDeleteTextures(1, &TextureName);
	}
	TextureName = 0;
}

class COpenGLRenderTarget
{
public:
	//! FrameBufferObject constructor
	COpenGLRenderTarget(const core::dimension2d<u32>& size, const io::path& name, const bool withDepth = false,
		irr::video::IVideoDriver* driver = 0, irr::video::ECOLOR_FORMAT format = irr::video::ECF_UNKNOWN);

	//! destructor
	virtual ~COpenGLRenderTarget();

	void bindRTT(bool clearColor = false, bool clearDepth = false, video::SColor = 0);
	void unbindRTT(const v2u32 &screensize);

	COpenGLRenderSurface* ColorTexture;
	COpenGLRenderSurface* DepthTexture;

protected:
	irr::video::IVideoDriver* Driver;
	GLuint ColorFrameBuffer;
};

COpenGLRenderTarget::COpenGLRenderTarget(const core::dimension2d<u32>& size, const io::path& name, const bool withDepth,
	irr::video::IVideoDriver* driver, irr::video::ECOLOR_FORMAT format)
{
	Driver = driver;
	ColorFrameBuffer = 0;
	ColorTexture = NULL;
	DepthTexture = NULL;

	// generate frame buffer
	glGenFramebuffers(1, &ColorFrameBuffer);
	bindRTT();

	ColorTexture = new COpenGLRenderSurface(size, name, false, format);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ColorTexture->TextureName);
	// attach color texture to frame buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT,
		GL_COLOR_ATTACHMENT0_EXT,
		GL_TEXTURE_2D,
		ColorTexture->TextureName,
		0);

	if (withDepth)
	{
		DepthTexture = new COpenGLRenderSurface(size, name, true, format);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, DepthTexture->TextureName);
		// attach depth texture to frame buffer
		glFramebufferTexture2D(GL_FRAMEBUFFER_EXT,
			GL_DEPTH_ATTACHMENT_EXT,
			GL_TEXTURE_2D,
			DepthTexture->TextureName,
			0);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	//GLenum check = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	//assert(check == GL_FRAMEBUFFER_COMPLETE);

	unbindRTT(size);
}

COpenGLRenderTarget::~COpenGLRenderTarget()
{
	if (ColorTexture) {
		delete ColorTexture;
		ColorTexture = NULL;
	}

	if (DepthTexture) {
		delete DepthTexture;
		DepthTexture = NULL;
	}
}

void COpenGLRenderTarget::bindRTT(bool clearColor, bool clearDepth, video::SColor color)
{
#if defined(GL_ARB_framebuffer_object)
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, ColorFrameBuffer);
#elif defined(GL_EXT_framebuffer_object)
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ColorFrameBuffer);
#else
	#error "no framebuffer support !"
#endif

	if (ColorTexture || DepthTexture)
	{
		glViewport(0, 0, ColorTexture->getSize().Width, ColorTexture->getSize().Height);
		const f32 inv = 1.0f / 255.0f;
		glClearColor(color.getRed() * inv, color.getGreen() * inv,
			color.getBlue() * inv, color.getAlpha() * inv);
		if (clearColor && clearDepth) {
			glDepthMask(GL_TRUE);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		else if (clearColor)
			glClear(GL_COLOR_BUFFER_BIT);
		else if (clearDepth) {
			glDepthMask(GL_TRUE);
			glClear(GL_DEPTH_BUFFER_BIT);
		}
	}
}

void COpenGLRenderTarget::unbindRTT(const v2u32 &screensize)
{
#if defined(GL_ARB_framebuffer_object)
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
#elif defined(GL_EXT_framebuffer_object)
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
#else
	#error "no framebuffer support !"
#endif
	//Driver->setRenderTarget(NULL);
	glViewport(0, 0, screensize.X, screensize.Y);
}
#endif // _IRR_COMPILE_WITH_OPENGL_

// ---

class IShaderDefaultPostProcessCallback : public irr::video::IShaderConstantSetCallBack
{
public:
	IShaderDefaultPostProcessCallback();

	virtual void OnSetConstants(irr::video::IMaterialRendererServices* services, irr::s32 userData);

	virtual void OnSetMaterial(const irr::video::SMaterial &material);

	inline void SetSunPosition(float x, float y) { SunPositionX = x;  SunPositionY = y; }
	inline void SetThreshold(float threshold) { Threshold = threshold; }
	inline void SetTime(float time) { Time = time; }
	inline void SetBlendFactor(float factor) { BlendFactor = factor; }
	inline void SetExposure(float factor) { Exposure = factor; }

private:
	irr::u32 NumTextures;
	irr::core::vector2df PixelSize;
	float Threshold;
	float Time;
	float BlendFactor;
	float Exposure;
	float SunPositionX, SunPositionY;
	irr::u32 RenderID;
	irr::u32 TexIDs[2];
	irr::u32 PixelSizeID;
};

IShaderDefaultPostProcessCallback::IShaderDefaultPostProcessCallback() : Threshold(0.5f), Time(0.0f), BlendFactor(1.0f), Exposure(1.0f)
{
	SunPositionX = 0.5f; SunPositionY = 0.5f;
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

	services->setPixelShaderConstant("Exposure", (irr::f32*)&Exposure, 1);
	services->setPixelShaderConstant("BlendFactor", (irr::f32*)&BlendFactor, 1);
	services->setPixelShaderConstant("Threshold", (irr::f32*)&Threshold, 1);
	services->setPixelShaderConstant("PixelSizeX"/*PixelSizeID*/, (irr::f32*)&PixelSize.X, 1);
	services->setPixelShaderConstant("PixelSizeY", (irr::f32*)&PixelSize.Y, 1);
	services->setPixelShaderConstant("SunPosition", (irr::f32*)&SunPositionX, 2);
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
		"bloom_add.frag",
		"light_scattering.frag",
		"add2.frag", 
		"albedo.frag", 
		"blur_h_add.frag", 
		"fxaa.frag",
		"debug_depth.frag",
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
		//assert(shadermat != -1);
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
	PostProcess::Effect& sunrays = postProcess.addNewEffect("sunrays");
	sunrays.addShader("light_scattering");
	// bloom
	PostProcess::Effect& bloom = postProcess.addNewEffect("bloom");
	bloom.addShader("bloom_prepass");
	bloom.addShader("blur_v");
	bloom.addShader("blur_h");
	bloom.addShader("bloom_add");	
	// blur
	PostProcess::Effect& blur = postProcess.addNewEffect("blur");
	blur.addShader("blur_v");
	blur.addShader("blur_h");
	blur.addShader("albedo");
	// debug depth
	PostProcess::Effect& depth = postProcess.addNewEffect("depth");
	depth.addShader("debug_depth");
}

void PostProcess::Init(const v2u32 &screensize, Client &client)
{
	postProcess.screenSize = screensize;

	if (!postProcess.imageScene)
	{
#if 0
		init_texture(postProcess.driver, screensize, &postProcess.imageScene, "pp_source");
		// TODO: use a viewport or dynamically recreate targets
		init_texture(postProcess.driver, screensize/2, &postProcess.imagePP[0], "pp_img1");
		init_texture(postProcess.driver, screensize/2, &postProcess.imagePP[1], "pp_img2");
#else
#ifdef _IRR_WINDOWS_API_
		glActiveTexture = (PFNGLACTIVETEXTUREPROC)IRR_OGL_LOAD_EXTENSION("glActiveTexture");
#endif
		// ARB FrameBufferObjects
		glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)IRR_OGL_LOAD_EXTENSION("glBindFramebuffer");
		glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)IRR_OGL_LOAD_EXTENSION("glDeleteFramebuffers");
		glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)IRR_OGL_LOAD_EXTENSION("glGenFramebuffers");
		glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)IRR_OGL_LOAD_EXTENSION("glCheckFramebufferStatus");
		glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)IRR_OGL_LOAD_EXTENSION("glFramebufferTexture2D");
		// EXT FrameBufferObjects
		//glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)IRR_OGL_LOAD_EXTENSION("glBindFramebufferEXT");
		//glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)IRR_OGL_LOAD_EXTENSION("glDeleteFramebuffersEXT");
		//glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)IRR_OGL_LOAD_EXTENSION("glGenFramebuffersEXT");
		//glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)IRR_OGL_LOAD_EXTENSION("glCheckFramebufferStatusEXT");
		//glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)IRR_OGL_LOAD_EXTENSION("glFramebufferTexture2DEXT");

		postProcess.imageScene = new COpenGLRenderTarget(screensize, "pp_source", true, postProcess.driver);
		postProcess.imagePP[0] = new COpenGLRenderTarget(screensize/2, "pp_img1", false, postProcess.driver);
		postProcess.imagePP[1] = new COpenGLRenderTarget(screensize/2, "pp_img2", false, postProcess.driver);
#endif

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

		postProcess.materialPP.ZBuffer = irr::video::ECFN_LESSEQUAL;
		postProcess.materialPP.ZWriteEnable = false;
		postProcess.materialPP.TextureLayer[0].BilinearFilter = true;
		postProcess.materialPP.TextureLayer[0].TextureWrapU = irr::video::ETC_CLAMP_TO_EDGE;
		postProcess.materialPP.TextureLayer[0].TextureWrapV = irr::video::ETC_CLAMP_TO_EDGE;
		postProcess.materialPP.TextureLayer[1].BilinearFilter = true;
		postProcess.materialPP.TextureLayer[1].TextureWrapU = irr::video::ETC_CLAMP_TO_EDGE;
		postProcess.materialPP.TextureLayer[1].TextureWrapV = irr::video::ETC_CLAMP_TO_EDGE;
		
		InitShaders();
	
		postProcess.lightScene = new COpenGLRenderTarget(v2u32(1024, 1024), "lightscene", true, postProcess.driver);
		postProcess.tempScene = new COpenGLRenderTarget(screensize, "temp", false, postProcess.driver);
/*
		irr::scene::ICameraSceneNode* cam = client.getSceneManager()->addCameraSceneNode(
			client.getSceneManager()->getRootSceneNode()
			, irr::core::vector3df(0, 2000, 0), irr::core::vector3df(-250.f, 100.f, -160.f)
			, -1, true);
		cam->setNearValue(0.1f);
		cam->setFarValue(10000.0f);
		cam->setFOV(0.71f);
		cam->setAspectRatio(1.0f);
		irr::core::matrix4 projectionMatrix;
#if 1
		projectionMatrix.buildProjectionMatrixPerspectiveFovLH(1.0f, 1.0f, 0.1f, 10000.0f);
		cam->setProjectionMatrix(projectionMatrix);
#else
		projectionMatrix.buildProjectionMatrixOrthoLH(1024.0f, 1024.0f, 1.0f, 10000.0f);
		cam->setProjectionMatrix(projectionMatrix, true);
#endif
		postProcess.lightCamera = cam;
		*/
	}
}

void PostProcess::Begin(video::SColor color)
{
	if (postProcess.imageScene) {
#if 0
		postProcess.driver->setRenderTarget(postProcess.imageScene, true, true, color);
#else
		postProcess.imageScene->bindRTT(true, true, color);
#endif
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
	for (int i = 0; i < shaderCount; ++i) {
		postProcess.effectChain.push_back(effect.shaders[i]);
	}
}

void PostProcess::End()
{
	if (!postProcess.imageScene)
		return;

	// maybe not be required because post process shaders don't require matrices
	postProcess.driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
	postProcess.driver->setTransform(video::ETS_VIEW, core::IdentityMatrix);
	postProcess.driver->setTransform(video::ETS_PROJECTION, core::IdentityMatrix);

	video::SMaterial oldmaterial = postProcess.driver->getMaterial2D();

	std::vector<s32>& shaders = postProcess.effectChain;
	u32 shaderCount = shaders.size();
	s32 finalShader = shaders[shaderCount - 1];

	//bool clearRT = true;

	if (shaderCount > 1)
	{
		int currentPP = 0;
		int shadermat = shaders[0];
		
		postProcess.materialPP.MaterialType = (irr::video::E_MATERIAL_TYPE)shadermat;
#if 0
		postProcess.driver->setRenderTarget(postProcess.imagePP[currentPP], clearRT, false);
		postProcess.materialPP.setTexture(0, postProcess.imageScene);
		postProcess.driver->setMaterial(postProcess.materialPP);
#else
		postProcess.imagePP[currentPP]->bindRTT(true);
		postProcess.materialPP.setTexture(0, postProcess.imageScene->ColorTexture);
		postProcess.driver->setMaterial(postProcess.materialPP);
#endif
		postProcess.driver->drawMeshBuffer(postProcess.bufferPP);
		
// light scattering
		postProcess.materialPP.MaterialType = (irr::video::E_MATERIAL_TYPE)postProcess.getShader("add2");
		postProcess.imagePP[currentPP ^ 1]->bindRTT(true);
		postProcess.materialPP.setTexture(0, postProcess.tempScene->ColorTexture);
		postProcess.materialPP.setTexture(1, postProcess.imagePP[currentPP]->ColorTexture);
		currentPP ^= 1;
		postProcess.driver->setMaterial(postProcess.materialPP);
		postProcess.driver->drawMeshBuffer(postProcess.bufferPP);
		
		for (u32 i = 1; i < shaderCount; ++i) {
			int shadermat = shaders[i];
			postProcess.materialPP.MaterialType = (irr::video::E_MATERIAL_TYPE)shadermat;
#if 0
			video::ITexture* target = (i == (shaderCount - 1)) ? NULL : postProcess.imagePP[currentPP ^ 1];
			postProcess.driver->setRenderTarget(target, clearRT, false);
			postProcess.materialPP.setTexture(0, postProcess.imagePP[currentPP]);
			// for now force texture 1 to be the original scene
			postProcess.materialPP.setTexture(1, postProcess.imageScene);
			postProcess.driver->setMaterial(postProcess.materialPP);
#else
			if (i == (shaderCount - 1)) {
				postProcess.imageScene->unbindRTT(postProcess.screenSize);
			}
			else {
				postProcess.imagePP[currentPP ^ 1]->bindRTT(true);
			}
			postProcess.materialPP.setTexture(0, postProcess.imagePP[currentPP]->ColorTexture);
			// for now force texture 1 to be the original scene
			postProcess.materialPP.setTexture(1, postProcess.imageScene->ColorTexture);
			postProcess.driver->setMaterial(postProcess.materialPP);
#endif
			postProcess.driver->drawMeshBuffer(postProcess.bufferPP);
			currentPP ^= 1;
		}
	}
	else
	{
		int shadermat = finalShader;
		postProcess.materialPP.MaterialType = (irr::video::E_MATERIAL_TYPE)shadermat;
#if 0
		postProcess.driver->setRenderTarget(0, false, false);
		postProcess.materialPP.setTexture(0, postProcess.imageScene);
		postProcess.driver->setMaterial(postProcess.materialPP);
#else
		postProcess.imageScene->unbindRTT(postProcess.screenSize);
		postProcess.materialPP.setTexture(0, postProcess.tempScene->ColorTexture);
		postProcess.driver->setMaterial(postProcess.materialPP);
#endif
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
#if 0
		postProcess.driver->removeTexture(postProcess.imagePP[1]);
		postProcess.driver->removeTexture(postProcess.imagePP[0]);
		postProcess.driver->removeTexture(postProcess.imageScene);
#else
		delete postProcess.tempScene;
		delete postProcess.lightScene;
		delete postProcess.imagePP[1];
		delete postProcess.imagePP[0];
		delete postProcess.imageScene;
#endif
		postProcess.lightScene = NULL;
		postProcess.imagePP[1] = NULL;
		postProcess.imagePP[0] = NULL;
		postProcess.imageScene = NULL;
	}
}

void PostProcess::SetSunPosition(float x, float y) { postProcess.callbackPP->SetSunPosition(x, y); }
void PostProcess::SetThreshold(float threshold) { postProcess.callbackPP->SetThreshold(threshold); }
void PostProcess::SetBlendingFactor(float factor) { postProcess.callbackPP->SetBlendFactor(factor); }
void PostProcess::SetExposure(float factor) { postProcess.callbackPP->SetExposure(factor); }


// ---

bool PostProcess::BeginOffScreen(video::SColor color)
{
	if (!postProcess.tempScene)
		return false;
	postProcess.tempScene->bindRTT(true, true);
	postProcess.imageScene->bindRTT(true, true, color);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	return true;
}

void PostProcess::EndOffScreen()
{
	{
		// maybe not be required because post process shaders don't require matrices
		postProcess.driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
		postProcess.driver->setTransform(video::ETS_VIEW, core::IdentityMatrix);
		postProcess.driver->setTransform(video::ETS_PROJECTION, core::IdentityMatrix);

		//PostProcess::ApplyEffect("depth");
		//PostProcess::ApplyEffect("copy");
		PostProcess::ApplyEffect("sunrays");

		//video::SMaterial oldmaterial = postProcess.driver->getMaterial2D();

		std::vector<s32>& shaders = postProcess.effectChain;
		u32 shaderCount = shaders.size();
		s32 finalShader = shaders[shaderCount - 1];

		int shadermat = finalShader;
		postProcess.materialPP.MaterialType = (irr::video::E_MATERIAL_TYPE)shadermat;

		//postProcess.lightScene->unbindRTT(postProcess.screenSize);
		postProcess.tempScene->bindRTT(true);
		postProcess.materialPP.setTexture(0, postProcess.imageScene->ColorTexture);
		//postProcess.materialPP.setTexture(0, postProcess.lightScene->DepthTexture);
		postProcess.driver->setMaterial(postProcess.materialPP);

		postProcess.driver->drawMeshBuffer(postProcess.bufferPP);
	
		postProcess.effectChain.clear();
	}
}

// ---

bool PostProcess::BeginShadowPass()
{
	if (!postProcess.lightScene)
		return false;
	//postProcess.imageScene->bindRTT(true, true);
	/*irr::core::matrix4 projectionMatrix, viewMatrix;
	viewMatrix.buildCameraLookAtMatrixLH(irr::core::vector3df(0.f, 0.f, 0.f), irr::core::vector3df(-136, 16, -126), irr::core::vector3df(0.0f, 1.f, 0.0f));
	projectionMatrix.buildProjectionMatrixPerspectiveFovLH(0.71f, 1.0f, 0.1f, 10000.0f);
	postProcess.driver->setTransform(video::ETS_PROJECTION,projectionMatrix);
	postProcess.driver->setTransform(video::ETS_VIEW, viewMatrix);*/
	return true;
}

void PostProcess::EndShadowPass(video::ITexture** depthTexture, bool displayDebug)
{
	if (displayDebug)
	{
		// maybe not be required because post process shaders don't require matrices
		postProcess.driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
		postProcess.driver->setTransform(video::ETS_VIEW, core::IdentityMatrix);
		postProcess.driver->setTransform(video::ETS_PROJECTION, core::IdentityMatrix);

		//PostProcess::ApplyEffect("depth");
		PostProcess::ApplyEffect("copy");
		
		//video::SMaterial oldmaterial = postProcess.driver->getMaterial2D();

		std::vector<s32>& shaders = postProcess.effectChain;
		u32 shaderCount = shaders.size();
		s32 finalShader = shaders[shaderCount - 1];

		int shadermat = finalShader;
		postProcess.materialPP.MaterialType = (irr::video::E_MATERIAL_TYPE)shadermat;

		//postProcess.lightScene->unbindRTT(postProcess.screenSize);
		postProcess.lightScene->bindRTT(true);
		postProcess.materialPP.setTexture(0, postProcess.imageScene->ColorTexture);
		//postProcess.materialPP.setTexture(0, postProcess.lightScene->DepthTexture);
		postProcess.driver->setMaterial(postProcess.materialPP);

		postProcess.driver->drawMeshBuffer(postProcess.bufferPP);

		postProcess.effectChain.clear();
	}

	if (depthTexture) {
		*depthTexture = postProcess.lightScene->DepthTexture;
	}
}
