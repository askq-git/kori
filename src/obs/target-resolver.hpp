// Copyright (C) 2026 ASKQ
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <memory>
#include <cstdint>

namespace kori {

class SceneItemTarget;

class TargetResolver {
public:
	virtual ~TargetResolver() = default;
	virtual std::unique_ptr<SceneItemTarget>
	resolve_selected_item(bool log_missing = true) = 0;
	virtual std::unique_ptr<SceneItemTarget>
	resolve_item_by_id(int64_t item_id) = 0;
};

} // namespace kori
