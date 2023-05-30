#pragma once

#include <glad/glad.h>
#include <vector>
#include <unordered_map>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include "types.h"

namespace game::gl
{
	void initState();
	void destroyState();

	enum class DataFormat : GLenum
	{
		R = GL_RED,
		Rg = GL_RG,
		Rgb = GL_RGB,
		Rgba = GL_RGBA
	};

	enum class DataType : GLenum
	{
		U8 = GL_UNSIGNED_BYTE,
		I8 = GL_BYTE,
		U16 = GL_UNSIGNED_SHORT,
		I16 = GL_SHORT,
		U32 = GL_UNSIGNED_INT,
		I32 = GL_INT,
		Fixed = GL_FIXED,
		F32 = GL_FLOAT,
		F64 = GL_DOUBLE
	};

	enum class TextureFormat : GLenum
	{
		Rgba2un = GL_RGBA2,
		Rgb4un = GL_RGB4,
		Rgba4un = GL_RGBA4,
		Rgb5un = GL_RGB5,
		Rgb565un = GL_RGB565,
		R8un = GL_R8,
		Rg8un = GL_RG8,
		Rgb8un = GL_RGB8,
		Rgba8un = GL_RGBA8,
		R8sn = GL_R8_SNORM,
		Rg8sn = GL_RG8_SNORM,
		Rgb8sn = GL_RGB8_SNORM,
		Rgba8sn = GL_RGBA8_SNORM,
		R8u = GL_R8UI,
		Rg8u = GL_RG8UI,
		Rgb8u = GL_RGB8UI,
		Rgba8u = GL_RGBA8UI,
		R8i = GL_R8I,
		Rg8i = GL_RG8I,
		Rgb8i = GL_RGB8I,
		Rgba8i = GL_RGBA8I,
		Rgb10un = GL_RGB10,
		Rgb12un = GL_RGB12,
		Rgba12un = GL_RGBA12,
		R16un = GL_R16,
		Rg16un = GL_RG16,
		Rgb16un = GL_RGB16,
		Rgba16un = GL_RGBA16,
		R16sn = GL_R16_SNORM,
		Rg16sn = GL_RG16_SNORM,
		Rgb16sn = GL_RGB16_SNORM,
		Rgba16sn = GL_RGBA16_SNORM,
		R16u = GL_R16UI,
		Rg16u = GL_RG16UI,
		Rgb16u = GL_RGB16UI,
		Rgba16u = GL_RGBA16UI,
		R16i = GL_R16I,
		Rg16i = GL_RG16I,
		Rgb16i = GL_RGB16I,
		Rgba16i = GL_RGBA16I,
		R16f = GL_R16F,
		Rg16f = GL_RG16F,
		Rgb16f = GL_RGB16F,
		Rgba16f = GL_RGBA16F,
		R32u = GL_R32UI,
		Rg32u = GL_RG32UI,
		Rgb32u = GL_RGB32UI,
		Rgba32u = GL_RGBA32UI,
		R32i = GL_R32I,
		Rg32i = GL_RG32I,
		Rgb32i = GL_RGB32I,
		Rgba32i = GL_RGBA32I,
		R32f = GL_R32F,
		Rg32f = GL_RG32F,
		Rgb32f = GL_RGB32F,
		Rgba32f = GL_RGBA32F,
		R3G3B2 = GL_R3_G3_B2,
		Rgb5A1un = GL_RGB5_A1,
		Rgb10A2un = GL_RGB10_A2,
		Rgb10A2u = GL_RGB10_A2UI,
		R11G11B10f = GL_R11F_G11F_B10F,
		Rgb9e5f = GL_RGB9_E5,
		Srgb8 = GL_SRGB8,
		Srgb8A8 = GL_SRGB8_ALPHA8,
		ComprRRgtc1 = GL_COMPRESSED_RED_RGTC1,
		ComprRRgtc1Snorm = GL_COMPRESSED_SIGNED_RED_RGTC1,
		ComprRgRgtc2 = GL_COMPRESSED_RG_RGTC2,
		ComprRgRgtc2Snorm = GL_COMPRESSED_SIGNED_RG_RGTC2,
		ComprRgbaBptc = GL_COMPRESSED_RGBA_BPTC_UNORM,
		ComprSrgbABptc = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,
		ComprRgbBptcf = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT,
		ComprRgbBptcuf = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,
		Stencil8 = GL_STENCIL_INDEX8
	};

	enum class TextureMinFilter : GLint
	{
		Nearest = GL_NEAREST,
		Linear = GL_LINEAR,
		NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
		LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
		NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
		LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
	};

