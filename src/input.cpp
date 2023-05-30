#include "input.h"
#include <unordered_map>

namespace game::input
{
	namespace
	{
		std::unordered_map<i32, Key> s_keys;
	}

	void init(Window &window)
	{
		window.addKeyCallback([](i32 key, bool pressed, bool ctrl, bool alt, bool shift)
		{
			if(auto iter = s_keys.find(key); iter != s_keys.end()) iter->second.down(pressed);
		});
	}

	Key::Key(Callback callback) : m_callback(std::move(callback)) {}

	void Key::down(bool down)
	{
		m_down.store(down, std::memory_order_release);
		if(m_callback) m_callback(down);
	}

	const Key &key(i32 glfwKey, Key::Callback callback)
	{
		return s_keys.emplace(glfwKey, std::move(callback)).first->second;
	}
}
