// Copyright (C) 2026 ASK Q Limited
// SPDX-License-Identifier: GPL-2.0-or-later

#include "easing.hpp"

#include <cmath>

namespace kori {

namespace {

float ease_in_out_cubic(const float progress)
{
	if (progress < 0.5F)
		return 4.0F * progress * progress * progress;

	const float p = -2.0F * progress + 2.0F;
	return 1.0F - (p * p * p) / 2.0F;
}

float ease_in_out_smootherstep(const float progress)
{
	return progress * progress * progress *
	       (progress * (progress * 6.0F - 15.0F) + 10.0F);
}

float ease_in_out_quint(const float progress)
{
	if (progress < 0.5F)
		return 16.0F * progress * progress * progress * progress *
		       progress;

	const float p = -2.0F * progress + 2.0F;
	return 1.0F - (p * p * p * p * p) / 2.0F;
}

float ease_out_back(const float progress)
{
	constexpr float overshoot = 1.70158F;
	const float p = progress - 1.0F;
	return 1.0F + (overshoot + 1.0F) * p * p * p +
	       overshoot * p * p;
}

} // namespace

float apply_easing(const float progress, const EasingPreset preset)
{
	switch (preset) {
	case EasingPreset::Cinematic:
		return ease_in_out_smootherstep(progress);
	case EasingPreset::SlowBurn:
		return ease_in_out_quint(progress);
	case EasingPreset::Punch:
		return ease_out_back(progress);
	case EasingPreset::Linear:
		return progress;
	case EasingPreset::Smooth:
	default:
		return ease_in_out_cubic(progress);
	}
}

} // namespace kori
