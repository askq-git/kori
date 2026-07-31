// Copyright (C) 2026 ASK Q Limited
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <cstdint>

namespace kori {

inline constexpr int64_t EntireSceneTargetId = -2;

enum class EasingPreset {
	Smooth = 0,
	Cinematic = 1,
	SlowBurn = 2,
	Punch = 3,
	Linear = 4,
};

enum class ActivationMode {
	Manual = 0,
	SceneActive = 1,
};

enum class CompletionMode {
	StayZoomed = 0,
	AutoReturn = 1,
};

struct AnimationSettings {
	float zoom_factor = 1.30F;
	float zoom_duration = 5.0F;
	float return_duration = 2.0F;
	float focus_x = 0.50F;
	float focus_y = 0.28F;
	EasingPreset easing = EasingPreset::Smooth;
	ActivationMode activation = ActivationMode::Manual;
	CompletionMode completion = CompletionMode::StayZoomed;
	float start_delay = 0.0F;
	float hold_duration = 1.0F;
	bool reset_on_scene_exit = true;
};

} // namespace kori
