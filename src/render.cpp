#define GLAD_GL_IMPLEMENTATION
#include "render.h"
#include <iostream>
#include <array>
#include <glm/gtx/transform.hpp>
#include <cmath>
#include "config.h"

namespace game
{
	Renderer::Renderer(Window &window, f64 zoom)
		: m_window(window),
		  m_emptyTexture(1, 1, gl::TextureFormat::Rgba8un),
		  m_multisampledFramebuffer(config::MsaaSamples > 1 ? std::make_unique<gl::Framebuffer>(window.width(), window.height(), std::vector { gl::TextureFormat::Rgba8un }, config::MsaaSamples) : nullptr),
		  m_framebuffer(window.width(), window.height(), { gl::TextureFormat::Rgba8un }),
		  m_size { window.width(), window.height() },
		  m_quadShader(gl::ShaderBuilder {}.vertex("quad").fragment("default").build()),
		  m_finalShader(gl::ShaderBuilder {}.vertex("framebuffer").fragment("final").build()),
		  m_projection(glm::ortho(static_cast<f64>(window.width()) / (zoom * -2.0), static_cast<f64>(window.width()) / (zoom * 2.0),
							static_cast<f64>(window.height()) / (zoom * -2.0), static_cast<f64>(window.height()) / (zoom * 2.0)))
	{
		window.resizeCallback([this](u32 width, u32 height) { resize(width, height); });

		std::array white { 1.0F, 1.0F, 1.0F, 1.0F };
		glTextureSubImage2D(m_emptyTexture.handle(), 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, &white);

		glViewport(0, 0, static_cast<GLsizei>(window.width()), static_cast<GLsizei>(window.height()));

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// grass colour
		// raise to power of 2.2 to convert to linear colour
		//glClearColor(static_cast<f32>(std::pow(43.0 / 255.0, 2.2)), static_cast<f32>(std::pow(126.0 / 255.0, 2.2)), 0.0F, 1.0F);
		glClearColor(static_cast<f32>(std::pow(43.0 / 255.0, 2.2)), static_cast<f32>(std::pow(126.0 / 255.0, 2.2)), 0.0F, 1.0F);
	}

	void Renderer::beginFrame(glm::dvec2 cameraPos)
	{
		if constexpr(config::MsaaSamples > 1) m_multisampledFramebuffer->bind();
		else m_framebuffer.bind();

		glClear(GL_COLOR_BUFFER_BIT);

		glm::dmat4 view = glm::translate(glm::dmat4 { 1.0 }, { -cameraPos, 0.0 });
		m_vp = m_projection * view;

		glEnable(GL_BLEND);
	}

	void Renderer::drawQuad(const RenderableQuad &quad)
	{
		auto model = glm::dmat4 { 1.0 };

		model = glm::translate(model, { quad.m_position, 0.0 });
		model = glm::rotate(model, quad.m_rotation, { 0.0, 0.0, 1.0 });
		model = glm::scale(model, { quad.m_scale, 1.0 });

		auto &shader = quad.m_shaderOverride ? *quad.m_shaderOverride : m_quadShader;

		shader.bind();

		shader.upload("mvp", m_vp * model);
		shader.upload("tint", quad.m_tint);

		const auto &texture = quad.m_textureOverride ? *quad.m_textureOverride : m_emptyTexture;

		texture.bind(0);

		gl::bindDefaultVao();
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void Renderer::endFrame()
	{
		glDisable(GL_BLEND);

		if constexpr(config::MsaaSamples > 1)
		{
			m_multisampledFramebuffer->unbind();
			m_multisampledFramebuffer->blitColourTo(m_framebuffer);
		}
		else m_framebuffer.unbind();

		m_framebuffer.draw(m_finalShader);
	}

	void Renderer::resize(u32 width, u32 height)
	{
		if(width != m_size.x || height != m_size.y)
		{
			if constexpr(config::MsaaSamples > 1) m_multisampledFramebuffer->resize(width, height);
			m_framebuffer.resize(width, height);

			glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

			m_size = { width, height };

			std::cout << "Resized to " << width << 'x' << height << std::endl;
		}
	}

	bool loadGl(Window &window)
	{
		if(!gladLoadGL(glfwGetProcAddress)) return false;

		glfwMakeContextCurrent(window.handle());

#ifndef NDEBUG
		glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *user)
		{
			static const std::array types { "ERROR", "DEPRECATED_BEHAVIOR", "UNDEFINED_BEHAVIOR", "PORTABILITY", "PERFORMANCE", "OTHER", "MARKER" };
			static const std::array severities { "HIGH", "MEDIUM", "LOW" };

			if(severity == GL_DEBUG_SEVERITY_NOTIFICATION || type == GL_DEBUG_TYPE_PUSH_GROUP || type == GL_DEBUG_TYPE_POP_GROUP) return;

			(type == GL_DEBUG_TYPE_ERROR ? std::cerr : std::cout) << "[OpenGL] type: " << types[type - GL_DEBUG_TYPE_ERROR]
					<< ", severity: " << severities[severity - GL_DEBUG_SEVERITY_HIGH] << ", message: " << message << std::endl;
		}, nullptr);

		glEnable(GL_DEBUG_OUTPUT);
#endif

		return true;
	}
}
