#include "game.h"
#include <cmath>
#include <algorithm>
#include "config.h"
#include <iostream>
#include "assets.h"
#include <sstream>
#include <tuple>

namespace game
{
	namespace
	{
		namespace physics
		{
			constexpr auto Gravity = 9.80665;
			constexpr auto AirResistance = 2.5;
			constexpr auto BounceFactor = -0.6;

			namespace car
			{
				constexpr auto Mass = 1200.0;
				constexpr auto InertiaScale = 1.0;
				constexpr auto CentreOfGravityToFrontAxle = 1.25;
				constexpr auto CentreOfGravityToRearAxle = 1.25;
				constexpr auto CentreOfGravityToGround = 0.55;
				constexpr auto TyreGrip = 2.0;
				constexpr auto LockGrip = 0.7;
				constexpr auto EngineForce = 8000.0;
				constexpr auto EngineReverseScale = 0.6;
				constexpr auto BrakeForce = 12000.0;
				constexpr auto HandbrakeForce = 4800.0;
				constexpr auto WeightTransfer = 0.2;
				constexpr auto CornerStiffnessFront = 5.0;
				constexpr auto CornerStiffnessRear = 5.2;
				constexpr auto RollResistance = 8.0;

				constexpr auto Inertia = Mass * InertiaScale;
				constexpr auto WheelBase = CentreOfGravityToFrontAxle + CentreOfGravityToRearAxle;
				constexpr auto AxleLoadRatioFront = CentreOfGravityToRearAxle / WheelBase;
				constexpr auto AxleLoadRatioRear = CentreOfGravityToFrontAxle / WheelBase;
				constexpr auto EngineReverseForce = -EngineForce * EngineReverseScale;
			}
		}

		void drawHitbox(Renderer &renderer, const util::OrientedBoundingBox &hitbox, const glm::vec3 tint = { 0.0F, 1.0F, 0.0F })
		{
			RenderableQuad hitboxSprite {};

			hitboxSprite.m_position = hitbox.m_position;
			hitboxSprite.m_rotation = hitbox.m_rotation;
			hitboxSprite.m_scale = hitbox.m_size;
			hitboxSprite.m_tint = tint;

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			renderer.drawQuad(hitboxSprite);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		constexpr auto DrawHitboxes = false; //change to make the hitboxes appear

		constexpr glm::dvec2 PlayerStartPos { -28.7, 2.0 };
		constexpr std::array NpcStartPos { glm::dvec2 { -26.5, 2.0 }, glm::dvec2 { -28.7, -1.0 }, glm::dvec2 { -26.5, -1.0 } };

		const std::vector<std::tuple<u64, size_t, bool>> NpcInputs {
			{ 0, Car::Accelerate, true },
			{ 96, Car::Accelerate, false },
			{ 96, Car::Brake, true },
			{ 128, Car::Brake, false},
			{ 128, Car::Right, true },
			{ 187, Car::Right, false },
			{ 187, Car::Accelerate, true },
			{ 338, Car::Accelerate, false },
			{ 340, Car::Brake, true },
			{ 372, Car::Brake, false },
		};

		/*
		const std::vector<glm::dvec3> NpcWaypoints {
			{ 2.0, -27.0, 40.0 / 3.0 },
			{ 3.0, 27.1, 40.0 / 3.0 },
			{ 1.0, 27.1, 20.5 / 3.0 },
			{ 2.5, -59.3 / 3.0, 20.5 / 3.0 },
			{ 1.0, -59.3 / 3.0, -0.95 / 3.0 },
			{ 2.5, 79.6 / 3.0, -0.95 / 3.0 },
			{ 1.0, 79.6 / 3.0, -7.0 },
			{ 3.0, -27.0, -7.0 },
			{ 2.0, -27.0, 8.8 }
		};
		 */

		//const std::array InputNames { "accelerate", "reverse", "brake", "left", "right", "handbrake" };

		struct GameData
		{
			GameData()
				: m_trackTexture("track"),
				  m_playerCarTexture("car"),
				  m_npcCarTexture("car2"),
				  m_startingLineHitbox({ -27.75, 15.35 / 3.0 }, 0.0, { 6.0, 3.0 }) {}

			~GameData() = default;

			void drawStartLineHitbox(Renderer &renderer) const
			{
				drawHitbox(renderer, m_startingLineHitbox, { 1.0, 0.0, 1.0 });
			}

			//this is called when a car finishes a lap
			void finishLap(Car &car, f64 time)
			{
				// this is called with -1 as the time the first time cars pass the start line
				if(time < 0.0) return;

				std::cout << "car finished with time " << time << " seconds" << std::endl;

				if(time < m_fastestTime)
				{
					if(m_fastestCar) m_slowerCars.push_back(m_fastestCar);

					m_fastestCar = &car;
					m_fastestTime = time;

					std::cout << "^ fastest time" << std::endl;
				}

				// 4 cars in the race
				if(++m_finishedCars == 4)
				{
					m_fastestCar->winRace();

					for(auto *slowCar : m_slowerCars)
					{
						slowCar->loseRace();
					}
				}
			}

			gl::SingleTexture m_trackTexture, m_playerCarTexture, m_npcCarTexture;
			util::OrientedBoundingBox m_startingLineHitbox;

			u32 m_finishedCars = 0;

			Car *m_fastestCar = nullptr;
			std::vector<Car *> m_slowerCars;
			f64 m_fastestTime = INFINITY;
		};

		std::unique_ptr<GameData> s_data { nullptr };
	}

