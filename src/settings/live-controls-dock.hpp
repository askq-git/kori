// Copyright (C) 2026 ASK Q Limited
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "../core/animation-settings.hpp"

#include <functional>
#include <optional>
#include <QWidget>

namespace kori {

class LiveControlsDock final : public QWidget {
public:
	using LoadSettings = std::function<std::optional<AnimationSettings>()>;
	using SaveSettings = std::function<bool(const AnimationSettings &)>;

	LiveControlsDock(std::function<void()> play,
			 std::function<void()> return_to_start,
			 LoadSettings load_settings,
			 SaveSettings save_settings,
			 std::function<void()> open_settings,
			 QWidget *parent = nullptr);

	void refresh();

private:
	std::function<void()> refresh_controls_;
};

} // namespace kori
