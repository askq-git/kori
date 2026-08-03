// Copyright (C) 2026 ASK Q Limited
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

struct obs_source;
typedef struct obs_source obs_source_t;

namespace kori {

void register_motion_blur_filter();
obs_source_t *create_motion_blur_filter(float focus_x, float focus_y);
void update_motion_blur_filter(obs_source_t *filter, float amount,
			       float focus_x, float focus_y);

} // namespace kori