	void loadGameData()
	{
		s_data = std::make_unique<GameData>();
	}

	void destroyGameData()
	{
		s_data.reset();
	}

	Track::Track()
	{
		m_sprite.m_scale = { 64.0, 36.0 };
		m_sprite.m_textureOverride = &s_data->m_trackTexture;

		auto imageData = assets::loadImage("collide");

		//takes collide.png and generates hitboxes on the track based on it
		if(imageData)
		{
			u32 x = 0;
			u32 y = 0;

			std::vector<bool> mergedMask;
			mergedMask.resize(imageData->m_width * imageData->m_height);

			while(true)
			{
				glm::uvec2 size { 1, 1 };

				// if the pixel is opaque and not already in a different hitbox,
				// start trying to create a hitbox
				if((imageData->pixelAt(x, y) >> 24) != 0 && !mergedMask[x + y * imageData->m_width])
				{
					glm::uvec2 pos { x, y };

					// loop through all rows
					for(u32 ny = y; ny < imageData->m_height; ++ny)
					{
						u32 currentWidth = 0;

						// loop through all pixels on the row
						for(u32 nx = x; nx < imageData->m_width; ++nx)
						{
							// if the pixel is opaque and not already in a different hitbox,
							// increment the number of colliding pixels on this row
							if((imageData->pixelAt(nx, ny) >> 24) != 0 && !mergedMask[nx + ny * imageData->m_width]) ++currentWidth;
							else break;
						}

						// if this is the first row, then it is the max width of this hitbox
						if(ny == y) size.x = currentWidth;
						// otherwise, continue downwards if the row is long enough
						else if(currentWidth >= size.x) ++size.y;
						// otherwise the hitbox is as big as it can be
						else break;
					}

					// mark all pixels we've added
					for(u32 nx = pos.x; nx < pos.x + size.x; ++nx)
					{
						for(u32 ny = pos.y; ny < pos.y + size.y; ++ny)
						{
							mergedMask[nx + ny * imageData->m_width] = true;
						}
					}

					// transform the hitbox into world coordinates
					glm::dvec2 hitboxCentre { ((pos.x + size.x / 2.0) * 16.0 - 1920.0) / 60.0, ((pos.y + size.y / 2.0) * 16.0 - 1080.0) / -60.0 };
					glm::dvec2 hitboxSize { static_cast<f64>(size.x) / 3.75, static_cast<f64>(size.y) / 3.75 };
					m_hitboxes.emplace_back(hitboxCentre, 0.0, hitboxSize);
				}

				// move right by the size of the line (or 1), continuing to the next
				// line if we're at the end of this row or stopping if we're at the
				// bottom of the image
				if((x += size.x) >= imageData->m_width)
				{
					x = 0;
					if(++y >= imageData->m_height) break;
				}
			}
		}
		else std::cerr << "Failed to load track collision" << std::endl;
	}

