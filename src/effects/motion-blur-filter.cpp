// Copyright (C) 2026 ASK Q Limited
// SPDX-License-Identifier: GPL-2.0-or-later

#include "motion-blur-filter.hpp"

#include <graphics/vec2.h>
#include <obs-module.h>

namespace {

constexpr const char *filter_id = "kori_motion_blur_filter";

struct MotionBlurFilter {
	obs_source_t *context = nullptr;
	gs_effect_t *effect = nullptr;
	gs_eparam_t *amount_param = nullptr;
	gs_eparam_t *focus_param = nullptr;
	float amount = 0.0F;
	vec2 focus{0.5F, 0.28F};
};

const char *get_name(void *)
{
	return "Kori Motion Blur (temporary)";
}

void update(void *data, obs_data_t *settings)
{
	auto *filter = static_cast<MotionBlurFilter *>(data);
	filter->amount = static_cast<float>(
		obs_data_get_double(settings, "amount"));
	filter->focus.x = static_cast<float>(
		obs_data_get_double(settings, "focus_x"));
	filter->focus.y = static_cast<float>(
		obs_data_get_double(settings, "focus_y"));
}

void destroy(void *data)
{
	auto *filter = static_cast<MotionBlurFilter *>(data);
	if (!filter)
		return;
	if (filter->effect) {
		obs_enter_graphics();
		gs_effect_destroy(filter->effect);
		obs_leave_graphics();
	}
	delete filter;
}

void *create(obs_data_t *settings, obs_source_t *context)
{
	auto *filter = new MotionBlurFilter;
	filter->context = context;
	char *effect_path = obs_module_file("effects/kori-motion-blur.effect");
	char *error = nullptr;
	obs_enter_graphics();
	filter->effect = gs_effect_create_from_file(effect_path, &error);
	if (filter->effect) {
		filter->amount_param = gs_effect_get_param_by_name(
			filter->effect, "amount");
		filter->focus_param = gs_effect_get_param_by_name(
			filter->effect, "focus");
	}
	obs_leave_graphics();
	if (error) {
		blog(LOG_ERROR, "[Kori] Motion blur shader: %s", error);
		bfree(error);
	}
	bfree(effect_path);
	if (!filter->effect) {
		destroy(filter);
		return nullptr;
	}
	update(filter, settings);
	return filter;
}

void render(void *data, gs_effect_t *)
{
	auto *filter = static_cast<MotionBlurFilter *>(data);
	if (!filter->effect ||
	    !obs_source_process_filter_begin(filter->context, GS_RGBA,
					     OBS_NO_DIRECT_RENDERING))
		return;
	gs_effect_set_float(filter->amount_param, filter->amount);
	gs_effect_set_vec2(filter->focus_param, &filter->focus);
	obs_source_process_filter_end(filter->context, filter->effect, 0, 0);
}

obs_source_info make_source_info()
{
	obs_source_info info{};
	info.id = filter_id;
	info.type = OBS_SOURCE_TYPE_FILTER;
	info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_SRGB;
	info.get_name = get_name;
	info.create = create;
	info.destroy = destroy;
	info.update = update;
	info.video_render = render;
	return info;
}

} // namespace

namespace kori {

void register_motion_blur_filter()
{
	static obs_source_info info = make_source_info();
	obs_register_source(&info);
}

obs_source_t *create_motion_blur_filter(const float focus_x,
					const float focus_y)
{
	obs_data_t *settings = obs_data_create();
	obs_data_set_double(settings, "amount", 0.0);
	obs_data_set_double(settings, "focus_x", focus_x);
	obs_data_set_double(settings, "focus_y", focus_y);
	obs_source_t *filter = obs_source_create_private(
		filter_id, "Kori Motion Blur (temporary)", settings);
	obs_data_release(settings);
	return filter;
}

void update_motion_blur_filter(obs_source_t *filter, const float amount,
			       const float focus_x, const float focus_y)
{
	if (!filter)
		return;
	obs_data_t *settings = obs_data_create();
	obs_data_set_double(settings, "amount", amount);
	obs_data_set_double(settings, "focus_x", focus_x);
	obs_data_set_double(settings, "focus_y", focus_y);
	obs_source_update(filter, settings);
	obs_data_release(settings);
}

} // namespace kori
