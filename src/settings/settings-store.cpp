// Copyright (C) 2026 ASKQ
// SPDX-License-Identifier: GPL-2.0-or-later

#include "settings-store.hpp"

#include <obs-frontend-api.h>
#include <obs.h>
#include <util/config-file.h>

#include <string>

namespace {

constexpr const char *defaults_section = "Kori";

void read_settings(config_t *config, const char *section,
		   kori::AnimationSettings &settings)
{
	if (config_has_user_value(config, section, "ZoomFactor"))
		settings.zoom_factor = static_cast<float>(
			config_get_double(config, section, "ZoomFactor"));
	if (config_has_user_value(config, section, "ZoomDuration"))
		settings.zoom_duration = static_cast<float>(
			config_get_double(config, section, "ZoomDuration"));
	if (config_has_user_value(config, section, "ReturnDuration"))
		settings.return_duration = static_cast<float>(
			config_get_double(config, section, "ReturnDuration"));
	if (config_has_user_value(config, section, "FocusX"))
		settings.focus_x = static_cast<float>(
			config_get_double(config, section, "FocusX"));
	if (config_has_user_value(config, section, "FocusY"))
		settings.focus_y = static_cast<float>(
			config_get_double(config, section, "FocusY"));
	if (config_has_user_value(config, section, "Easing")) {
		const int value =
			static_cast<int>(config_get_int(config, section, "Easing"));
		if (value >= static_cast<int>(kori::EasingPreset::Smooth) &&
		    value <= static_cast<int>(kori::EasingPreset::Linear))
			settings.easing =
				static_cast<kori::EasingPreset>(value);
	}
	if (config_has_user_value(config, section, "Activation")) {
		const int value = static_cast<int>(
			config_get_int(config, section, "Activation"));
		if (value >= static_cast<int>(
				     kori::ActivationMode::Manual) &&
		    value <= static_cast<int>(
				     kori::ActivationMode::SceneActive))
			settings.activation =
				static_cast<kori::ActivationMode>(value);
	}
	if (config_has_user_value(config, section, "Completion")) {
		const int value = static_cast<int>(
			config_get_int(config, section, "Completion"));
		if (value >= static_cast<int>(
				     kori::CompletionMode::StayZoomed) &&
		    value <= static_cast<int>(
				     kori::CompletionMode::AutoReturn))
			settings.completion =
				static_cast<kori::CompletionMode>(value);
	}
	if (config_has_user_value(config, section, "StartDelay"))
		settings.start_delay = static_cast<float>(
			config_get_double(config, section, "StartDelay"));
	if (config_has_user_value(config, section, "HoldDuration"))
		settings.hold_duration = static_cast<float>(
			config_get_double(config, section, "HoldDuration"));
	if (config_has_user_value(config, section, "ResetOnSceneExit"))
		settings.reset_on_scene_exit = config_get_bool(
			config, section, "ResetOnSceneExit");
}

void write_settings(config_t *config, const char *section,
		    const kori::AnimationSettings &settings)
{
	config_set_double(config, section, "ZoomFactor", settings.zoom_factor);
	config_set_double(config, section, "ZoomDuration",
			  settings.zoom_duration);
	config_set_double(config, section, "ReturnDuration",
			  settings.return_duration);
	config_set_double(config, section, "FocusX", settings.focus_x);
	config_set_double(config, section, "FocusY", settings.focus_y);
	config_set_int(config, section, "Easing",
		       static_cast<int>(settings.easing));
	config_set_int(config, section, "Activation",
		       static_cast<int>(settings.activation));
	config_set_int(config, section, "Completion",
		       static_cast<int>(settings.completion));
	config_set_double(config, section, "StartDelay", settings.start_delay);
	config_set_double(config, section, "HoldDuration",
			  settings.hold_duration);
	config_set_bool(config, section, "ResetOnSceneExit",
			settings.reset_on_scene_exit);
}

std::string scene_profile_section(const obs_source_t *scene_source)
{
	const char *uuid =
		scene_source ? obs_source_get_uuid(scene_source) : nullptr;
	if (!uuid || !*uuid)
		return {};
	return std::string("Kori.SceneProfile.") + uuid;
}

} // namespace

