#pragma once

#include "types.h"
#include <atomic>
#include <GLFW/glfw3.h>
#include <functional>
#include <vector>

namespace game
{
	using ResizeCallback = std::function<void(u32 /* width */, u32 /* height */)>;

	using KeyCallback = std::function<void(i32 /* key */, bool /* pressed */, bool /* ctrl */, bool /* alt */, bool /* shift */)>;
	using MouseButtonCallback = std::function<void(i32 /* button */, bool /* pressed */, bool /* ctrl */, bool /* alt */, bool /* shift */)>;

	class Window
	{
	public:
		Window(u32 width, u32 height, std::string title, bool fullscreen = false);
		~Window();

		void title(std::string title);
		[[nodiscard]] inline auto &title() const { return m_title; }

		void requestClose();
		[[nodiscard]] bool shouldClose() const;

		void update();

		void resizeCallback(ResizeCallback callback);

		void addKeyCallback(KeyCallback callback);
		void addMouseButtonCallback(MouseButtonCallback callback);

		void toggleFullscreen();

		void swapInterval(u32 swapInterval);

		[[nodiscard]] inline auto width() const { return m_width; }
		[[nodiscard]] inline auto height() const { return m_height; }

		[[nodiscard]] inline auto *handle() const { return m_window; }

		Window(const Window &) = delete;
		Window(Window &&) = delete;

		Window &operator=(const Window &) = delete;
		Window &operator=(Window &&) = delete;

	private:
		GLFWwindow *m_window;

		std::string m_title;

		u32 m_width, m_height;
		u32 m_windowWidth, m_windowHeight;
		u32 m_posX, m_posY;

		bool m_requiresResize = false;

		u32 m_swapInterval = 1;
		bool m_requiresSwapIntervalUpdate = true;

		bool m_fullscreen;

		std::atomic<bool> m_shouldClose = false;

		ResizeCallback m_resizeCallback = nullptr;

		std::vector<KeyCallback> m_keyCallbacks {};
		std::vector<MouseButtonCallback> m_mouseButtonCallbacks {};
	};
}
