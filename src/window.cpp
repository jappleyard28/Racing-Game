#include "window.h"

namespace game
{
	Window::Window(u32 width, u32 height, std::string title, bool fullscreen)
		: m_title(std::move(title)),
		  m_fullscreen(fullscreen)
	{
		glfwDefaultWindowHints();

		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);

		GLFWmonitor *monitor = nullptr;

		if(fullscreen)
		{
			monitor = glfwGetPrimaryMonitor();

			const auto *mode = glfwGetVideoMode(monitor);

			m_windowWidth = width;
			m_windowHeight = height;

			m_posX = (mode->width - m_width) / 2;
			m_posY = (mode->height - m_height) / 2;

			m_width = mode->width;
			m_height = mode->height;
		}

		m_window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), m_title.c_str(), monitor, nullptr);

		if(!m_window) throw std::runtime_error("failed to create window");

		glfwSetWindowUserPointer(m_window, this);

		int w, h;
		glfwGetFramebufferSize(m_window, &w, &h);

		m_width = static_cast<u32>(w);
		m_height = static_cast<u32>(h);

		glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow *window, i32 width, i32 height)
		{
			auto *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

			w->m_width = width;
			w->m_height = height;

			w->m_requiresResize = true;
		});

		glfwSetKeyCallback(m_window, [](GLFWwindow *window, i32 key, i32 scancode, i32 action, i32 mods)
		{
			if(action != GLFW_REPEAT)
			{
				auto *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

				bool pressed = action == GLFW_PRESS;

				bool ctrl = mods & GLFW_MOD_CONTROL;
				bool alt = mods & GLFW_MOD_ALT;
				bool shift = mods & GLFW_MOD_SHIFT;

				if(pressed)
				{
					switch(key)
					{
						case GLFW_KEY_F9:
							w->requestClose();
							break;

						case GLFW_KEY_ENTER:
							if(alt) w->toggleFullscreen();
							break;
					}
				}

				for(const auto &callback : w->m_keyCallbacks)
				{
					callback(key, pressed, ctrl, alt, shift);
				}
			}
		});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow *window, i32 button, i32 action, i32 mods)
		{
			auto *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

			bool pressed = action == GLFW_PRESS;

			bool ctrl = mods & GLFW_MOD_CONTROL;
			bool alt = mods & GLFW_MOD_ALT;
			bool shift = mods & GLFW_MOD_SHIFT;

			for(const auto &callback : w->m_mouseButtonCallbacks)
			{
				callback(button, pressed, ctrl, alt, shift);
			}
		});

		glfwMakeContextCurrent(m_window);
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_window);
	}

	void Window::title(std::string title)
	{
		m_title = std::move(title);
		glfwSetWindowTitle(m_window, m_title.c_str());
	}

	void Window::requestClose()
	{
		m_shouldClose.store(true, std::memory_order_release);
	}

	bool Window::shouldClose() const
	{
		return m_shouldClose.load(std::memory_order_acquire);
	}

	void Window::update()
	{
		glfwSwapBuffers(m_window);
		glfwPollEvents();

		if(m_requiresResize)
		{
			m_requiresResize = false;
			if(m_resizeCallback) m_resizeCallback(m_width, m_height);
		}

		if(m_requiresSwapIntervalUpdate)
		{
			m_requiresSwapIntervalUpdate = false;
			glfwSwapInterval(static_cast<int>(m_swapInterval));
		}

		if(glfwWindowShouldClose(m_window)) m_shouldClose.store(true, std::memory_order_release);
	}

	void Window::resizeCallback(ResizeCallback callback)
	{
		m_resizeCallback = std::move(callback);
	}

	void Window::addKeyCallback(KeyCallback callback)
	{
		m_keyCallbacks.push_back(std::move(callback));
	}

	void Window::addMouseButtonCallback(MouseButtonCallback callback)
	{
		m_mouseButtonCallbacks.push_back(std::move(callback));
	}

	void Window::toggleFullscreen()
	{
		if(m_fullscreen)
		{
			glfwSetWindowMonitor(m_window, nullptr, static_cast<int>(m_posX), static_cast<int>(m_posY), static_cast<int>(m_windowWidth), static_cast<int>(m_windowHeight), GLFW_DONT_CARE);
			m_fullscreen = false;
		}
		else
		{
			auto *monitor = glfwGetPrimaryMonitor();

			const auto *mode = glfwGetVideoMode(monitor);

			i32 x, y;
			glfwGetWindowPos(m_window, &x, &y);

			m_posX = x;
			m_posY = y;

			m_windowWidth = m_width;
			m_windowHeight = m_height;

			glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

			m_fullscreen = true;
		}

		m_requiresSwapIntervalUpdate = true;
	}

	void Window::swapInterval(u32 swapInterval)
	{
		m_swapInterval = swapInterval;
		m_requiresSwapIntervalUpdate = true;
	}
}
