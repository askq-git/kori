// Copyright (C) 2026 ASK Q Limited
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QWidget>

#include <atomic>
#include <functional>
#include <graphics/graphics.h>
#include <obs.h>

class QMouseEvent;
class QPointF;
class QPaintEngine;
class QResizeEvent;
class QShowEvent;

namespace kori {

class FocusPreviewWidget final : public QWidget {
public:
	explicit FocusPreviewWidget(obs_source_t *source, QWidget *parent = nullptr);
	~FocusPreviewWidget() override;

	void set_source(obs_source_t *source);
	void set_focus(float x, float y);
	void set_change_callback(std::function<void(float, float)> callback)
	{
		change_callback_ = std::move(callback);
	}
	float focus_x() const { return focus_x_.load(); }
	float focus_y() const { return focus_y_.load(); }

	QPaintEngine *paintEngine() const override;

private:
	static void draw_preview(void *data, uint32_t width, uint32_t height);
	void create_display();
	void create_target_primitives();
	void destroy_target_primitives();
	void update_focus_from_position(const QPointF &position, bool clamp);
	void source_rect(float widget_width, float widget_height, float &x,
			 float &y, float &width, float &height) const;

	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void paintEvent(QPaintEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;
	void showEvent(QShowEvent *event) override;

	obs_source_t *source_ = nullptr;
	obs_display_t *display_ = nullptr;
	gs_vertbuffer_t *target_circle_ = nullptr;
	gs_vertbuffer_t *target_cross_ = nullptr;
	std::atomic<float> focus_x_{0.5F};
	std::atomic<float> focus_y_{0.28F};
	bool dragging_ = false;
	std::function<void(float, float)> change_callback_;
};

} // namespace kori
