#pragma once

#include "types.h"
#include "glw.h"
#include "window.h"
#include <memory>

namespace game
{
	struct RenderableQuad
	{
		glm::dvec2 m_position { 0.0 };
		f64 m_rotation = 0.0;
		glm::dvec2 m_scale { 1.0, 1.0 };
		glm::vec3 m_tint = { 1.0F, 1.0F, 1.0F };
		gl::ShaderProgram *m_shaderOverride { nullptr };
		gl::SingleTexture *m_textureOverride { nullptr };
	};

	struct GlStateGuard
	{
		GlStateGuard()
		{
			gl::initState();
		}

		~GlStateGuard()
		{
			gl::destroyState();
		}
	};

	class Renderer
	{
	public:
		explicit Renderer(Window &window, f64 zoom = 1.0);
		~Renderer() = default;

		void beginFrame(glm::dvec2 camPos);

		void drawQuad(const RenderableQuad &quad);

		void endFrame();

		void resize(u32 width, u32 height);

		[[nodiscard]] inline auto &window() { return m_window; }

		Renderer(const Renderer &) = delete;
		Renderer(Renderer &&) = delete;

		Renderer &operator=(const Renderer &) = delete;
		Renderer &operator=(Renderer &&) = delete;

	private:
		[[maybe_unused]] GlStateGuard m_glStateGuard {};

		Window &m_window;

		gl::SingleTexture m_emptyTexture;
		std::unique_ptr<gl::Framebuffer> m_multisampledFramebuffer;
		gl::Framebuffer m_framebuffer;
		glm::uvec2 m_size;
		gl::ShaderProgram m_quadShader, m_finalShader;

		glm::dmat4 m_projection;
		glm::dmat4 m_vp {};
	};

	bool loadGl(Window &window);
}
