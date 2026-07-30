// Copyright (C) 2026 ASKQ
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "animation-settings.hpp"
#include "scene-item-target.hpp"

#include <memory>
#include <obs.h>
#include <string>
#include <vector>

namespace kori {

class AnimationEngine {
public:
	void set_settings(const AnimationSettings &settings);
	void play(std::unique_ptr<SceneItemTarget> target,
		  bool automatic = false);
	void play_scene(obs_source_t *scene_source, bool automatic = false);
	void return_to_start();
	void restore_immediately();
	void handle_scene_exit();
	void tick(float seconds);

private:
	enum class State {
		Idle,
		Waiting,
		Zooming,
		Holding,
		Zoomed,
		Returning
	};

	struct AnimatedItem {
		std::unique_ptr<SceneItemTarget> target;
		obs_transform_info original_transform{};
		obs_sceneitem_crop original_crop{};
		vec2 start_position{};
		vec2 start_scale{};
		vec2 start_bounds{};
		vec2 end_position{};
		vec2 end_scale{};
		vec2 end_bounds{};
		vec2 segment_from_position{};
		vec2 segment_from_scale{};
		vec2 segment_from_bounds{};
		vec2 segment_to_position{};
		vec2 segment_to_scale{};
		vec2 segment_to_bounds{};
	};

	void capture_item(std::unique_ptr<SceneItemTarget> target);
	void calculate_source_target(AnimatedItem &item);
	void calculate_scene_target(AnimatedItem &item, float canvas_width,
				    float canvas_height);
	void restore_snapshot(bool log_result);
	void begin_zoom();
	void begin_segment(bool returning, float duration, State state);
	bool has_snapshot() const { return !items_.empty(); }

	std::vector<AnimatedItem> items_;
	AnimationSettings settings_{};
	std::string target_description_;
	float elapsed_ = 0.0F;
	float segment_duration_ = 1.0F;
	State state_ = State::Idle;
	bool automatic_ = false;
	bool whole_scene_ = false;
};

} // namespace kori
