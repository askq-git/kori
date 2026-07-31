// Copyright (C) 2026 ASK Q Limited
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "target-resolver.hpp"

#include <string>
#include <vector>

namespace kori {

struct SceneTargetInfo {
	int64_t item_id;
	std::string name;
};

class MainCanvasTargetResolver final : public TargetResolver {
public:
	std::unique_ptr<SceneItemTarget>
	resolve_selected_item(bool log_missing = true) override;
	std::unique_ptr<SceneItemTarget>
	resolve_item_by_id(int64_t item_id) override;
	std::vector<SceneTargetInfo> available_items() const;
	std::string current_scene_name() const;
};

} // namespace kori
