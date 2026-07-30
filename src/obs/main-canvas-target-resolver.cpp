// Copyright (C) 2026 ASKQ
// SPDX-License-Identifier: GPL-2.0-or-later

#include "main-canvas-target-resolver.hpp"

#include "../core/scene-item-target.hpp"

#include <obs-frontend-api.h>
#include <obs-module.h>

namespace {

bool find_selected_item(obs_scene_t *, obs_sceneitem_t *item, void *data)
{
	auto **selected = static_cast<obs_sceneitem_t **>(data);
	if (obs_sceneitem_selected(item)) {
		*selected = item;
		return false;
	}
	return true;
}

bool collect_items(obs_scene_t *, obs_sceneitem_t *item, void *data)
{
	auto *items = static_cast<std::vector<kori::SceneTargetInfo> *>(data);
	obs_source_t *source = obs_sceneitem_get_source(item);
	const char *name = source ? obs_source_get_name(source) : nullptr;
	items->push_back(
		{obs_sceneitem_get_id(item), name ? name : "<unnamed source>"});
	return true;
}

} // namespace

namespace kori {

std::unique_ptr<SceneItemTarget>
MainCanvasTargetResolver::resolve_selected_item(const bool log_missing)
{
	obs_source_t *scene_source = obs_frontend_get_current_scene();
	if (!scene_source) {
		blog(LOG_WARNING, "[Kori] No active main-canvas scene");
		return nullptr;
	}

	obs_scene_t *scene = obs_scene_from_source(scene_source);
	obs_sceneitem_t *selected = nullptr;
	if (scene)
		obs_scene_enum_items(scene, find_selected_item, &selected);

	obs_source_release(scene_source);

	if (!selected && log_missing) {
		blog(LOG_WARNING,
			"[Kori] Play ignored: select one item in the active scene first");
	}

	if (!selected)
		return nullptr;
	return std::make_unique<SceneItemTarget>(selected);
}

std::unique_ptr<SceneItemTarget>
MainCanvasTargetResolver::resolve_item_by_id(const int64_t item_id)
{
	obs_source_t *scene_source = obs_frontend_get_current_scene();
	if (!scene_source)
		return nullptr;

	obs_scene_t *scene = obs_scene_from_source(scene_source);
	obs_sceneitem_t *item =
		scene ? obs_scene_find_sceneitem_by_id(scene, item_id) : nullptr;
	auto target =
		item ? std::make_unique<SceneItemTarget>(item) : nullptr;
	obs_source_release(scene_source);
	return target;
}

std::vector<SceneTargetInfo>
MainCanvasTargetResolver::available_items() const
{
	std::vector<SceneTargetInfo> items;
	obs_source_t *scene_source = obs_frontend_get_current_scene();
	if (!scene_source)
		return items;
	obs_scene_t *scene = obs_scene_from_source(scene_source);
	if (scene)
		obs_scene_enum_items(scene, collect_items, &items);
	obs_source_release(scene_source);
	return items;
}

std::string MainCanvasTargetResolver::current_scene_name() const
{
	obs_source_t *scene_source = obs_frontend_get_current_scene();
	if (!scene_source)
		return {};
	const char *name = obs_source_get_name(scene_source);
	std::string result = name ? name : "";
	obs_source_release(scene_source);
	return result;
}

} // namespace kori
