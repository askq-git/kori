// Copyright (C) 2026 ASK Q Limited
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <obs.h>

namespace kori {

class SceneItemTarget {
public:
	explicit SceneItemTarget(obs_sceneitem_t *item) : item_(item)
	{
		if (item_)
			obs_sceneitem_addref(item_);
	}

	~SceneItemTarget()
	{
		if (item_)
			obs_sceneitem_release(item_);
	}

	SceneItemTarget(const SceneItemTarget &) = delete;
	SceneItemTarget &operator=(const SceneItemTarget &) = delete;

	obs_sceneitem_t *get() const { return item_; }

private:
	obs_sceneitem_t *item_ = nullptr;
};

} // namespace kori

