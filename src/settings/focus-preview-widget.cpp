// Copyright (C) 2026 ASK Q Limited
// SPDX-License-Identifier: GPL-2.0-or-later

#include "focus-preview-widget.hpp"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QSizePolicy>
#include <QWindow>

#include <algorithm>
#include <cmath>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <graphics/graphics.h>

namespace kori {

FocusPreviewWidget::FocusPreviewWidget(obs_source_t *source, QWidget *parent)
	: QWidget(parent), source_(source ? obs_source_get_ref(source) : nullptr)
{
	if (source_) {
		obs_source_inc_showing(source_);
	}

	setFixedSize(480, 270);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setCursor(Qt::CrossCursor);
	setMouseTracking(true);
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_StaticContents);
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setAttribute(Qt::WA_DontCreateNativeAncestors);
	setAttribute(Qt::WA_NativeWindow);

	create_target_primitives();
}

FocusPreviewWidget::~FocusPreviewWidget()
{
	if (display_) {
		obs_display_remove_draw_callback(display_, draw_preview, this);
		obs_display_destroy(display_);
		display_ = nullptr;
	}
	destroy_target_primitives();
	if (source_) {
		obs_source_dec_showing(source_);
		obs_source_release(source_);
		source_ = nullptr;
	}
}

void FocusPreviewWidget::set_source(obs_source_t *source)
{
	if (source == source_)
		return;

	if (display_)
		obs_display_remove_draw_callback(display_, draw_preview, this);
	if (source_) {
		obs_source_dec_showing(source_);
		obs_source_release(source_);
	}
	source_ = source ? obs_source_get_ref(source) : nullptr;
	if (source_) {
		obs_source_inc_showing(source_);
	}
	if (display_)
		obs_display_add_draw_callback(display_, draw_preview, this);
}

void FocusPreviewWidget::create_target_primitives()
{
	obs_enter_graphics();

	gs_render_start(true);
	for (int angle = 0; angle <= 360; angle += 10) {
		const float radians =
			static_cast<float>(angle) * 3.14159265358979323846F /
			180.0F;
		gs_vertex2f(std::cos(radians), std::sin(radians));
	}
	target_circle_ = gs_render_save();

	gs_render_start(true);
	gs_vertex2f(-1.75F, 0.0F);
	gs_vertex2f(-1.08F, 0.0F);
	gs_vertex2f(1.08F, 0.0F);
	gs_vertex2f(1.75F, 0.0F);
	gs_vertex2f(0.0F, -1.75F);
	gs_vertex2f(0.0F, -1.08F);
	gs_vertex2f(0.0F, 1.08F);
	gs_vertex2f(0.0F, 1.75F);
	target_cross_ = gs_render_save();

	obs_leave_graphics();
}

void FocusPreviewWidget::destroy_target_primitives()
{
	if (!target_circle_ && !target_cross_)
		return;

	obs_enter_graphics();
	if (target_circle_)
		gs_vertexbuffer_destroy(target_circle_);
	if (target_cross_)
		gs_vertexbuffer_destroy(target_cross_);
	target_circle_ = nullptr;
	target_cross_ = nullptr;
	obs_leave_graphics();
}

QPaintEngine *FocusPreviewWidget::paintEngine() const
{
	return nullptr;
}

void FocusPreviewWidget::create_display()
{
	if (display_ || !isVisible() || !windowHandle() ||
	    !windowHandle()->isExposed())
		return;

	const qreal ratio = devicePixelRatioF();
	gs_init_data info{};
	info.cx = static_cast<uint32_t>(std::max(1.0, width() * ratio));
	info.cy = static_cast<uint32_t>(std::max(1.0, height() * ratio));
	info.format = GS_BGRA;
	info.zsformat = GS_ZS_NONE;
	info.window.hwnd = reinterpret_cast<HWND>(winId());

	display_ = obs_display_create(&info, 0xFF202020);
	if (display_)
		obs_display_add_draw_callback(display_, draw_preview, this);
}

void FocusPreviewWidget::source_rect(const float widget_width,
				     const float widget_height, float &x,
				     float &y, float &rect_width,
				     float &rect_height) const
{
	const float source_width = static_cast<float>(
		std::max(source_ ? obs_source_get_width(source_) : 0U, 1U));
	const float source_height = static_cast<float>(
		std::max(source_ ? obs_source_get_height(source_) : 0U, 1U));
	const float scale = std::min(widget_width / source_width,
				     widget_height / source_height);
	rect_width = source_width * scale;
	rect_height = source_height * scale;
	x = (widget_width - rect_width) * 0.5F;
	y = (widget_height - rect_height) * 0.5F;
}