	void Track::tick(World &world, f64 delta, u64 tick)
	{
		//
	}
 //draws track hitboxes as well as track if it's enabled
	void Track::render(Renderer &renderer, f64 partialTick)
	{
		renderer.drawQuad(m_sprite);

		if constexpr(DrawHitboxes)
		{
			for(const auto &hitbox : m_hitboxes)
			{
				drawHitbox(renderer, hitbox);
			}
		}
	}

	// returns true if the hitbox collides with the track
	bool Track::intersects(const util::OrientedBoundingBox &hitbox) const
	{
		return std::any_of(std::cbegin(m_hitboxes), std::cend(m_hitboxes), [&hitbox](const util::OrientedBoundingBox &box) { return hitbox.intersects(box); });
	}

	Car::Car()
	{
		m_hitbox.m_size = { 2.1, 1.3 };
		m_hitbox.m_rotation = m_rotation;

		m_sprite.m_scale = { 3.0, 1.6 };
	}

	void Car::tick(World &world, f64 delta, u64 tick)
	{
		updateInputs(tick);

		const auto steerAmount = 1.0 - std::min(m_absoluteVelocity, 250.0) / 280.0;

		auto steer = 0.0;

		if(m_inputs[Left]) steer += steerAmount;
		if(m_inputs[Right]) steer -= steerAmount;

		{
			std::unique_lock lock(m_updateLock);

			m_prevPosition = m_position;
			m_prevRotation = m_rotation;

			// Marco Monster's car physics model, with reversing
			const auto s = std::sin(m_rotation);
			const auto c = std::cos(m_rotation);

			const glm::dvec2 localVelocity { c * m_velocity.x + s * m_velocity.y, c * m_velocity.y - s * m_velocity.x };

			const auto axleLoadFront = physics::car::Mass * (physics::car::AxleLoadRatioFront * physics::Gravity - physics::car::WeightTransfer * m_localAccel.x * physics::car::CentreOfGravityToGround / physics::car::WheelBase);
			const auto axleLoadRear = physics::car::Mass * (physics::car::AxleLoadRatioRear * physics::Gravity + physics::car::WeightTransfer * m_localAccel.x * physics::car::CentreOfGravityToGround / physics::car::WheelBase);

			const auto yawSpeedFront = physics::car::CentreOfGravityToFrontAxle * m_yawRate;
			const auto yawSpeedRear = -physics::car::CentreOfGravityToRearAxle * m_yawRate;

			const auto alphaFront = std::atan2(localVelocity.y + yawSpeedFront, std::abs(localVelocity.x)) - util::sign(localVelocity.x) * steer;
			const auto alphaRear = std::atan2(localVelocity.y + yawSpeedRear, std::abs(localVelocity.x));

			const auto tyreGripFront = physics::car::TyreGrip;
			const auto tyreGripRear = physics::car::TyreGrip * (m_inputs[Handbrake] ? physics::car::LockGrip : 1.0);

			const auto frictionForceFront = std::clamp(-physics::car::CornerStiffnessFront * alphaFront, -tyreGripFront, tyreGripFront) * axleLoadFront;
			const auto frictionForceRear = std::clamp(-physics::car::CornerStiffnessRear * alphaRear, -tyreGripRear, tyreGripRear) * axleLoadRear;

			const auto brakeForce = std::min((m_inputs[Brake] ? physics::car::BrakeForce : 0.0) + (m_inputs[Handbrake] ? physics::car::HandbrakeForce : 0.0), physics::car::BrakeForce);
			const auto throttle = (m_inputs[Accelerate] ? physics::car::EngineForce : 0.0) + (m_inputs[Reverse] ? physics::car::EngineReverseForce : 0.0);

			const auto tractionForceX = throttle - brakeForce * util::sign(localVelocity.x);
			const auto tractionForceY = 0.0;

			const auto dragForceX = -physics::car::RollResistance * localVelocity.x - physics::AirResistance * localVelocity.x * std::abs(localVelocity.x);
			const auto dragForceY = -physics::car::RollResistance * localVelocity.y - physics::AirResistance * localVelocity.y * std::abs(localVelocity.y);

			const auto totalForceX = dragForceX + tractionForceX;
			const auto totalForceY = dragForceY + tractionForceY * std::cos(steer) * frictionForceFront + frictionForceRear;

			m_localAccel.x = totalForceX / physics::car::Mass;
			m_localAccel.y = totalForceY / physics::car::Mass;

			const glm::dvec2 accel { c * m_localAccel.x - s * m_localAccel.y, s * m_localAccel.x + c * m_localAccel.y };

			// delta = 1/64
			m_velocity += accel * delta;

			m_absoluteVelocity = glm::length(m_velocity);

			f64 angularTorque;

			if(m_absoluteVelocity < 0.5 && throttle == 0.0)
			{
				m_velocity = { 0.0, 0.0 };
				m_absoluteVelocity = angularTorque = m_yawRate = 0.0;
			}
			else angularTorque = (frictionForceFront + tractionForceY) * physics::car::CentreOfGravityToFrontAxle - frictionForceRear * physics::car::CentreOfGravityToRearAxle;

			const auto angularAccel = angularTorque / physics::car::Inertia;

			m_yawRate += angularAccel * delta;

			m_hitbox.m_position += m_velocity * delta;
			m_hitbox.m_rotation += m_yawRate * delta;

			//

			// if it collides, reverse the direction and slow the car down a bit by BounceFactor amount
			if(colliding(world))
			{
				m_velocity *= physics::BounceFactor; // velocity
				m_yawRate *= physics::BounceFactor; // rotation speed

				m_hitbox.m_position += m_velocity * delta;
			}

			// sets the car to the new position after it collides
			m_rotation = m_hitbox.m_rotation;
			m_position = m_hitbox.m_position;

			const auto inStartLine = intersects(s_data->m_startingLineHitbox);

			// if it has either left or entered the starting line then start or end the lap depending on whether we entered or left the starting line
			if(inStartLine != m_inStartLine)
			{
				if(inStartLine) endLap();
				else startLap();
			}

			m_inStartLine = inStartLine;
		}
	}

