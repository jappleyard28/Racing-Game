#include <iostream>
#include "glw.h"
#include "window.h"
#include "render.h"
#include "input.h"
#include "game.h"
#include <atomic>
#include <thread>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <memory>

namespace game
{
	namespace
	{
		struct GlfwInitGuard
		{
			GlfwInitGuard() : m_success(glfwInit()) {}

			~GlfwInitGuard()
			{
				if(m_success) glfwTerminate();
			}

			bool m_success;
		};

		struct GameDataGuard
		{
			GameDataGuard()
			{
				loadGameData();
			}

			~GameDataGuard()
			{
				destroyGameData();
			}
		};

		// changing will wreck npc ai
		constexpr f64 TicksPerSecond = 64.0;
		constexpr f64 TickLength = 1.0 / TicksPerSecond;

		class TickThread
		{
		public:
			explicit TickThread(World &world)
				: m_world(world),
				  m_thread([this]() { run(); }) {}

			~TickThread()
			{
				if(m_thread.joinable()) stop();
			}

			[[nodiscard]] inline auto lastTick() const { return m_lastTick.load(std::memory_order_acquire); }

			void startCountingTicks()
			{
				m_countTicks.store(true, std::memory_order_release);
			}

			void stop()
			{
				m_stop.store(true, std::memory_order_release);
				m_thread.join();
			}

		private:
			World &m_world;
			std::atomic_bool m_stop { false };
			std::atomic_bool m_countTicks { false };
			std::atomic<f64> m_lastTick {};
			std::thread m_thread;

			void run()
			{
				auto time = glfwGetTime();
				auto prevTime = time;

				u64 ticks = 0;

				while(!m_stop.load(std::memory_order_acquire))
				{
					//const auto delta = time - prevTime;
					prevTime = time;
					time = glfwGetTime();

					const auto targetTime = time + TickLength;

					m_world.tick(TickLength, ticks);
					if(m_countTicks.load(std::memory_order_acquire)) ticks++;

					const auto tickTime = glfwGetTime() - time;

					if(tickTime > TickLength) std::cerr << "tick took " << (std::round(tickTime * 100000.0) / 100.0) << " ms, should take max " << (std::round(TickLength * 100000.0) / 100.0) << " ms" << std::endl;
					else if(tickTime < TickLength)
					{
						// Subtract one millisecond to avoid
						// oversleeping due to scheduler accuracy
						auto ms = static_cast<u64>(std::floor(std::max(1.0, (TickLength - tickTime) * 1000.0)) - 1.0);
						std::this_thread::sleep_for(std::chrono::milliseconds(ms));

						// busy sleep for the rest
						while(glfwGetTime() < targetTime) {}
					}

					m_lastTick.store(time, std::memory_order_release);
				}
			}
		};
	}

	int start()
	{
		GlfwInitGuard glfwInitGuard;

		if(!glfwInitGuard.m_success)
		{
			std::cerr << "failed to initialise GLFW" << std::endl;
			return 1;
		}

		Window window(1366, 768, "Racing game");

		if(!loadGl(window))
		{
			std::cerr << "failed to load OpenGL" << std::endl;
			return 2;
		}

		input::init(window); // initialises stuff that uses the window

		Renderer renderer(window, 30.0F);

		GameDataGuard gameDataGuard;

		World world;

		TickThread tickThread(world);

		auto player = std::make_shared<PlayerCar>();

		world.addEntity(std::make_shared<Track>());
		world.addEntity(player);
		world.addEntity(std::make_shared<NpcCar>(0));
		world.addEntity(std::make_shared<NpcCar>(1));
		world.addEntity(std::make_shared<NpcCar>(2));

		tickThread.startCountingTicks();

		while(!window.shouldClose())
		{
			const auto time = glfwGetTime();
			const auto partialTick = (time - tickThread.lastTick()) * TicksPerSecond - 1.0;

			renderer.beginFrame(player->updateFramePosition(partialTick));

			world.render(renderer, partialTick);

			renderer.endFrame();
			window.update();
		}

		tickThread.stop();

		return 0;
	}
}