	enum class TextureMagFilter : GLint
	{
		Nearest = GL_NEAREST,
		Linear = GL_LINEAR
	};

	enum class TextureWrapMode : GLint
	{
		ClampToEdge = GL_CLAMP_TO_EDGE,
		ClampToBorder = GL_CLAMP_TO_BORDER,
		MirroredRepeat = GL_MIRRORED_REPEAT,
		Repeat = GL_REPEAT,
		MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE
	};

	enum class TextureComponent : GLenum
	{
		R = GL_TEXTURE_SWIZZLE_R,
		G = GL_TEXTURE_SWIZZLE_G,
		B = GL_TEXTURE_SWIZZLE_B,
		A = GL_TEXTURE_SWIZZLE_A,
		Rgba = GL_TEXTURE_SWIZZLE_RGBA,
	};

	enum class TextureSwizzle : GLint
	{
		Red = GL_RED,
		Green = GL_GREEN,
		Blue = GL_BLUE,
		Alpha = GL_ALPHA,
		Zero = GL_ZERO,
		One = GL_ONE
	};

	class SingleTexture
	{
	public:
		SingleTexture(u32 width, u32 height, u32 depth, GLuint handle);
		SingleTexture(u32 width, TextureFormat format, u32 levels = 1);
		SingleTexture(u32 width, u32 height, TextureFormat format, u32 levels = 1, u32 samples = 1);
		SingleTexture(u32 width, u32 height, u32 depth, TextureFormat format, u32 levels = 1);
		explicit SingleTexture(const std::string &id);
		~SingleTexture();

		SingleTexture(SingleTexture &&) noexcept;

		[[nodiscard]] inline auto handle() const { return m_texture; }

		[[nodiscard]] inline auto width() const { return m_width; }
		[[nodiscard]] inline auto height() const { return m_height; }
		[[nodiscard]] inline auto depth() const { return m_depth; }

		void bind(u32 unit) const;
		void unbind(u32 unit) const;

		[[nodiscard]] TextureMinFilter minFilter() const;
		[[nodiscard]] TextureMagFilter magFilter() const;

		void minFilter(TextureMinFilter filter);
		void magFilter(TextureMagFilter filter);

		[[nodiscard]] TextureWrapMode wrapS() const;
		[[nodiscard]] TextureWrapMode wrapT() const;
		[[nodiscard]] TextureWrapMode wrapR() const;

		void wrapS(TextureWrapMode mode);
		void wrapT(TextureWrapMode mode);
		void wrapR(TextureWrapMode mode);

		[[nodiscard]] TextureSwizzle swizzle(TextureComponent component) const;
		void swizzle(TextureComponent component, TextureSwizzle swizzle);

		SingleTexture(const SingleTexture &) = delete;

		SingleTexture &operator=(const SingleTexture &) = delete;
		SingleTexture &operator=(SingleTexture &&) = delete;

	private:
		GLuint m_texture = 0;
		u32 m_width, m_height = 1, m_depth = 1;
	};

	enum class FramebufferTarget : GLenum
	{
		Read = GL_READ_FRAMEBUFFER,
		Draw = GL_DRAW_FRAMEBUFFER,
		Both = GL_FRAMEBUFFER
	};

	class ShaderProgram;

	class Framebuffer
	{
	public:
		Framebuffer(u32 width, u32 height, std::vector<TextureFormat> colourAttachments, u32 samples = 1);
		~Framebuffer();

		Framebuffer(Framebuffer &&) noexcept;

		[[nodiscard]] inline auto handle() const { return m_framebuffer; }

		[[nodiscard]] inline auto &colourAttachments() { return m_colourAttachments; }

		[[nodiscard]] inline auto width() const { return m_width; }
		[[nodiscard]] inline auto height() const { return m_height; }

		void draw(ShaderProgram &shader);

		void bind(FramebufferTarget target = FramebufferTarget::Draw) const;
		void unbind(FramebufferTarget target = FramebufferTarget::Draw) const;

		void resize(u32 width, u32 height);

		void blitColourTo(Framebuffer &other, bool linear = true) const;

		Framebuffer(const Framebuffer &) = delete;

		Framebuffer &operator=(const Framebuffer &) = delete;
		Framebuffer &operator=(Framebuffer &&) = delete;

	private:
		GLuint m_framebuffer = 0;
		u32 m_width, m_height;

		u32 m_samples;

		std::vector<TextureFormat> m_colourAttachmentFormats;
		std::vector<SingleTexture> m_colourAttachments;

