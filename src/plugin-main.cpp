// Copyright (C) 2026 ASKQ
// SPDX-License-Identifier: GPL-2.0-or-later

#include "core/animation-engine.hpp"
#include "obs/main-canvas-target-resolver.hpp"
#include "settings/settings-dialog.hpp"
#include "settings/settings-store.hpp"

#include <QApplication>
#include <QMessageBox>
#include <QMetaObject>
#include <QWidget>
#include <memory>
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <string>
#include <unordered_map>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("kori", "en-US")
OBS_MODULE_AUTHOR("ASKQ")

MODULE_EXPORT const char *obs_module_description(void)
{
	return "Kori: production-quality source and scene motion for OBS";
}

namespace {

std::unique_ptr<kori::AnimationEngine> engine;
std::unique_ptr<kori::MainCanvasTargetResolver> resolver;
kori::SettingsStore settings_store;
obs_hotkey_id play_hotkey = OBS_INVALID_HOTKEY_ID;
obs_hotkey_id reset_hotkey = OBS_INVALID_HOTKEY_ID;
obs_hotkey_id open_settings_hotkey = OBS_INVALID_HOTKEY_ID;

void trigger_current_scene_if_configured()
{
	if (!engine || !resolver)
		return;

	obs_source_t *scene_source = obs_frontend_get_current_scene();
	if (!scene_source)
		return;

	const std::string scene_name = obs_source_get_name(scene_source);
	int64_t configured_id =
		settings_store.automatic_target_item_id_for(scene_source);
	bool legacy_candidate = false;
	if (configured_id == -1) {
		configured_id =
			settings_store.manual_target_item_id_for(scene_source);
		legacy_candidate = configured_id != -1;
	}
	if (configured_id == -1) {
		obs_source_release(scene_source);
		return;
	}

	if (configured_id == kori::EntireSceneTargetId) {
		const auto profile =
			settings_store.load_for_scene(scene_source);
		if (profile.activation !=
		    kori::ActivationMode::SceneActive) {
			if (!legacy_candidate)
				settings_store.clear_automatic_target(
					scene_source);
			obs_source_release(scene_source);
			return;
		}
		engine->set_settings(profile);
		blog(LOG_INFO,
		     "[Kori] Scene '%s' activated; starting configured whole-scene automation",
		     scene_name.c_str());
		engine->play_scene(scene_source, true);
		obs_source_release(scene_source);
		return;
	}

	auto target = resolver->resolve_item_by_id(configured_id);
	if (!target || !target->get()) {
		blog(LOG_WARNING,
		     "[Kori] Automatic zoom skipped for scene '%s': configured source is unavailable",
		     scene_name.c_str());
		settings_store.clear_automatic_target(scene_source);
		obs_source_release(scene_source);
		return;
	}

	const auto profile = settings_store.load_for(target->get());
	if (profile.activation != kori::ActivationMode::SceneActive) {
		if (!legacy_candidate)
			settings_store.clear_automatic_target(scene_source);
		obs_source_release(scene_source);
		return;
	}
	if (legacy_candidate)
		settings_store.save_for(target->get(), profile);
	obs_source_release(scene_source);

	engine->set_settings(profile);
	blog(LOG_INFO, "[Kori] Scene '%s' activated; starting configured automation",
	     scene_name.c_str());
	engine->play(std::move(target), true);
}

void on_frontend_event(const enum obs_frontend_event event, void *)
{
	if (!engine)
		return;

	switch (event) {
	case OBS_FRONTEND_EVENT_SCENE_CHANGED:
		engine->handle_scene_exit();
		trigger_current_scene_if_configured();
		break;
	case OBS_FRONTEND_EVENT_FINISHED_LOADING:
		trigger_current_scene_if_configured();
		break;
	case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGING:
		engine->handle_scene_exit();
		break;
	case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CLEANUP:
	case OBS_FRONTEND_EVENT_EXIT:
		engine->restore_immediately();
		break;
	default:
		break;
	}
}

void on_play_hotkey(void *, obs_hotkey_id, obs_hotkey_t *, bool pressed)
{
	if (!pressed || !engine || !resolver)
		return;

	auto selected = resolver->resolve_selected_item(false);
	if (selected) {
		engine->set_settings(settings_store.load_for(selected->get()));
		engine->play(std::move(selected));
		return;
	}

	obs_source_t *scene_source = obs_frontend_get_current_scene();
	if (!scene_source)
		return;
	int64_t configured_id =
		settings_store.manual_target_item_id_for(scene_source);
	if (configured_id == kori::EntireSceneTargetId) {
		engine->set_settings(
			settings_store.load_for_scene(scene_source));
		engine->play_scene(scene_source);
		obs_source_release(scene_source);
		return;
	}

	auto target = configured_id >= 0
			      ? resolver->resolve_item_by_id(configured_id)
			      : nullptr;
	if (!target) {
		const auto available = resolver->available_items();
		if (!available.empty()) {
			configured_id = available.front().item_id;
			target = resolver->resolve_item_by_id(configured_id);
			settings_store.set_manual_target(scene_source,
							 configured_id);
			blog(LOG_WARNING,
			     "[Kori] Saved manual target was unavailable; recovered to '%s'",
			     available.front().name.c_str());
		}
	}
	obs_source_release(scene_source);
	if (!target) {
		blog(LOG_WARNING,
		     "[Kori] Play ignored: the scene has no animatable items");
		return;
	}
	engine->set_settings(settings_store.load_for(target->get()));
	engine->play(std::move(target));
}

void on_reset_hotkey(void *, obs_hotkey_id, obs_hotkey_t *, bool pressed)
{
	if (pressed && engine)
		engine->return_to_start();
}

void on_tick(void *, float seconds)
{
	if (engine)
		engine->tick(seconds);
}

void on_open_settings(void *);

void on_open_settings_hotkey(void *, obs_hotkey_id, obs_hotkey_t *,
			     const bool pressed)
{
	if (!pressed)
		return;
	QMetaObject::invokeMethod(
		QApplication::instance(),
		[]() { on_open_settings(nullptr); }, Qt::QueuedConnection);
}

void on_open_settings(void *)
{
	auto *parent =
		static_cast<QWidget *>(obs_frontend_get_main_window());
	obs_source_t *current_scene = obs_frontend_get_current_scene();
	if (!current_scene)
		return;

	const auto available_items = resolver->available_items();
	auto selected_target = resolver->resolve_selected_item(false);
	int64_t selected_item_id =
		selected_target
			? obs_sceneitem_get_id(selected_target->get())
			: settings_store.manual_target_item_id_for(
				  current_scene);
	if (selected_item_id == -1) {
		selected_item_id =
			available_items.empty()
				? kori::EntireSceneTargetId
				: available_items.front().item_id;
	}
	if (selected_item_id >= 0 &&
	    !resolver->resolve_item_by_id(selected_item_id)) {
		selected_item_id =
			available_items.empty()
				? kori::EntireSceneTargetId
				: available_items.front().item_id;
		settings_store.set_manual_target(current_scene,
						 selected_item_id);
	}

	std::vector<kori::SettingsTarget> targets;
	targets.push_back(
		{kori::EntireSceneTargetId, "Entire scene (everything)"});
	int64_t automatic_target_id =
		settings_store.automatic_target_item_id_for(current_scene);
	if (automatic_target_id == -1) {
		const int64_t legacy_target =
			settings_store.manual_target_item_id_for(current_scene);
		if (legacy_target == kori::EntireSceneTargetId) {
			const auto profile =
				settings_store.load_for_scene(current_scene);
			if (profile.activation ==
			    kori::ActivationMode::SceneActive) {
				settings_store.save_for_scene(current_scene,
							      profile);
				automatic_target_id = legacy_target;
			}
		} else {
			auto legacy_item =
				resolver->resolve_item_by_id(legacy_target);
			if (legacy_item) {
			const auto legacy_profile =
				settings_store.load_for(legacy_item->get());
			if (legacy_profile.activation ==
			    kori::ActivationMode::SceneActive) {
				settings_store.save_for(legacy_item->get(),
							legacy_profile);
				automatic_target_id = legacy_target;
			}
			}
		}
	}

	std::unordered_map<std::string, size_t> name_totals;
	std::unordered_map<std::string, size_t> name_instances;
	for (const auto &item : available_items)
		++name_totals[item.name];
	for (const auto &item : available_items) {
		std::string display_name = item.name;
		const size_t instance = ++name_instances[item.name];
		if (name_totals[item.name] > 1)
			display_name += " (instance " +
					std::to_string(instance) + ")";
		targets.push_back({item.item_id, display_name});
	}
	if (targets.empty())
		return;

	auto updated =
		selected_item_id == kori::EntireSceneTargetId
			? settings_store.load_for_scene(current_scene)
			: [&]() {
				  auto item = resolver->resolve_item_by_id(
					  selected_item_id);
				  return item
						 ? settings_store.load_for(
							   item->get())
						 : settings_store
							   .load_defaults();
			  }();
	const auto load_settings = [current_scene](const int64_t item_id) {
		if (item_id == kori::EntireSceneTargetId)
			return settings_store.load_for_scene(current_scene);
		auto item = resolver->resolve_item_by_id(item_id);
		return item ? settings_store.load_for(item->get())
			    : settings_store.load_defaults();
	};
	const auto source_for_target =
		[current_scene](const int64_t item_id) {
		if (item_id == kori::EntireSceneTargetId)
			return current_scene;
		auto item = resolver->resolve_item_by_id(item_id);
		return item ? obs_sceneitem_get_source(item->get()) : nullptr;
	};
	const auto save_settings =
		[parent, current_scene](const int64_t item_id,
			 const kori::AnimationSettings &updated) {
			if (item_id == kori::EntireSceneTargetId) {
				settings_store.save_for_scene(current_scene,
							      updated);
				engine->set_settings(updated);
				blog(LOG_INFO,
				     "[Kori] Whole-scene profile applied: scene='%s', zoom=%.2fx, duration=%.1fs, focus=(%.0f%%, %.0f%%)",
				     obs_source_get_name(current_scene),
				     updated.zoom_factor,
				     updated.zoom_duration,
				     updated.focus_x * 100.0F,
				     updated.focus_y * 100.0F);
				return true;
			}
			auto saved_target =
				resolver->resolve_item_by_id(item_id);
			if (!saved_target) {
				QMessageBox::warning(
					parent, "Kori",
					"The selected source is no longer available. No settings were saved.");
				return false;
			}
			settings_store.save_for(saved_target->get(), updated);
			engine->set_settings(updated);
			obs_source_t *source = obs_sceneitem_get_source(
				saved_target->get());
			blog(LOG_INFO,
			     "[Kori] Scene-item profile applied: target='%s', zoom=%.2fx, zoom duration=%.1fs, return duration=%.1fs, focus=(%.0f%%, %.0f%%)",
			     source ? obs_source_get_name(source) : "<unknown>",
			     updated.zoom_factor, updated.zoom_duration,
			     updated.return_duration,
			     updated.focus_x * 100.0F,
			     updated.focus_y * 100.0F);
			return true;
		};
	const auto preview_zoom =
		[current_scene](const int64_t item_id,
		   const kori::AnimationSettings &preview) {
		if (!engine)
			return;
		engine->set_settings(preview);
		if (item_id == kori::EntireSceneTargetId) {
			engine->play_scene(current_scene);
			return;
		}
		auto item = resolver->resolve_item_by_id(item_id);
		if (!item)
			return;
		engine->play(std::move(item));
	};
	const auto preview_return = []() {
		if (engine)
			engine->return_to_start();
	};
	kori::show_settings_dialog(
		parent, resolver->current_scene_name(), targets,
		selected_item_id, automatic_target_id, updated, load_settings,
		source_for_target, save_settings, preview_zoom, preview_return);
	obs_source_release(current_scene);
}

} // namespace

