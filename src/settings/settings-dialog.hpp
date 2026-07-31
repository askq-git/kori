// Copyright (C) 2026 ASK Q Limited
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "../core/animation-settings.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

class QWidget;
struct obs_source;
typedef struct obs_source obs_source_t;

namespace kori {

struct SettingsTarget {
	int64_t item_id;
	std::string name;
};

void show_settings_dialog(
	QWidget *parent, const std::string &scene_name,
	const std::vector<SettingsTarget> &targets, int64_t target_item_id,
	int64_t automatic_target_item_id, const AnimationSettings &settings,
	const std::function<AnimationSettings(int64_t)> &load_settings,
	const std::function<obs_source_t *(int64_t)> &source_for_target,
	const std::function<bool(int64_t, const AnimationSettings &)> &save_settings,
	const std::function<void(int64_t, const AnimationSettings &)> &preview_zoom,
	const std::function<void()> &preview_return);

} // namespace kori
