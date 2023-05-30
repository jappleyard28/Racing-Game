#pragma once

#include "types.h"
#include <glm/vec2.hpp>
#include <cmath>
#include <algorithm>

namespace game::util
{
	template <typename T>
	constexpr T Pi = T(3.14159265358979323846264338327950288);

	template <typename T>
	[[nodiscard]] constexpr inline T toRad(T t)
	{
		return t * T(Pi<T> / T(180.0));
	}

	template <typename T>
	[[nodiscard]] constexpr inline T toDeg(T t)
	{
		return t * T(T(180.0) / Pi<T>);
	}

	template <typename T>
	[[nodiscard]] constexpr inline i32 sign(T v) //returns -1 if it's below 0 and 1 otherwise
	{
		return v < T(0) ? -1 : 1;
	}

	template <typename T>
	[[nodiscard]] constexpr inline T lerp(T v0, T v1, f64 t) //linear interpolation
	{
		return v0 + t * (v1 - v0);
	}

	template <typename T>
	[[nodiscard]] constexpr inline T lerpClamped(T v0, T v1, f64 t)
	{
		return v0 + std::clamp(t, 0.0, 1.0) * (v1 - v0);
	}

	struct OrientedBoundingBox
	{
		OrientedBoundingBox() = default;
		OrientedBoundingBox(glm::dvec2 position, f64 rotation, glm::dvec2 size)
			: m_position(position),
			  m_rotation(rotation),
			  m_size(size) {}

		[[nodiscard]] bool intersects(const OrientedBoundingBox &other) const;

		glm::dvec2 m_position { 0.0 };
		f64 m_rotation = 0.0;
		glm::dvec2 m_size { 1.0, 1.0 };
	};
}
