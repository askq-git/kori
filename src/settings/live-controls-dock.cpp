// Copyright (C) 2026 ASK Q Limited
// SPDX-License-Identifier: GPL-2.0-or-later

#include "live-controls-dock.hpp"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

namespace kori {

LiveControlsDock::LiveControlsDock(std::function<void()> play,
				   std::function<void()> return_to_start,
				   LoadSettings load_settings,
				   SaveSettings save_settings,
				   std::function<void()> open_settings,
				   QWidget *parent)
	: QWidget(parent)
{
	setObjectName("koriLiveControlsDock");
	setMinimumWidth(260);

	auto *layout = new QVBoxLayout(this);
	auto *intro = new QLabel("<b>Kori controls</b>", this);
	intro->setWordWrap(true);
	layout->addWidget(intro);
	auto *help = new QLabel(
		"Play the saved zoom, then refine its timing and movement below.",
		this);
	help->setWordWrap(true);
	layout->addWidget(help);

	auto *playback_label = new QLabel("<b>Playback</b>", this);
	layout->addWidget(playback_label);
	auto *playback_actions = new QHBoxLayout;
	auto *play_button = new QPushButton("Play zoom", this);
	play_button->setToolTip("Play the saved Kori zoom for this scene.");
	play_button->setStyleSheet(
		"QPushButton { background: #287a45; color: white; font-weight: bold; padding: 5px; }");
	QObject::connect(play_button, &QPushButton::clicked, this,
			 [play = std::move(play)]() { play(); });
	playback_actions->addWidget(play_button);

	auto *return_button = new QPushButton("Return", this);
	return_button->setToolTip("Return smoothly to the captured starting view.");
	QObject::connect(return_button, &QPushButton::clicked, this,
			 [return_to_start = std::move(return_to_start)]() {
				return_to_start();
			});
	playback_actions->addWidget(return_button);
	layout->addLayout(playback_actions);

	auto add_separator = [layout, this]() {
		auto *line = new QFrame(this);
		line->setFrameShape(QFrame::HLine);
		line->setFrameShadow(QFrame::Sunken);
		layout->addWidget(line);
	};
	add_separator();

	auto *refine_label = new QLabel("<b>Refine this zoom</b>", this);
	layout->addWidget(refine_label);

	auto *form = new QFormLayout;
	auto *zoom_factor = new QDoubleSpinBox(this);
	zoom_factor->setRange(1.05, 4.0);
	zoom_factor->setSingleStep(0.05);
	zoom_factor->setDecimals(2);
	zoom_factor->setSuffix(" x");
	zoom_factor->setToolTip("How close the zoom finishes.");
	form->addRow("Zoom amount", zoom_factor);

	auto *zoom_duration = new QDoubleSpinBox(this);
	zoom_duration->setRange(0.1, 60.0);
	zoom_duration->setSingleStep(0.1);
	zoom_duration->setDecimals(1);
	zoom_duration->setSuffix(" s");
	zoom_duration->setToolTip("How long Kori takes to zoom in.");
	form->addRow("Zoom-in time", zoom_duration);

	auto *return_duration = new QDoubleSpinBox(this);
	return_duration->setRange(0.1, 60.0);
	return_duration->setSingleStep(0.1);
	return_duration->setDecimals(1);
	return_duration->setSuffix(" s");
	return_duration->setToolTip("How long Kori takes to return to the starting view.");
	form->addRow("Return time", return_duration);

	auto *curve = new QComboBox(this);
	curve->addItem("Smooth", static_cast<int>(EasingPreset::Smooth));
	curve->addItem("Cinematic", static_cast<int>(EasingPreset::Cinematic));
	curve->addItem("Slow burn", static_cast<int>(EasingPreset::SlowBurn));
	curve->addItem("Punch", static_cast<int>(EasingPreset::Punch));
	curve->addItem("Linear", static_cast<int>(EasingPreset::Linear));
	curve->setToolTip("The movement curve for the zoom and return.");
	form->addRow("Movement curve", curve);
	layout->addLayout(form);

	auto apply_to_controls = [zoom_factor, zoom_duration, return_duration,
					  curve](const AnimationSettings &settings) {
		const QSignalBlocker block_zoom(zoom_factor);
		const QSignalBlocker block_zoom_time(zoom_duration);
		const QSignalBlocker block_return_time(return_duration);
		const QSignalBlocker block_curve(curve);
		zoom_factor->setValue(settings.zoom_factor);
		zoom_duration->setValue(settings.zoom_duration);
		return_duration->setValue(settings.return_duration);
		const int index = curve->findData(static_cast<int>(settings.easing));
		curve->setCurrentIndex(index < 0 ? 0 : index);
	};
	auto *notice = new QLabel(this);
	notice->setWordWrap(true);
	notice->setStyleSheet("color: #9a6400;");
	layout->addWidget(notice);
	auto *actions = new QHBoxLayout;
	auto *refresh_button = new QPushButton("Refresh", this);
	actions->addWidget(refresh_button);
	auto *apply_button = new QPushButton("Apply changes", this);
	apply_button->setEnabled(false);
	apply_button->setToolTip("Save these quick controls to the current Kori target.");
	apply_button->setStyleSheet(
		"QPushButton:enabled { background: #2d6ca2; color: white; font-weight: bold; padding: 5px; }");
	actions->addWidget(apply_button);
	layout->addLayout(actions);

	refresh_controls_ = [load_settings, apply_to_controls, notice, apply_button,
				     zoom_factor, zoom_duration, return_duration, curve]() {
		const auto settings = load_settings();
		if (!settings) {
			notice->setText("Select a source or save a Kori target in this scene first.");
			apply_button->setEnabled(false);
			zoom_factor->setEnabled(false);
			zoom_duration->setEnabled(false);
			return_duration->setEnabled(false);
			curve->setEnabled(false);
			return;
		}
		zoom_factor->setEnabled(true);
		zoom_duration->setEnabled(true);
		return_duration->setEnabled(true);
		curve->setEnabled(true);
		apply_to_controls(*settings);
		apply_button->setEnabled(false);
		notice->setText("Changes are not saved until you click Apply changes.");
	};
	QObject::connect(refresh_button, &QPushButton::clicked, this,
			 [this]() { refresh(); });
	auto mark_dirty = [notice, apply_button]() {
		apply_button->setEnabled(true);
		notice->setText("You have unsaved changes. Click Apply changes to commit them.");
	};
	QObject::connect(zoom_factor, qOverload<double>(&QDoubleSpinBox::valueChanged),
			 this, mark_dirty);
	QObject::connect(zoom_duration, qOverload<double>(&QDoubleSpinBox::valueChanged),
			 this, mark_dirty);
	QObject::connect(return_duration, qOverload<double>(&QDoubleSpinBox::valueChanged),
			 this, mark_dirty);
	QObject::connect(curve, qOverload<int>(&QComboBox::currentIndexChanged),
			 this, mark_dirty);
	QObject::connect(apply_button, &QPushButton::clicked, this,
			 [load_settings, save_settings, zoom_factor, zoom_duration,
			  return_duration, curve, notice, apply_button]() {
				const auto current = load_settings();
				if (!current) {
					notice->setText("Nothing to apply: select or configure a Kori target first.");
					apply_button->setEnabled(false);
					return;
				}
				auto updated = *current;
				updated.zoom_factor = static_cast<float>(zoom_factor->value());
				updated.zoom_duration = static_cast<float>(zoom_duration->value());
				updated.return_duration = static_cast<float>(return_duration->value());
				updated.easing = static_cast<EasingPreset>(curve->currentData().toInt());
				const bool saved = save_settings(updated);
				notice->setText(saved ? "Quick controls saved."
							       : "Kori could not save those controls for this target.");
				apply_button->setEnabled(!saved);
			});
	refresh();

	add_separator();
	auto *focus_button = new QPushButton("Set focus point…", this);
	focus_button->setToolTip("Open detailed settings to click the exact zoom point.");
	QObject::connect(focus_button, &QPushButton::clicked, this,
			 [open_settings]() { open_settings(); });
	layout->addWidget(focus_button);

	add_separator();
	auto *settings_button = new QPushButton("Open detailed settings", this);
	settings_button->setToolTip(
		"Choose a target and adjust focus, style and playback settings.");
	QObject::connect(settings_button, &QPushButton::clicked, this,
			 [open_settings = std::move(open_settings)]() {
				open_settings();
			});
	layout->addWidget(settings_button);
	layout->addStretch();
}

void LiveControlsDock::refresh()
{
	if (refresh_controls_)
		refresh_controls_();
}

} // namespace kori