void FocusPreviewWidget::draw_preview(void *data, const uint32_t width,
				      const uint32_t height)
{
	auto *widget = static_cast<FocusPreviewWidget *>(data);
	if (!widget->source_)
		return;

	float x;
	float y;
	float draw_width;
	float draw_height;
	widget->source_rect(static_cast<float>(width),
			    static_cast<float>(height), x, y, draw_width,
			    draw_height);
	const uint32_t source_width =
		std::max(obs_source_get_width(widget->source_), 1U);
	const uint32_t source_height =
		std::max(obs_source_get_height(widget->source_), 1U);

	gs_viewport_push();
	gs_projection_push();
	const bool previous_srgb = gs_set_linear_srgb(true);
	gs_ortho(0.0F, static_cast<float>(source_width), 0.0F,
		 static_cast<float>(source_height), -100.0F, 100.0F);
	gs_set_viewport(static_cast<int>(x), static_cast<int>(y),
			static_cast<int>(draw_width),
			static_cast<int>(draw_height));
	obs_source_video_render(widget->source_);
	gs_set_linear_srgb(previous_srgb);
	gs_projection_pop();
	gs_viewport_pop();

	if (!widget->target_circle_ || !widget->target_cross_)
		return;

	const float target_x =
		x + widget->focus_x_.load() * draw_width;
	const float target_y =
		y + widget->focus_y_.load() * draw_height;
	gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
	gs_eparam_t *color = gs_effect_get_param_by_name(solid, "color");
	gs_technique_t *technique =
		gs_effect_get_technique(solid, "Solid");
	vec4 target_color{};
	vec4_set(&target_color, 1.0F, 0.18F, 0.12F, 0.95F);
	gs_effect_set_vec4(color, &target_color);

	gs_viewport_push();
	gs_projection_push();
	gs_set_viewport(0, 0, static_cast<int>(width),
			static_cast<int>(height));
	gs_ortho(0.0F, static_cast<float>(width), 0.0F,
		 static_cast<float>(height), -100.0F, 100.0F);
	gs_technique_begin(technique);
	gs_technique_begin_pass(technique, 0);

	gs_matrix_push();
	gs_matrix_identity();
	gs_matrix_translate3f(target_x, target_y, 0.0F);
	gs_matrix_scale3f(11.0F, 11.0F, 1.0F);
	gs_load_vertexbuffer(widget->target_circle_);
	gs_draw(GS_LINESTRIP, 0, 0);
	gs_matrix_pop();

	gs_matrix_push();
	gs_matrix_identity();
	gs_matrix_translate3f(target_x, target_y, 0.0F);
	gs_matrix_scale3f(2.5F, 2.5F, 1.0F);
	gs_load_vertexbuffer(widget->target_circle_);
	gs_draw(GS_LINESTRIP, 0, 0);
	gs_matrix_pop();

	gs_matrix_push();
	gs_matrix_identity();
	gs_matrix_translate3f(target_x, target_y, 0.0F);
	gs_matrix_scale3f(11.0F, 11.0F, 1.0F);
	gs_load_vertexbuffer(widget->target_cross_);
	gs_draw(GS_LINES, 0, 0);
	gs_matrix_pop();

	gs_load_vertexbuffer(nullptr);
	gs_technique_end_pass(technique);
	gs_technique_end(technique);
	gs_projection_pop();
	gs_viewport_pop();
}

void FocusPreviewWidget::set_focus(const float x, const float y)
{
	focus_x_.store(std::clamp(x, 0.0F, 1.0F));
	focus_y_.store(std::clamp(y, 0.0F, 1.0F));
}

void FocusPreviewWidget::update_focus_from_position(const QPointF &position,
						    const bool clamp)
{
	float x;
	float y;
	float draw_width;
	float draw_height;
	source_rect(static_cast<float>(width()), static_cast<float>(height()), x,
		    y, draw_width, draw_height);
	if (!clamp &&
	    (position.x() < x || position.x() > x + draw_width ||
	     position.y() < y || position.y() > y + draw_height))
		return;

	set_focus(
		std::clamp(static_cast<float>((position.x() - x) / draw_width),
			   0.0F, 1.0F),
		std::clamp(static_cast<float>((position.y() - y) / draw_height),
			   0.0F, 1.0F));
	if (change_callback_)
		change_callback_(focus_x(), focus_y());
}

void FocusPreviewWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		dragging_ = true;
		update_focus_from_position(event->position(), false);
	}
	QWidget::mousePressEvent(event);
}

void FocusPreviewWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (dragging_ && (event->buttons() & Qt::LeftButton))
		update_focus_from_position(event->position(), true);
	QWidget::mouseMoveEvent(event);
}

void FocusPreviewWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
		dragging_ = false;
	QWidget::mouseReleaseEvent(event);
}

void FocusPreviewWidget::paintEvent(QPaintEvent *event)
{
	create_display();
	QWidget::paintEvent(event);
}

void FocusPreviewWidget::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	create_display();
	if (display_) {
		const qreal ratio = devicePixelRatioF();
		obs_display_resize(
			display_,
			static_cast<uint32_t>(std::max(1.0, width() * ratio)),
			static_cast<uint32_t>(std::max(1.0, height() * ratio)));
	}
}

void FocusPreviewWidget::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	create_display();
}

} // namespace kori
