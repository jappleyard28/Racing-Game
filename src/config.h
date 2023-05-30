#pragma once

#include <GLFW/glfw3.h>
#include "types.h"

namespace game::config
{
	// THESE WILL NOT AFFECT THE CONTROLS SHOWN ON THE TRACK
	constexpr i32 KeyAccelerate = GLFW_KEY_W;
	constexpr i32 KeyReverse = GLFW_KEY_LEFT_CONTROL;
	constexpr i32 KeyBrake = GLFW_KEY_S;
	constexpr i32 KeySteerLeft = GLFW_KEY_A;
	constexpr i32 KeySteerRight = GLFW_KEY_D;
	constexpr i32 KeyHandbrake = GLFW_KEY_SPACE;

	constexpr u32 MsaaSamples = 4;
}