namespace kori {

std::string SettingsStore::profile_section(const obs_sceneitem_t *item)
{
	if (!item)
		return {};

	obs_scene_t *scene = obs_sceneitem_get_scene(item);
	obs_source_t *scene_source = scene ? obs_scene_get_source(scene) : nullptr;
	const char *scene_uuid =
		scene_source ? obs_source_get_uuid(scene_source) : nullptr;
	if (!scene_uuid || !*scene_uuid)
		return {};

	return std::string("Kori.Profile.") + scene_uuid + "." +
	       std::to_string(obs_sceneitem_get_id(item));
}

std::string SettingsStore::scene_section(const obs_source_t *scene_source)
{
	const char *scene_uuid =
		scene_source ? obs_source_get_uuid(scene_source) : nullptr;
	if (!scene_uuid || !*scene_uuid)
		return {};
	return std::string("Kori.Scene.") + scene_uuid;
}

AnimationSettings SettingsStore::load_defaults() const
{
	AnimationSettings settings;
	config_t *config = obs_frontend_get_user_config();
	if (!config)
		return settings;

	read_settings(config, defaults_section, settings);
	return settings;
}

AnimationSettings SettingsStore::load_for(const obs_sceneitem_t *item) const
{
	AnimationSettings settings = load_defaults();
	config_t *config = obs_frontend_get_user_config();
	const std::string section = profile_section(item);
	if (config && !section.empty())
		read_settings(config, section.c_str(), settings);
	return settings;
}

AnimationSettings
SettingsStore::load_for_scene(const obs_source_t *scene_source) const
{
	AnimationSettings settings = load_defaults();
	config_t *config = obs_frontend_get_user_config();
	const std::string section = scene_profile_section(scene_source);
	if (config && !section.empty())
		read_settings(config, section.c_str(), settings);
	return settings;
}

void SettingsStore::save_for(const obs_sceneitem_t *item,
			     const AnimationSettings &settings) const
{
	config_t *config = obs_frontend_get_user_config();
	const std::string section = profile_section(item);
	if (!config || section.empty())
		return;

	write_settings(config, section.c_str(), settings);

	obs_scene_t *scene = obs_sceneitem_get_scene(item);
	obs_source_t *scene_source = scene ? obs_scene_get_source(scene) : nullptr;
	const std::string scene_profile = scene_section(scene_source);
	if (!scene_profile.empty()) {
		const int64_t item_id = obs_sceneitem_get_id(item);
		config_set_int(config, scene_profile.c_str(),
			       "ManualTargetItemId", item_id);

		const int64_t previous_automatic =
			automatic_target_item_id_for(scene_source);
		if (settings.activation == ActivationMode::SceneActive) {
			if (previous_automatic >= 0 &&
			    previous_automatic != item_id) {
				const char *scene_uuid =
					obs_source_get_uuid(scene_source);
				const std::string previous_section =
					std::string("Kori.Profile.") +
					scene_uuid + "." +
					std::to_string(previous_automatic);
				config_set_int(
					config, previous_section.c_str(),
					"Activation",
					static_cast<int>(
						ActivationMode::Manual));
			} else if (previous_automatic ==
				   EntireSceneTargetId) {
				const std::string previous_section =
					scene_profile_section(scene_source);
				config_set_int(
					config, previous_section.c_str(),
					"Activation",
					static_cast<int>(
						ActivationMode::Manual));
			}
			config_set_int(config, scene_profile.c_str(),
				       "AutomaticTargetItemId", item_id);
		} else if (previous_automatic == item_id) {
			config_remove_value(config, scene_profile.c_str(),
					    "AutomaticTargetItemId");
		}
	}
	config_save_safe(config, "tmp", nullptr);
}

void SettingsStore::save_for_scene(
	const obs_source_t *scene_source,
	const AnimationSettings &settings) const
{
	config_t *config = obs_frontend_get_user_config();
	const std::string profile = scene_profile_section(scene_source);
	const std::string scene_profile = scene_section(scene_source);
	if (!config || profile.empty() || scene_profile.empty())
		return;

	write_settings(config, profile.c_str(), settings);
	config_set_int(config, scene_profile.c_str(), "ManualTargetItemId",
		       EntireSceneTargetId);

	const int64_t previous_automatic =
		automatic_target_item_id_for(scene_source);
	if (settings.activation == ActivationMode::SceneActive) {
		if (previous_automatic >= 0) {
			const char *scene_uuid =
				obs_source_get_uuid(scene_source);
			const std::string previous_section =
				std::string("Kori.Profile.") +
				scene_uuid + "." +
				std::to_string(previous_automatic);
			config_set_int(
				config, previous_section.c_str(), "Activation",
				static_cast<int>(ActivationMode::Manual));
		}
		config_set_int(config, scene_profile.c_str(),
			       "AutomaticTargetItemId",
			       EntireSceneTargetId);
	} else if (previous_automatic == EntireSceneTargetId) {
		config_remove_value(config, scene_profile.c_str(),
				    "AutomaticTargetItemId");
	}
	config_save_safe(config, "tmp", nullptr);
}

int64_t SettingsStore::manual_target_item_id_for(
	const obs_source_t *scene_source) const
{
	config_t *config = obs_frontend_get_user_config();
	const std::string section = scene_section(scene_source);
	if (!config || section.empty())
		return -1;
	if (config_has_user_value(config, section.c_str(),
				  "ManualTargetItemId"))
		return config_get_int(config, section.c_str(),
				      "ManualTargetItemId");
	if (config_has_user_value(config, section.c_str(), "TargetItemId"))
		return config_get_int(config, section.c_str(), "TargetItemId");
	return -1;
}

int64_t SettingsStore::automatic_target_item_id_for(
	const obs_source_t *scene_source) const
{
	config_t *config = obs_frontend_get_user_config();
	const std::string section = scene_section(scene_source);
	if (!config || section.empty() ||
	    !config_has_user_value(config, section.c_str(),
				   "AutomaticTargetItemId"))
		return -1;
	return config_get_int(config, section.c_str(),
			      "AutomaticTargetItemId");
}

void SettingsStore::set_manual_target(const obs_source_t *scene_source,
				      const int64_t item_id) const
{
	config_t *config = obs_frontend_get_user_config();
	const std::string section = scene_section(scene_source);
	if (!config || section.empty())
		return;
	config_set_int(config, section.c_str(), "ManualTargetItemId", item_id);
	config_save_safe(config, "tmp", nullptr);
}

void SettingsStore::clear_automatic_target(
	const obs_source_t *scene_source) const
{
	config_t *config = obs_frontend_get_user_config();
	const std::string section = scene_section(scene_source);
	if (!config || section.empty())
		return;
	config_remove_value(config, section.c_str(), "AutomaticTargetItemId");
	config_save_safe(config, "tmp", nullptr);
}

} // namespace kori
