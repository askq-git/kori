// Copyright (C) 2026 ASKQ
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "../core/animation-settings.hpp"

#include <string>
#include <cstdint>

struct obs_scene_item;
typedef struct obs_scene_item obs_sceneitem_t;
struct obs_source;
typedef struct obs_source obs_source_t;

namespace kori {

class SettingsStore {
public:
	AnimationSettings load_defaults() const;
	AnimationSettings load_for(const obs_sceneitem_t *item) const;
	AnimationSettings load_for_scene(
		const obs_source_t *scene_source) const;
	void save_for(const obs_sceneitem_t *item,
		      const AnimationSettings &settings) const;
	void save_for_scene(const obs_source_t *scene_source,
			    const AnimationSettings &settings) const;
	int64_t manual_target_item_id_for(
		const obs_source_t *scene_source) const;
	int64_t automatic_target_item_id_for(
		const obs_source_t *scene_source) const;
	void set_manual_target(const obs_source_t *scene_source,
			       int64_t item_id) const;
	void clear_automatic_target(const obs_source_t *scene_source) const;

private:
	static std::string profile_section(const obs_sceneitem_t *item);
	static std::string scene_section(const obs_source_t *scene_source);
};

} // namespace kori
