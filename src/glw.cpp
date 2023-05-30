#include "glw.h"
#include <utility>
#include <vector>
#include <sstream>
#include "assets.h"
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <memory>
#include "config.h"

namespace game::gl
{
	namespace
	{
		class GlStateTracker
		{
		public:
			GlStateTracker()
			{
				GLint maxTextureUnits = 0;
				glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);

				m_currentTextures.resize(maxTextureUnits);

				glCreateVertexArrays(1, &m_emptyVao);
				bindVao(m_emptyVao);
			}

			~GlStateTracker()
			{
				glDeleteVertexArrays(1, &m_emptyVao);
			}

			void bindTexture(u32 unit, GLuint texture)
			{
				if(unit < m_currentTextures.size() && texture != m_currentTextures[unit])
				{
					glBindTextureUnit(unit, texture);
					m_currentTextures[unit] = texture;
				}
			}

			void bindShaderProgram(GLuint program)
			{
				if(program != m_currentShaderProgram)
				{
					glUseProgram(program);
					m_currentShaderProgram = program;
				}
			}

			void bindVao(GLuint vao)
			{
				if(vao == 0) vao = m_emptyVao;

				if(vao != m_currentVao)
				{
					glBindVertexArray(vao);
					m_currentVao = vao;
				}
			}

			void bindFramebuffer(FramebufferTarget target, GLuint framebuffer)
			{
				if(target == FramebufferTarget::Both || target == FramebufferTarget::Read)
				{
					if(framebuffer != m_currentReadFramebuffer)
					{
						glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
						m_currentReadFramebuffer = framebuffer;
					}
				}

				if(target == FramebufferTarget::Both || target == FramebufferTarget::Draw)
				{
					if(framebuffer != m_currentDrawFramebuffer)
					{
						glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
						m_currentDrawFramebuffer = framebuffer;
					}
				}
			}

			void drawFullscreenQuad()
			{
				bindVao(m_emptyVao);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}

			GlStateTracker(const GlStateTracker &) = delete;
			GlStateTracker(GlStateTracker &&) = delete;

			GlStateTracker &operator=(const GlStateTracker &) = delete;
			GlStateTracker &operator=(GlStateTracker &&) = delete;

		private:
			std::vector<GLuint> m_currentTextures;
			GLuint m_currentShaderProgram = 0;
			GLuint m_currentVao = 0;
			GLuint m_currentReadFramebuffer = 0;
			GLuint m_currentDrawFramebuffer = 0;

			GLuint m_emptyVao = 0;
		};

