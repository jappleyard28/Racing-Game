#include "util.h"
#include <glm/vec2.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <array>
#include <algorithm>

namespace game::util
{
	namespace
	{
        inline bool sat(const std::array<f64, 4> &proj, f64 len)
		{
			f64 min = std::min(std::min(proj[0], proj[1]), std::min(proj[2], proj[3]));
			f64 max = std::max(std::max(proj[0], proj[1]), std::max(proj[2], proj[3]));

			return min > len || max < 0.0;
		}
	}

	//detects whether this obb intersects the other obb with separating axis theorem
	bool OrientedBoundingBox::intersects(const OrientedBoundingBox &other) const
	{
	    //stores width, height, rotation and centre (m_position), rotate the size and adds to the position

	    //first object
		const auto halfsize = m_size / 2.0;
		const std::array vertices {
			m_position + glm::rotate(glm::dvec2 { -halfsize.x, -halfsize.y }, m_rotation),
			m_position + glm::rotate(glm::dvec2 { halfsize.x, -halfsize.y }, m_rotation),
			m_position + glm::rotate(glm::dvec2 { halfsize.x, halfsize.y }, m_rotation),
			m_position + glm::rotate(glm::dvec2 { -halfsize.x, halfsize.y }, m_rotation)
		};

		//second object which it might be colliding with
		const auto otherHalfsize = other.m_size / 2.0;
		const std::array otherVertices {
			other.m_position + glm::rotate(glm::dvec2 { -otherHalfsize.x, -otherHalfsize.y }, other.m_rotation),
			other.m_position + glm::rotate(glm::dvec2 { otherHalfsize.x, -otherHalfsize.y }, other.m_rotation),
			other.m_position + glm::rotate(glm::dvec2 { otherHalfsize.x, otherHalfsize.y }, other.m_rotation),
			other.m_position + glm::rotate(glm::dvec2 { -otherHalfsize.x, otherHalfsize.y }, other.m_rotation)
		};

		//lab stuff
		glm::dvec2 axis = vertices[1] - vertices[0];
		f64 len = glm::length(axis);

		axis /= len; //divides by the length to get a unit vector

		if(sat({
	        glm::dot(otherVertices[0] - vertices[0], axis),
	        glm::dot(otherVertices[1] - vertices[0], axis),
	        glm::dot(otherVertices[2] - vertices[0], axis),
	        glm::dot(otherVertices[3] - vertices[0], axis)
		}, len)) return false;

		axis = vertices[2] - vertices[1];
		len = glm::length(axis);

		axis /= len;

		if(sat({
	        glm::dot(otherVertices[0] - vertices[1], axis),
	        glm::dot(otherVertices[1] - vertices[1], axis),
	        glm::dot(otherVertices[2] - vertices[1], axis),
	        glm::dot(otherVertices[3] - vertices[1], axis)
		}, len)) return false;

		axis = otherVertices[1] - otherVertices[0];
		len = glm::length(axis);

		axis /= len;

		if(sat({
			glm::dot(vertices[0] - otherVertices[0], axis),
			glm::dot(vertices[1] - otherVertices[0], axis),
			glm::dot(vertices[2] - otherVertices[0], axis),
			glm::dot(vertices[3] - otherVertices[0], axis)
		}, len)) return false;

		axis = otherVertices[2] - otherVertices[1];
		len = glm::length(axis);

		axis /= len;

		if(sat({
			glm::dot(vertices[0] - otherVertices[1], axis),
			glm::dot(vertices[1] - otherVertices[1], axis),
			glm::dot(vertices[2] - otherVertices[1], axis),
			glm::dot(vertices[3] - otherVertices[1], axis)
		}, len)) return false;

		return true;
	}
}