	// return true if the car collides with the inputted hitbox
	bool Car::intersects(const util::OrientedBoundingBox &hitbox) const
	{
		return hitbox.intersects(m_hitbox);
	}

	// return true if the car collides with anything
	bool Car::colliding(const World &world)
	{
		return std::any_of(std::cbegin(world.entities()), std::cend(world.entities()),
					 [this](const std::shared_ptr<Entity> &entity) { return entity.get() != this && entity->intersects(m_hitbox); });
	}

	void Car::startLap()
	{
		m_lapStartTime = glfwGetTime();
	}

	void Car::endLap()
	{
		f64 lapTime = m_laps++ == 0 ? -1.0 : glfwGetTime() - m_lapStartTime;
		m_lastLapTime = lapTime;
		s_data->finishLap(*this, lapTime);
	}

	PlayerCar::PlayerCar()
		: m_accelerateKey(input::key(config::KeyAccelerate)),
		  m_reverseKey(input::key(config::KeyReverse)),
		  m_brakeKey(input::key(config::KeyBrake)),
		  m_leftKey(input::key(config::KeySteerLeft)),
		  m_rightKey(input::key(config::KeySteerRight)),
		  m_handbrakeKey(input::key(config::KeyHandbrake))
	{
		m_prevPosition = m_position = m_hitbox.m_position = m_sprite.m_position = PlayerStartPos;
		m_sprite.m_textureOverride = &s_data->m_playerCarTexture; //gives the sprite the car texture
	}