bool obs_module_load(void)
{
	engine = std::make_unique<kori::AnimationEngine>();
	resolver = std::make_unique<kori::MainCanvasTargetResolver>();
	engine->set_settings(settings_store.load_defaults());

	play_hotkey = obs_hotkey_register_frontend(
		"kori.play", "Kori: Play slow zoom", on_play_hotkey, nullptr);
	reset_hotkey = obs_hotkey_register_frontend(
		"kori.reset", "Kori: Return smoothly to start",
		on_reset_hotkey, nullptr);
	open_settings_hotkey = obs_hotkey_register_frontend(
		"kori.open_settings", "Kori: Open settings for current source",
		on_open_settings_hotkey, nullptr);
	obs_frontend_add_tools_menu_item("Kori Settings",
					 on_open_settings, nullptr);
	obs_frontend_add_event_callback(on_frontend_event, nullptr);
	obs_add_tick_callback(on_tick, nullptr);

	blog(LOG_INFO,
		"[Kori] Loaded version %s (standard OBS canvas)",
		KORI_VERSION);
	blog(LOG_INFO,
		"[Kori] Configure per-scene automation under Tools > Kori Settings; Play, Return and Open Settings hotkeys are available");
	return true;
}

void obs_module_unload(void)
{
	obs_frontend_remove_event_callback(on_frontend_event, nullptr);
	obs_remove_tick_callback(on_tick, nullptr);
	if (play_hotkey != OBS_INVALID_HOTKEY_ID)
		obs_hotkey_unregister(play_hotkey);
	if (reset_hotkey != OBS_INVALID_HOTKEY_ID)
		obs_hotkey_unregister(reset_hotkey);
	if (open_settings_hotkey != OBS_INVALID_HOTKEY_ID)
		obs_hotkey_unregister(open_settings_hotkey);
	if (engine)
		engine->restore_immediately();
	engine.reset();
	resolver.reset();
	blog(LOG_INFO, "[Kori] Unloaded");
}
