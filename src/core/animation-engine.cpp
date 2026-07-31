// Copyright (C) 2026 ASK Q Limited
// SPDX-License-Identifier: GPL-2.0-or-later

#include "animation-engine.hpp"

#include "easing.hpp"

#include <algorithm>
#include <cmath>
#include <obs-module.h>

namespace {

bool collect_scene_item(obs_scene_t *, obs_sceneitem_t *item, void *data)
{
	auto *items = static_cast<std::vector<obs_sceneitem_t *> *>(data);
	items->push_back(item);
	return true;
}

vec2 interpolate(const vec2 &from, const vec2 &to, const float amount)
{
	vec2 result{};
	result.x = from.x + (to.x - from.x) * amount;
	result.y = from.y + (to.y - from.y) * amount;
	return result;
}

} // namespace

namespace kori {

void AnimationEngine::set_settings(const AnimationSettings &settings)
{
	settings_ = settings;
}

void AnimationEngine::capture_item(std::unique_ptr<SceneItemTarget> target)
{
	if (!target || !target->get())
		return;

	AnimatedItem captured;
	captured.target = std::move(target);
	obs_sceneitem_get_info2(captured.target->get(),
				&captured.original_transform);
	obs_sceneitem_get_crop(captured.target->get(),
			       &captured.original_crop);
	captured.start_position = captured.original_transform.pos;
	captured.start_scale = captured.original_transform.scale;
	captured.start_bounds = captured.original_transform.bounds;
	captured.end_position = captured.start_position;
	captured.end_scale = captured.start_scale;
	captured.end_bounds = captured.start_bounds;
	items_.push_back(std::move(captured));
}

void AnimationEngine::play(std::unique_ptr<SceneItemTarget> target,
			   const bool automatic)
{
	if (!target || !target->get())
		return;
	if (has_snapshot())
		restore_snapshot(false);

	obs_source_t *source = obs_sceneitem_get_source(target->get());
	target_description_ =
		source ? obs_source_get_name(source) : "<unknown>";
	capture_item(std::move(target));
	if (!has_snapshot())
		return;
	calculate_source_target(items_.front());
	automatic_ = automatic;
	whole_scene_ = false;

	if (automatic_ && settings_.start_delay > 0.0F) {
		elapsed_ = 0.0F;
		state_ = State::Waiting;
		blog(LOG_INFO,
		     "[Kori] Automatic zoom armed: target='%s', delay=%.1fs",
		     target_description_.c_str(), settings_.start_delay);
		return;
	}
	begin_zoom();
}

void AnimationEngine::play_scene(obs_source_t *scene_source,
				 const bool automatic)
{
	if (!scene_source)
		return;
	obs_scene_t *scene = obs_scene_from_source(scene_source);
	if (!scene)
		return;
	if (has_snapshot())
		restore_snapshot(false);

	std::vector<obs_sceneitem_t *> scene_items;
	obs_scene_enum_items(scene, collect_scene_item, &scene_items);
	for (obs_sceneitem_t *item : scene_items)
		capture_item(std::make_unique<SceneItemTarget>(item));
	if (!has_snapshot()) {
		blog(LOG_WARNING,
		     "[Kori] Entire scene zoom ignored: scene has no items");
		return;
	}

	obs_video_info video{};
	const bool have_video = obs_get_video_info(&video);
	const float width = static_cast<float>(
		have_video ? video.base_width : obs_source_get_width(scene_source));
	const float height = static_cast<float>(
		have_video ? video.base_height : obs_source_get_height(scene_source));
	for (auto &item : items_)
		calculate_scene_target(item, std::max(width, 1.0F),
				       std::max(height, 1.0F));

	target_description_ =
		std::string("Entire scene: ") + obs_source_get_name(scene_source);
	automatic_ = automatic;
	whole_scene_ = true;
	if (automatic_ && settings_.start_delay > 0.0F) {
		elapsed_ = 0.0F;
		state_ = State::Waiting;
		blog(LOG_INFO,
		     "[Kori] Automatic whole-scene zoom armed: scene='%s', items=%zu, delay=%.1fs",
		     obs_source_get_name(scene_source), items_.size(),
		     settings_.start_delay);
		return;
	}
	begin_zoom();
}

void AnimationEngine::calculate_source_target(AnimatedItem &item)
{
	obs_sceneitem_t *scene_item = item.target->get();
	obs_source_t *source = obs_sceneitem_get_source(scene_item);
	const float width = std::max(
		1.0F, static_cast<float>(obs_source_get_width(source)) -
			      static_cast<float>(item.original_crop.left +
						 item.original_crop.right));
	const float height = std::max(
		1.0F, static_cast<float>(obs_source_get_height(source)) -
			      static_cast<float>(item.original_crop.top +
						 item.original_crop.bottom));

	float anchor_x = 0.5F;
	float anchor_y = 0.5F;
	if (item.original_transform.alignment & OBS_ALIGN_LEFT)
		anchor_x = 0.0F;
	else if (item.original_transform.alignment & OBS_ALIGN_RIGHT)
		anchor_x = 1.0F;
	if (item.original_transform.alignment & OBS_ALIGN_TOP)
		anchor_y = 0.0F;
	else if (item.original_transform.alignment & OBS_ALIGN_BOTTOM)
		anchor_y = 1.0F;

	const float local_x = (settings_.focus_x - anchor_x) * width *
			      item.start_scale.x;
	const float local_y = (settings_.focus_y - anchor_y) * height *
			      item.start_scale.y;
	const float radians =
		item.original_transform.rot * 3.14159265358979323846F / 180.0F;
	const float world_x =
		std::cos(radians) * local_x - std::sin(radians) * local_y;
	const float world_y =
		std::sin(radians) * local_x + std::cos(radians) * local_y;

	item.end_scale.x = item.start_scale.x * settings_.zoom_factor;
	item.end_scale.y = item.start_scale.y * settings_.zoom_factor;
	item.end_position.x =
		item.start_position.x - world_x * (settings_.zoom_factor - 1.0F);
	item.end_position.y =
		item.start_position.y - world_y * (settings_.zoom_factor - 1.0F);
}

void AnimationEngine::calculate_scene_target(AnimatedItem &item,
					     const float canvas_width,
					     const float canvas_height)
{
	const float focus_x = settings_.focus_x * canvas_width;
	const float focus_y = settings_.focus_y * canvas_height;
	item.end_position.x =
		focus_x + (item.start_position.x - focus_x) *
				  settings_.zoom_factor;
	item.end_position.y =
		focus_y + (item.start_position.y - focus_y) *
				  settings_.zoom_factor;

	if (item.original_transform.bounds_type == OBS_BOUNDS_NONE) {
		item.end_scale.x =
			item.start_scale.x * settings_.zoom_factor;
		item.end_scale.y =
			item.start_scale.y * settings_.zoom_factor;
	} else {
		item.end_bounds.x =
			item.start_bounds.x * settings_.zoom_factor;
		item.end_bounds.y =
			item.start_bounds.y * settings_.zoom_factor;
	}
}

void AnimationEngine::begin_segment(const bool returning,
				    const float duration, const State state)
{
	for (auto &item : items_) {
		if (returning) {
			obs_transform_info current{};
			obs_sceneitem_get_info2(item.target->get(), &current);
			item.segment_from_position = current.pos;
			item.segment_from_scale = current.scale;
			item.segment_from_bounds = current.bounds;
			item.segment_to_position = item.start_position;
			item.segment_to_scale = item.start_scale;
			item.segment_to_bounds = item.start_bounds;
		} else {
			item.segment_from_position = item.start_position;
			item.segment_from_scale = item.start_scale;
			item.segment_from_bounds = item.start_bounds;
			item.segment_to_position = item.end_position;
			item.segment_to_scale = item.end_scale;
			item.segment_to_bounds = item.end_bounds;
		}
	}
	segment_duration_ = std::max(0.05F, duration);
	elapsed_ = 0.0F;
	state_ = state;
}

void AnimationEngine::begin_zoom()
{
	begin_segment(false, settings_.zoom_duration, State::Zooming);
	blog(LOG_INFO,
	     "[Kori] Zoom started: target='%s', items=%zu, zoom=%.2fx, duration=%.1fs, focus=(%.0f%%, %.0f%%)",
	     target_description_.c_str(), items_.size(), settings_.zoom_factor,
	     settings_.zoom_duration, settings_.focus_x * 100.0F,
	     settings_.focus_y * 100.0F);
}

void AnimationEngine::return_to_start()
{
	if (!has_snapshot()) {
		blog(LOG_INFO, "[Kori] Return ignored: no captured transform");
		return;
	}
	begin_segment(true, settings_.return_duration, State::Returning);
	blog(LOG_INFO,
	     "[Kori] Animated return started: target='%s', duration=%.1fs",
	     target_description_.c_str(), settings_.return_duration);
}

void AnimationEngine::restore_snapshot(const bool log_result)
{
	if (!has_snapshot())
		return;
	for (auto &item : items_) {
		if (!item.target || !item.target->get())
			continue;
		obs_sceneitem_set_info2(item.target->get(),
					&item.original_transform);
		obs_sceneitem_set_crop(item.target->get(),
				       &item.original_crop);
	}
	items_.clear();
	state_ = State::Idle;
	automatic_ = false;
	whole_scene_ = false;
	if (log_result)
		blog(LOG_INFO,
		     "[Kori] Original transform set restored exactly");
}

void AnimationEngine::restore_immediately()
{
	if (!has_snapshot()) {
		blog(LOG_INFO,
		     "[Kori] Immediate restore ignored: no captured transform");
		return;
	}
	restore_snapshot(true);
}

void AnimationEngine::handle_scene_exit()
{
	if (!has_snapshot())
		return;
	if (settings_.reset_on_scene_exit) {
		blog(LOG_INFO,
		     "[Kori] Scene exited; cancelling automation and restoring target");
		restore_snapshot(false);
		return;
	}
	automatic_ = false;
	state_ = State::Zoomed;
	blog(LOG_INFO,
	     "[Kori] Scene exited; automatic actions cancelled without restoring target");
}

void AnimationEngine::tick(const float seconds)
{
	if (!has_snapshot())
		return;
	if (state_ == State::Waiting) {
		elapsed_ += seconds;
		if (elapsed_ >= settings_.start_delay)
			begin_zoom();
		return;
	}
	if (state_ == State::Holding) {
		elapsed_ += seconds;
		if (elapsed_ >= settings_.hold_duration)
			return_to_start();
		return;
	}
	if (state_ != State::Zooming && state_ != State::Returning)
		return;

	elapsed_ += seconds;
	const float progress =
		std::clamp(elapsed_ / segment_duration_, 0.0F, 1.0F);
	const float eased = apply_easing(progress, settings_.easing);
	for (auto &item : items_) {
		obs_transform_info current = item.original_transform;
		current.pos = interpolate(item.segment_from_position,
					  item.segment_to_position, eased);
		current.scale = interpolate(item.segment_from_scale,
					    item.segment_to_scale, eased);
		current.bounds = interpolate(item.segment_from_bounds,
					     item.segment_to_bounds, eased);
		if (whole_scene_) {
			obs_sceneitem_set_info2(item.target->get(), &current);
		} else {
			obs_sceneitem_set_pos(item.target->get(), &current.pos);
			obs_sceneitem_set_scale(item.target->get(),
						&current.scale);
		}
	}

	if (progress < 1.0F)
		return;
	if (state_ == State::Returning) {
		restore_snapshot(true);
	} else if (automatic_ &&
		   settings_.completion == CompletionMode::AutoReturn) {
		state_ = State::Holding;
		elapsed_ = 0.0F;
		blog(LOG_INFO,
		     "[Kori] Zoom complete; holding for %.1fs before automatic return",
		     settings_.hold_duration);
	} else {
		state_ = State::Zoomed;
		blog(LOG_INFO,
		     "[Kori] Zoom complete; Return animates to the captured transform");
	}
}

} // namespace kori
