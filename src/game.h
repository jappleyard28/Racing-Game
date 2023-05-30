#pragma once

#include "types.h"
#include "util.h"
#include "render.h"
#include "input.h"
#include <vector>
#include <shared_mutex>
#include <memory>
#include <array>

namespace game
{
	void loadGameData();
	void destroyGameData();

	class World;

	class Entity
	{
	public:
		Entity() = default;
		virtual ~Entity() = default;

		virtual void tick(World &world, f64 delta, u64 tick) = 0;
		virtual void render(Renderer &renderer, f64 partialTick) = 0;

		[[nodiscard]] virtual bool intersects(const util::OrientedBoundingBox &hitbox) const = 0;
	};

	class Track : public Entity
	{
	public:
		Track();
		~Track() override = default;

		void tick(World &world, f64 delta, u64 tick) override;
		void render(Renderer &renderer, f64 partialTick) override;

		[[nodiscard]] bool intersects(const util::OrientedBoundingBox &hitbox) const override;

	private:
		RenderableQuad m_sprite {};
		std::vector<util::OrientedBoundingBox> m_hitboxes;
	};

	class Car : public Entity
	{
	public:
		Car();
		~Car() override = default;

		static constexpr size_t Accelerate = 0;
		static constexpr size_t Reverse = 1;
		static constexpr size_t Brake = 2;
		static constexpr size_t Left = 3;
		static constexpr size_t Right = 4;
		static constexpr size_t Handbrake = 5;

		virtual void updateInputs(u64 tick) = 0;

		void tick(World &world, f64 delta, u64 tick) final;

		virtual void winRace() {}
		virtual void loseRace() {}

		[[nodiscard]] bool intersects(const util::OrientedBoundingBox &hitbox) const override;

	protected:
		std::array<bool, 6> m_inputs { false, false, false, false, false, false };

		bool m_inStartLine = false;
		f64 m_lastLapTime = -1.0;

		RenderableQuad m_sprite {};
		std::mutex m_updateLock {};

		glm::dvec2 m_position { 0.0 };
		f64 m_rotation = util::toRad(90.0);

		glm::dvec2 m_prevPosition { m_position };
		f64 m_prevRotation { m_rotation };

		glm::dvec2 m_velocity { 0.0 };

		glm::dvec2 m_localAccel { 0.0 };
		f64 m_absoluteVelocity = 0.0;

		f64 m_yawRate = 0.0;

		util::OrientedBoundingBox m_hitbox {};

		u32 m_laps = 0;
		f64 m_lapStartTime;

		void startLap();
		void endLap();

		[[nodiscard]] bool colliding(const World &world);
	};

	class PlayerCar : public Car
	{
	public:
		PlayerCar();
		~PlayerCar() override = default;

		void updateInputs(u64 tick) override;
		void render(Renderer &renderer, f64 partialTick) override;

		[[nodiscard]] glm::dvec2 updateFramePosition(f64 partialTick);

		void winRace() override;
		void loseRace() override;

	private:
		const input::Key &m_accelerateKey, &m_reverseKey, &m_brakeKey, &m_leftKey, &m_rightKey, &m_handbrakeKey;
	};

	class NpcCar : public Car
	{
	public:
		explicit NpcCar(u32 index);
		~NpcCar() override = default;

		[[nodiscard]] inline auto index() const { return m_index; }

		void updateInputs(u64 tick) override;
		void render(Renderer &renderer, f64 partialTick) override;

	private:
		u32 m_index;
	};

	class World
	{
	public:
		World() = default;
		~World() = default;

		void tick(f64 delta, u64 tick);
		void render(Renderer &renderer, f64 partialTick);

		void addEntity(std::shared_ptr<Entity> entity);
		void removeEntity(const std::shared_ptr<Entity> &entity);

		[[nodiscard]] inline auto &entities() const { return m_entities; }

		World(const World &) = delete;
		World(World &&) = delete;

		World &operator=(const World &) = delete;
		World &operator=(World &&) = delete;

	private:
		std::shared_mutex m_entityLock;
		std::vector<std::shared_ptr<Entity>> m_entities;
	};
}