		std::unique_ptr<GlStateTracker> s_state { nullptr };
	}

	void initState()
	{
		assets::addAssetType("shader", "shaders", "glsl");

		s_state = std::make_unique<GlStateTracker>();
	}

	void destroyState()
	{
		s_state.reset();
	}

	SingleTexture::SingleTexture(u32 width, u32 height, u32 depth, GLuint handle)
		: m_texture(handle),
		  m_width(width), m_height(height), m_depth(depth) {}

	SingleTexture::SingleTexture(u32 width, TextureFormat format, u32 levels)
		: m_width(width)
	{
		glCreateTextures(GL_TEXTURE_1D, 1, &m_texture);
		glTextureStorage1D(m_texture, static_cast<GLsizei>(levels), static_cast<GLenum>(format), static_cast<GLsizei>(width));
	}

	SingleTexture::SingleTexture(u32 width, u32 height, TextureFormat format, u32 levels, u32 samples)
		: m_width(width), m_height(height)
	{
		if(samples > 1)
		{
			glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &m_texture);
			glTextureStorage2DMultisample(m_texture, static_cast<GLsizei>(samples), static_cast<GLenum>(format), static_cast<GLsizei>(width), static_cast<GLsizei>(height), GL_FALSE);
		}
		else
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
			glTextureStorage2D(m_texture, static_cast<GLsizei>(levels), static_cast<GLenum>(format), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
		}
	}

	SingleTexture::SingleTexture(u32 width, u32 height, u32 depth, TextureFormat format, u32 levels)
		: m_width(width), m_height(height), m_depth(depth)
	{
		glCreateTextures(GL_TEXTURE_3D, 1, &m_texture);
		glTextureStorage3D(m_texture, static_cast<GLsizei>(levels), static_cast<GLenum>(format), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLsizei>(depth));
	}

	SingleTexture::SingleTexture(const std::string &id)
		: m_width(0), m_height(0), m_depth(0)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);

		if(auto data = assets::loadImage(id); data)
		{
			m_width = data->m_width;
			m_height = data->m_height;
			m_depth = 1;

			glTextureStorage2D(m_texture, 1, GL_SRGB8_ALPHA8, static_cast<GLsizei>(data->m_width), static_cast<GLsizei>(data->m_height));
			glTextureSubImage2D(m_texture, 0, 0, 0, static_cast<GLsizei>(data->m_width), static_cast<GLsizei>(data->m_height), data->m_channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data->m_data);
		}
	}

	SingleTexture::~SingleTexture()
	{
		if(m_texture) glDeleteTextures(1, &m_texture);
	}

	SingleTexture::SingleTexture(SingleTexture &&other) noexcept
		: m_texture(other.m_texture),
		  m_width(other.m_width), m_height(other.m_height), m_depth(other.m_depth)
	{
		other.m_texture = 0;
	}

	void SingleTexture::bind(u32 unit) const
	{
		s_state->bindTexture(unit, m_texture);
	}

	void SingleTexture::unbind(u32 unit) const
	{
		s_state->bindTexture(unit, 0);
	}

	TextureMinFilter SingleTexture::minFilter() const
	{
		GLint filter;
		glGetTextureParameteriv(m_texture, GL_TEXTURE_MIN_FILTER, &filter);
		return static_cast<TextureMinFilter>(filter);
	}

	TextureMagFilter SingleTexture::magFilter() const
	{
		GLint filter;
		glGetTextureParameteriv(m_texture, GL_TEXTURE_MAG_FILTER, &filter);
		return static_cast<TextureMagFilter>(filter);
	}

	void SingleTexture::minFilter(TextureMinFilter filter)
	{
		glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(filter));
	}

	void SingleTexture::magFilter(TextureMagFilter filter)
	{
		glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(filter));
	}

	TextureWrapMode SingleTexture::wrapS() const
	{
		GLint mode;
		glGetTextureParameteriv(m_texture, GL_TEXTURE_WRAP_S, &mode);
		return static_cast<TextureWrapMode>(mode);
	}

	TextureWrapMode SingleTexture::wrapT() const
	{
		GLint mode;
		glGetTextureParameteriv(m_texture, GL_TEXTURE_WRAP_T, &mode);
		return static_cast<TextureWrapMode>(mode);
	}

	TextureWrapMode SingleTexture::wrapR() const
	{
		GLint mode;
		glGetTextureParameteriv(m_texture, GL_TEXTURE_WRAP_R, &mode);
		return static_cast<TextureWrapMode>(mode);
	}

	void SingleTexture::wrapS(TextureWrapMode mode)
	{
		glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, static_cast<GLint>(mode));
	}

	void SingleTexture::wrapT(TextureWrapMode mode)
	{
		glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, static_cast<GLint>(mode));
	}

	void SingleTexture::wrapR(TextureWrapMode mode)
	{
		glTextureParameteri(m_texture, GL_TEXTURE_WRAP_R, static_cast<GLint>(mode));
	}

	TextureSwizzle SingleTexture::swizzle(TextureComponent component) const
	{
		GLint swizzle;
		glGetTextureParameteriv(m_texture, static_cast<GLenum>(component), &swizzle);
		return static_cast<TextureSwizzle>(swizzle);
	}

	void SingleTexture::swizzle(TextureComponent component, TextureSwizzle swizzle)
	{
		glTextureParameteri(m_texture, static_cast<GLenum>(component), static_cast<GLint>(swizzle));
	}

	Framebuffer::Framebuffer(u32 width, u32 height, std::vector<TextureFormat> colourAttachments, u32 samples)
		: m_colourAttachmentFormats(std::move(colourAttachments)),
		  m_width(width), m_height(height),
		  m_samples(samples)
	{
		_resize();
	}

	Framebuffer::~Framebuffer()
	{
		if(m_framebuffer) glDeleteFramebuffers(1, &m_framebuffer);
	}

	Framebuffer::Framebuffer(Framebuffer &&other) noexcept
		: m_colourAttachmentFormats(other.m_colourAttachmentFormats),
		  m_colourAttachments(std::move(other.m_colourAttachments)),
		  m_width(other.m_width), m_height(other.m_height),
		  m_samples(other.m_samples)
	{
		other.m_framebuffer = 0;
	}

	void Framebuffer::draw(ShaderProgram &shader)
	{
		if(m_colourAttachments.empty()) return;

		shader.bind();

		for(u32 i = 0; i < m_colourAttachments.size(); ++i)
		{
			m_colourAttachments[i].bind(i);
		}

		s_state->drawFullscreenQuad();
	}

	void Framebuffer::bind(FramebufferTarget target) const
	{
		s_state->bindFramebuffer(target, m_framebuffer);
	}

	void Framebuffer::unbind(FramebufferTarget target) const
	{
		s_state->bindFramebuffer(target, 0);
	}

	void Framebuffer::resize(u32 width, u32 height)
	{
		if(width != m_width || height != m_height)
		{
			if(m_framebuffer) glDeleteFramebuffers(1, &m_framebuffer);

			m_width = width;
			m_height = height;

			_resize();
		}
	}

	void Framebuffer::blitColourTo(Framebuffer &other, bool linear) const
	{
		glBlitNamedFramebuffer(m_framebuffer, other.m_framebuffer, 0, 0, static_cast<GLint>(m_width), static_cast<GLint>(m_height), 0, 0, static_cast<GLint>(other.m_width), static_cast<GLint>(other.m_height), GL_COLOR_BUFFER_BIT, linear ? GL_LINEAR : GL_NEAREST);
	}

	void Framebuffer::_resize()
	{
		glCreateFramebuffers(1, &m_framebuffer);

		if(!m_colourAttachmentFormats.empty())
		{
			if(m_colourAttachments.empty()) m_colourAttachments.reserve(m_colourAttachmentFormats.size());
			else m_colourAttachments.clear();

			for(size_t i = 0; i < m_colourAttachmentFormats.size(); ++i)
			{
				auto &texture = m_colourAttachments.emplace_back(m_width, m_height, m_colourAttachmentFormats[i], 1, m_samples);
				glNamedFramebufferTexture(m_framebuffer, GL_COLOR_ATTACHMENT0 + i, texture.handle(), 0);
			}
		}

		m_colourAttachments.shrink_to_fit();
	}

	namespace
	{
		const std::unordered_map<ShaderType, std::string> ShaderSuffixes {
			{ ShaderType::Vertex, ".v" },
			{ ShaderType::TessControl, ".tc" },
			{ ShaderType::TessEval, ".te" },
			{ ShaderType::Geometry, ".g" },
			{ ShaderType::Fragment, ".f" },
			{ ShaderType::Compute, ".c" }
		};

		struct ShaderObject
		{
			explicit ShaderObject(ShaderType type)
				: m_shader(glCreateShader(static_cast<GLenum>(type))) {}

			~ShaderObject()
			{
				if(m_shader) glDeleteShader(m_shader);
			}

			ShaderObject(ShaderObject &&other) noexcept
				: m_shader(other.m_shader)
			{
				other.m_shader = 0;
			}

			ShaderObject(const ShaderObject &) = delete;

			ShaderObject &operator=(const ShaderObject &) = delete;
			ShaderObject &operator=(ShaderObject &&) = delete;

			GLuint m_shader;
		};

		struct ProgramObject
		{
			ProgramObject()
				: m_program(glCreateProgram()) {}

			~ProgramObject()
			{
				if(m_program) glDeleteProgram(m_program);
			}

			ProgramObject(ProgramObject &&other) noexcept
				: m_program(other.m_program)
			{
				other.m_program = 0;
			}

			[[nodiscard]] inline auto release()
			{
				auto r = m_program;
				m_program = 0;
				return r;
			}

			ProgramObject(const ProgramObject &) = delete;

			ProgramObject &operator=(const ProgramObject &) = delete;
			ProgramObject &operator=(ProgramObject &&) = delete;

			GLuint m_program;
		};
	}

	ShaderProgram::ShaderProgram(const std::unordered_map<ShaderType, std::string> &shaders)
	{
		ProgramObject program;

		std::vector<ShaderObject> shaderObjects;
		shaderObjects.reserve(shaders.size());

		for(const auto &[type, id] : shaders)
		{
			std::optional<std::string> src = assets::loadText("shader", id + ShaderSuffixes.at(type));

			if(src.has_value())
			{
				const auto &shader = shaderObjects.emplace_back(type);

				std::ostringstream ssrc;
				ssrc << "#version 450 core\n#extension GL_ARB_bindless_texture : enable\n" << *src;

				// pain
				const auto srcStr = ssrc.str();
				const auto *srcCStr = srcStr.c_str();

				glShaderSource(shader.m_shader, 1, &srcCStr, nullptr);
				glCompileShader(shader.m_shader);

				GLint status = 0;
				glGetShaderiv(shader.m_shader, GL_COMPILE_STATUS, &status);

				if(!status)
				{
					GLint logLength = 0;
					glGetShaderiv(shader.m_shader, GL_INFO_LOG_LENGTH, &logLength);

					std::string log;
					log.resize(static_cast<size_t>(logLength));

					glGetShaderInfoLog(shader.m_shader, logLength, &logLength, log.data());

					std::ostringstream err;
					err << "Failed to load shader " << id << ShaderSuffixes.at(type) << ":\n" << log;

					throw std::runtime_error(err.str());
				}

				glAttachShader(program.m_program, shader.m_shader);
			}
			else throw std::runtime_error("Failed to load shader");
		}

		glLinkProgram(program.m_program);

		for(const auto &shader : shaderObjects)
		{
			glDetachShader(program.m_program, shader.m_shader);
		}

		GLint status = 0;
		glGetProgramiv(program.m_program, GL_LINK_STATUS, &status);

		if(!status)
		{
			GLint logLength = 0;
			glGetProgramiv(program.m_program, GL_INFO_LOG_LENGTH, &logLength);

			std::string log;
			log.resize(static_cast<size_t>(logLength));

			glGetProgramInfoLog(program.m_program, logLength, &logLength, log.data());

			std::ostringstream err;
			err << "Failed to link shader program:\n" << log;

			throw std::runtime_error(err.str());
		}

		m_program = program.release();
	}

	ShaderProgram::~ShaderProgram()
	{
		if(m_program) glDeleteProgram(m_program);
	}

	ShaderProgram::ShaderProgram(ShaderProgram &&other) noexcept
		: m_program(other.m_program),
		  m_uniformCache(std::move(other.m_uniformCache))
	{
		other.m_program = 0;
	}

	GLint ShaderProgram::uniformHandle(const std::string &name)
	{
		GLint loc;

		if(const auto iter = m_uniformCache.find(name); iter == m_uniformCache.end())
			loc = m_uniformCache[name] = glGetUniformLocation(m_program, name.c_str());
		else loc = iter->second;

		return loc;
	}

	void ShaderProgram::upload(const std::string &uniform, u32 x)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform1ui(m_program, loc, x);
	}

	void ShaderProgram::upload(const std::string &uniform, i32 x)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform1i(m_program, loc, x);
	}

	void ShaderProgram::upload(const std::string &uniform, f32 x)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform1f(m_program, loc, x);
	}

	void ShaderProgram::upload(const std::string &uniform, f64 x)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform1d(m_program, loc, x);
	}

	void ShaderProgram::upload(const std::string &uniform, bool x)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform1i(m_program, loc, x ? 1 : 0);
	}

	void ShaderProgram::upload(const std::string &uniform, u32 x, u32 y)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform2ui(m_program, loc, x, y);
	}

	void ShaderProgram::upload(const std::string &uniform, i32 x, i32 y)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform2i(m_program, loc, x, y);
	}

	void ShaderProgram::upload(const std::string &uniform, f32 x, f32 y)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform2f(m_program, loc, x, y);
	}

	void ShaderProgram::upload(const std::string &uniform, f64 x, f64 y)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform2d(m_program, loc, x, y);
	}

	void ShaderProgram::upload(const std::string &uniform, bool x, bool y)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform2i(m_program, loc, x ? 1 : 0, y ? 1 : 0);
	}

	void ShaderProgram::upload(const std::string &uniform, u32 x, u32 y, u32 z)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform3ui(m_program, loc, x, y, z);
	}

	void ShaderProgram::upload(const std::string &uniform, i32 x, i32 y, i32 z)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform3i(m_program, loc, x, y, z);
	}

	void ShaderProgram::upload(const std::string &uniform, f32 x, f32 y, f32 z)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform3f(m_program, loc, x, y, z);
	}

	void ShaderProgram::upload(const std::string &uniform, f64 x, f64 y, f64 z)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform3d(m_program, loc, x, y, z);
	}

	void ShaderProgram::upload(const std::string &uniform, bool x, bool y, bool z)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform3i(m_program, loc, x ? 1 : 0, y ? 1 : 0, z ? 1 : 0);
	}

	void ShaderProgram::upload(const std::string &uniform, u32 x, u32 y, u32 z, u32 w)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform4ui(m_program, loc, x, y, z, w);
	}

	void ShaderProgram::upload(const std::string &uniform, i32 x, i32 y, i32 z, i32 w)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform4i(m_program, loc, x, y, z, w);
	}

	void ShaderProgram::upload(const std::string &uniform, f32 x, f32 y, f32 z, f32 w)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform4f(m_program, loc, x, y, z, w);
	}

	void ShaderProgram::upload(const std::string &uniform, f64 x, f64 y, f64 z, f64 w)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform4d(m_program, loc, x, y, z, w);
	}

	void ShaderProgram::upload(const std::string &uniform, bool x, bool y, bool z, bool w)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform4i(m_program, loc, x ? 1 : 0, y ? 1 : 0, z ? 1 : 0, w ? 1 : 0);
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::uvec2 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform2uiv(m_program, loc, 1, glm::value_ptr(v));
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::ivec2 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform2iv(m_program, loc, 1, glm::value_ptr(v));
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::vec2 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform2fv(m_program, loc, 1, glm::value_ptr(v));
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::dvec2 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform2dv(m_program, loc, 1, glm::value_ptr(v));
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::bvec2 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0)
		{
			const std::array arr = { v.x ? 1 : 0, v.y ? 1 : 0 };
			glProgramUniform2iv(m_program, loc, 1, arr.data());
		}
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::uvec3 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform3uiv(m_program, loc, 1, glm::value_ptr(v));
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::ivec3 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform3iv(m_program, loc, 1, glm::value_ptr(v));
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::vec3 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform3fv(m_program, loc, 1, glm::value_ptr(v));
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::dvec3 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform3dv(m_program, loc, 1, glm::value_ptr(v));
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::bvec3 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0)
		{
			const std::array arr = { v.x ? 1 : 0, v.y ? 1 : 0, v.z ? 1 : 0 };
			glProgramUniform3iv(m_program, loc, 1, arr.data());
		}
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::uvec4 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform4uiv(m_program, loc, 1, glm::value_ptr(v));
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::ivec4 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform4iv(m_program, loc, 1, glm::value_ptr(v));
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::vec4 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform4fv(m_program, loc, 1, glm::value_ptr(v));
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::dvec4 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniform4dv(m_program, loc, 1, glm::value_ptr(v));
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::bvec4 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0)
		{
			const std::array arr = { v.x ? 1 : 0, v.y ? 1 : 0, v.z ? 1 : 0, v.w ? 1 : 0 };
			glProgramUniform4iv(m_program, loc, 1, arr.data());
		}
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::mat2 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniformMatrix2fv(m_program, loc, 1, GL_FALSE, glm::value_ptr(v));
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::mat3 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniformMatrix3fv(m_program, loc, 1, GL_FALSE, glm::value_ptr(v));
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::mat4 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0) glProgramUniformMatrix4fv(m_program, loc, 1, GL_FALSE, glm::value_ptr(v));
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::dmat2 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0)
		{
			const glm::mat2 fv {
				static_cast<f32>(v[0].x), static_cast<f32>(v[0].y),
				static_cast<f32>(v[1].x), static_cast<f32>(v[1].y)
			};

			glProgramUniformMatrix2fv(m_program, loc, 1, GL_FALSE, glm::value_ptr(fv));
		}
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::dmat3 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0)
		{
			const glm::mat3 fv {
					static_cast<f32>(v[0].x), static_cast<f32>(v[0].y), static_cast<f32>(v[0].z),
					static_cast<f32>(v[1].x), static_cast<f32>(v[1].y), static_cast<f32>(v[1].z),
					static_cast<f32>(v[2].x), static_cast<f32>(v[2].y), static_cast<f32>(v[2].z)
			};

			glProgramUniformMatrix3fv(m_program, loc, 1, GL_FALSE, glm::value_ptr(fv));
		}
	}

	void ShaderProgram::upload(const std::string &uniform, const glm::dmat4 &v)
	{
		if(auto loc = uniformHandle(uniform); loc >= 0)
		{
			const glm::mat4 fv {
					static_cast<f32>(v[0].x), static_cast<f32>(v[0].y), static_cast<f32>(v[0].z), static_cast<f32>(v[0].w),
					static_cast<f32>(v[1].x), static_cast<f32>(v[1].y), static_cast<f32>(v[1].z), static_cast<f32>(v[1].w),
					static_cast<f32>(v[2].x), static_cast<f32>(v[2].y), static_cast<f32>(v[2].z), static_cast<f32>(v[2].w),
					static_cast<f32>(v[3].x), static_cast<f32>(v[3].y), static_cast<f32>(v[3].z), static_cast<f32>(v[3].w)
			};

			glProgramUniformMatrix4fv(m_program, loc, 1, GL_FALSE, glm::value_ptr(fv));
		}
	}

	void ShaderProgram::bind() const
	{
		s_state->bindShaderProgram(m_program);
	}

	void ShaderProgram::unbind() const
	{
		s_state->bindShaderProgram(0);
	}

	ShaderBuilder &ShaderBuilder::vertex(std::string id)
	{
		m_shaders[ShaderType::Vertex] = std::move(id);
		return *this;
	}

	ShaderBuilder &ShaderBuilder::tessControl(std::string id)
	{
		m_shaders[ShaderType::TessControl] = std::move(id);
		return *this;
	}

	ShaderBuilder &ShaderBuilder::tessEval(std::string id)
	{
		m_shaders[ShaderType::TessEval] = std::move(id);
		return *this;
	}

	ShaderBuilder &ShaderBuilder::geometry(std::string id)
	{
		m_shaders[ShaderType::Geometry] = std::move(id);
		return *this;
	}

	ShaderBuilder &ShaderBuilder::fragment(std::string id)
	{
		m_shaders[ShaderType::Fragment] = std::move(id);
		return *this;
	}

	ShaderBuilder &ShaderBuilder::compute(std::string id)
	{
		m_shaders[ShaderType::Compute] = std::move(id);
		return *this;
	}

	ShaderProgram ShaderBuilder::build()
	{
		return ShaderProgram(m_shaders);
	}

	void bindDefaultVao()
	{
		s_state->bindVao(0);
	}
}