	void PlayerCar::updateInputs(u64 tick)
	{
		m_inputs[Accelerate] = m_accelerateKey.down();
		m_inputs[Reverse] = m_reverseKey.down();
		m_inputs[Brake] = m_brakeKey.down();
		m_inputs[Left] = m_leftKey.down();
		m_inputs[Right] = m_rightKey.down();
		m_inputs[Handbrake] = m_handbrakeKey.down();
	}

	void PlayerCar::render(Renderer &renderer, f64 partialTick)
	{
		std::unique_lock lock(m_updateLock);

		if(m_lastLapTime > 0.0)
		{
			std::stringstream str;
			str << "Racing game - last lap time: " << m_lastLapTime << " seconds";
			m_lastLapTime = -1.0;
			renderer.window().title(str.str());
		}

		renderer.drawQuad(m_sprite);

		if constexpr(DrawHitboxes) drawHitbox(renderer, m_hitbox);
	}

	// returns the position of the car and that's used to keep the car in the centre of the screen
	// interpolates between the old position and new position of the car depending on how far through the tick it is
	glm::dvec2 PlayerCar::updateFramePosition(f64 partialTick)
	{
		std::unique_lock lock(m_updateLock);

		m_sprite.m_rotation = util::lerp(m_prevRotation, m_rotation, partialTick);
		return m_sprite.m_position = util::lerp(m_prevPosition, m_position, partialTick);
	}

	void PlayerCar::winRace()
	{
		std::cout << "You won the race! Your time: " << m_lastLapTime << " seconds" << std::endl;
	}

	void PlayerCar::loseRace()
	{
		std::cout << "You lost the race! Your time: " << m_lastLapTime << " seconds" << std::endl;
	}

	NpcCar::NpcCar(u32 index)
		: m_index(index)
	{
		m_prevPosition = m_position = m_hitbox.m_position = m_sprite.m_position = NpcStartPos[index];
		m_sprite.m_textureOverride = &s_data->m_npcCarTexture;
	}

	// updates the inputs based on if it is on the right tick or not
	void NpcCar::updateInputs(u64 tick)
	{
		for(const auto &action : NpcInputs)
		{
			if(std::get<0>(action) + 1 == tick)
			{
				//std::cout << "tick " << (tick + 1) << ": input " << std::get<1>(action) << (std::get<2>(action) ? " on (" : " off (") << InputNames[std::get<1>(action)] << ')' << std::endl;
				m_inputs[std::get<1>(action)] = std::get<2>(action);
			}
		}
	}

	void NpcCar::render(Renderer &renderer, f64 partialTick)
	{
		std::unique_lock lock(m_updateLock);

		m_sprite.m_position = util::lerp(m_prevPosition, m_position, partialTick);
		m_sprite.m_rotation = util::lerp(m_prevRotation, m_rotation, partialTick);

		renderer.drawQuad(m_sprite);

		if constexpr(DrawHitboxes) drawHitbox(renderer, m_hitbox);
	}

	void World::tick(f64 delta, u64 tick)
	{
		std::shared_lock lock(m_entityLock);

		for(auto &entity : m_entities)
		{
			entity->tick(*this, delta, tick);
		}
	}

	void World::render(Renderer &renderer, f64 partialTick)
	{
		std::shared_lock lock(m_entityLock);

		for(auto &entity : m_entities)
		{
			entity->render(renderer, partialTick);
		}

		if(DrawHitboxes) s_data->drawStartLineHitbox(renderer);
	}

	void World::addEntity(std::shared_ptr<Entity> entity)
	{
		std::unique_lock lock(m_entityLock);
		m_entities.push_back(std::move(entity));
	}

	void World::removeEntity(const std::shared_ptr<Entity> &entity)
	{
		std::unique_lock lock(m_entityLock);
		m_entities.erase(std::remove_if(std::begin(m_entities), std::end(m_entities), [&entity](const std::shared_ptr<Entity> &elem) { return entity.get() == elem.get(); }), std::end(m_entities));
	}
}