		void _resize();
	};

	enum class ShaderType : GLenum
	{
		Vertex = GL_VERTEX_SHADER,
		TessControl = GL_TESS_CONTROL_SHADER,
		TessEval = GL_TESS_EVALUATION_SHADER,
		Geometry = GL_GEOMETRY_SHADER,
		Fragment = GL_FRAGMENT_SHADER,
		Compute = GL_COMPUTE_SHADER
	};

	class ShaderProgram
	{
	public:
		~ShaderProgram();

		ShaderProgram(ShaderProgram &&) noexcept;

		[[nodiscard]] inline auto handle() const { return m_program; }

		[[nodiscard]] GLint uniformHandle(const std::string &name);

		void upload(const std::string &uniform, u32 x);
		void upload(const std::string &uniform, i32 x);
		void upload(const std::string &uniform, f32 x);
		void upload(const std::string &uniform, f64 x);
		void upload(const std::string &uniform, bool x);

		void upload(const std::string &uniform, u32 x, u32 y);
		void upload(const std::string &uniform, i32 x, i32 y);
		void upload(const std::string &uniform, f32 x, f32 y);
		void upload(const std::string &uniform, f64 x, f64 y);
		void upload(const std::string &uniform, bool x, bool y);

		void upload(const std::string &uniform, u32 x, u32 y, u32 z);
		void upload(const std::string &uniform, i32 x, i32 y, i32 z);
		void upload(const std::string &uniform, f32 x, f32 y, f32 z);
		void upload(const std::string &uniform, f64 x, f64 y, f64 z);
		void upload(const std::string &uniform, bool x, bool y, bool z);

		void upload(const std::string &uniform, u32 x, u32 y, u32 z, u32 w);
		void upload(const std::string &uniform, i32 x, i32 y, i32 z, i32 w);
		void upload(const std::string &uniform, f32 x, f32 y, f32 z, f32 w);
		void upload(const std::string &uniform, f64 x, f64 y, f64 z, f64 w);
		void upload(const std::string &uniform, bool x, bool y, bool z, bool w);

		void upload(const std::string &uniform, const glm::uvec2 &v);
		void upload(const std::string &uniform, const glm::ivec2 &v);
		void upload(const std::string &uniform, const glm::vec2 &v);
		void upload(const std::string &uniform, const glm::dvec2 &v);
		void upload(const std::string &uniform, const glm::bvec2 &v);

		void upload(const std::string &uniform, const glm::uvec3 &v);
		void upload(const std::string &uniform, const glm::ivec3 &v);
		void upload(const std::string &uniform, const glm::vec3 &v);
		void upload(const std::string &uniform, const glm::dvec3 &v);
		void upload(const std::string &uniform, const glm::bvec3 &v);

		void upload(const std::string &uniform, const glm::uvec4 &v);
		void upload(const std::string &uniform, const glm::ivec4 &v);
		void upload(const std::string &uniform, const glm::vec4 &v);
		void upload(const std::string &uniform, const glm::dvec4 &v);
		void upload(const std::string &uniform, const glm::bvec4 &v);

		void upload(const std::string &uniform, const glm::mat2 &v);
		void upload(const std::string &uniform, const glm::mat3 &v);
		void upload(const std::string &uniform, const glm::mat4 &v);

		void upload(const std::string &uniform, const glm::dmat2 &v);
		void upload(const std::string &uniform, const glm::dmat3 &v);
		void upload(const std::string &uniform, const glm::dmat4 &v);

		void bind() const;
		void unbind() const;

		ShaderProgram(const ShaderProgram &) = delete;

		ShaderProgram &operator=(const ShaderProgram &) = delete;
		ShaderProgram &operator=(ShaderProgram &&) = delete;

	private:
		explicit ShaderProgram(const std::unordered_map<ShaderType, std::string> &shaders);

		GLuint m_program = 0;
		std::unordered_map<std::string, GLint> m_uniformCache;

		friend class ShaderBuilder;
	};

	class ShaderBuilder
	{
	public:
		ShaderBuilder &vertex(std::string id);
		ShaderBuilder &tessControl(std::string id);
		ShaderBuilder &tessEval(std::string id);
		ShaderBuilder &geometry(std::string id);
		ShaderBuilder &fragment(std::string id);
		ShaderBuilder &compute(std::string id);

		[[nodiscard]] ShaderProgram build();

	private:
		std::unordered_map<ShaderType, std::string> m_shaders;
	};

	void bindDefaultVao();
}
