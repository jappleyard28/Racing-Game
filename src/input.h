#pragma once

#include "types.h"
#include <atomic>
#include "window.h"
#include <GLFW/glfw3.h>

namespace game::input
{
	void init(Window &window);

	class Key
	{
	public:
		using Callback = std::function<void(bool)>;

		explicit Key(Callback callback = nullptr);
		~Key() = default;

		void down(bool down);
		[[nodiscard]] inline auto down() const { return m_down.load(std::memory_order_acquire); }

		Key(const Key &) = delete;
		Key(Window &&) = delete;

		Key &operator=(const Key &) = delete;
		Key &operator=(Key &&) = delete;

	private:
		std::atomic<bool> m_down { false };
		Callback m_callback = nullptr;
	};

	const Key &key(i32 glfwKey, Key::Callback callback = nullptr);
}
