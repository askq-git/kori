// Copyright (C) 2026 ASK Q Limited
// SPDX-License-Identifier: GPL-2.0-or-later

#include "settings-dialog.hpp"

#include "focus-preview-widget.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QFormLayout>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <cmath>

namespace kori {

void show_settings_dialog(
	QWidget *parent, const std::string &scene_name,
	const std::vector<SettingsTarget> &targets, int64_t target_item_id,
	int64_t automatic_target_item_id, const AnimationSettings &settings,
	const std::function<AnimationSettings(int64_t)> &load_settings,
	const std::function<obs_source_t *(int64_t)> &source_for_target,
	const std::function<bool(int64_t, const AnimationSettings &)> &save_settings,
	const std::function<void(int64_t, const AnimationSettings &)> &preview_zoom,
	const std::function<void()> &preview_return)
{
	QDialog dialog(parent);
	dialog.setWindowTitle(
		QString("Kori Settings — %1").arg(KORI_VERSION));
	dialog.setMinimumSize(560, 650);
	dialog.resize(600, 860);

	auto *layout = new QVBoxLayout(&dialog);
	layout->setAlignment(Qt::AlignTop);
	auto *intro = new QLabel("Click the subject's face in the preview.", &dialog);
	intro->setWordWrap(true);
	layout->addWidget(intro);

	auto *scene_label =
		new QLabel(QString("Scene: <b>%1</b>")
				   .arg(QString::fromStdString(scene_name).toHtmlEscaped()),
			   &dialog);
	layout->addWidget(scene_label);

	auto *target = new QComboBox(&dialog);
	int initial_target_index = 0;
	for (size_t index = 0; index < targets.size(); ++index) {
		QString label = QString::fromStdString(targets[index].name);
		if (targets[index].item_id == automatic_target_item_id)
			label += " — automatic target";
		target->addItem(label,
				QVariant::fromValue<qlonglong>(
					targets[index].item_id));
		if (targets[index].item_id == target_item_id)
			initial_target_index = static_cast<int>(index);
	}
	target->setCurrentIndex(initial_target_index);
	int current_target_index = initial_target_index;
	int64_t current_target_id =
		targets[static_cast<size_t>(initial_target_index)].item_id;
	bool dirty = false;
	layout->addWidget(new QLabel("Target source", &dialog));
	layout->addWidget(target);
	auto *target_help = new QLabel(
		"Play uses this target by default. One target per scene can also "
		"run automatically.",
		&dialog);
	target_help->setWordWrap(true);
	layout->addWidget(target_help);
	auto *save_status = new QLabel(" ", &dialog);
	layout->addWidget(save_status);

	auto *preview =
		new FocusPreviewWidget(source_for_target(current_target_id), &dialog);
	preview->set_focus(settings.focus_x, settings.focus_y);
	layout->addWidget(preview, 0, Qt::AlignHCenter);

	auto *preview_buttons = new QDialogButtonBox(&dialog);
	auto *play_preview =
		preview_buttons->addButton("Preview Zoom", QDialogButtonBox::ActionRole);
	auto *return_preview =
		preview_buttons->addButton("Preview Return", QDialogButtonBox::ActionRole);
	layout->addWidget(preview_buttons);

	auto *settings_scroll = new QScrollArea(&dialog);
	settings_scroll->setWidgetResizable(true);
	settings_scroll->setFrameShape(QFrame::NoFrame);
	auto *settings_panel = new QWidget(settings_scroll);
	auto *form = new QFormLayout(settings_panel);
	auto *zoom = new QDoubleSpinBox(&dialog);
	zoom->setRange(1.01, 3.00);
	zoom->setSingleStep(0.05);
	zoom->setDecimals(2);
	zoom->setSuffix("x");
	zoom->setValue(settings.zoom_factor);

	auto *duration = new QDoubleSpinBox(&dialog);
	duration->setRange(0.2, 30.0);
	duration->setSingleStep(0.5);
	duration->setSuffix(" seconds");
	duration->setValue(settings.zoom_duration);

	auto *return_duration = new QDoubleSpinBox(&dialog);
	return_duration->setRange(0.2, 30.0);
	return_duration->setSingleStep(0.5);
	return_duration->setSuffix(" seconds");
	return_duration->setValue(settings.return_duration);

	auto *easing = new QComboBox(&dialog);
	easing->addItems({"Smooth (current)", "Cinematic", "Slow Burn",
			  "Punch (overshoot)", "Linear"});
	easing->setCurrentIndex(static_cast<int>(settings.easing));

	auto *activation = new QComboBox(&dialog);
	activation->addItems({"Manual / hotkey only",
			      "Automatically when scene becomes active"});
	activation->setCurrentIndex(static_cast<int>(settings.activation));

	auto *start_delay = new QDoubleSpinBox(&dialog);
	start_delay->setRange(0.0, 30.0);
	start_delay->setSingleStep(0.5);
	start_delay->setSuffix(" seconds");
	start_delay->setValue(settings.start_delay);

	auto *completion = new QComboBox(&dialog);
	completion->addItems({"Stay zoomed", "Hold, then return"});
	completion->setCurrentIndex(static_cast<int>(settings.completion));

	auto *hold_duration = new QDoubleSpinBox(&dialog);
	hold_duration->setRange(0.0, 30.0);
	hold_duration->setSingleStep(0.5);
	hold_duration->setSuffix(" seconds");
	hold_duration->setValue(settings.hold_duration);

	auto *reset_on_exit =
		new QCheckBox("Reset framing when scene becomes inactive", &dialog);
	reset_on_exit->setChecked(settings.reset_on_scene_exit);

	auto *preset = new QComboBox(&dialog);
	preset->addItems({"Upper centre (face)", "Centre", "Upper left",
			  "Upper right", "Custom"});

	auto *advanced = new QCheckBox("Show advanced focus percentages", &dialog);
	auto *focus_x = new QDoubleSpinBox(&dialog);
	focus_x->setRange(0.0, 100.0);
	focus_x->setSuffix("%");
	focus_x->setValue(settings.focus_x * 100.0);

	auto *focus_y = new QDoubleSpinBox(&dialog);
	focus_y->setRange(0.0, 100.0);
	focus_y->setSuffix("%");
	focus_y->setValue(settings.focus_y * 100.0);

	const auto set_focus = [focus_x, focus_y, preview](const int index) {
		const QSignalBlocker block_x(focus_x);
		const QSignalBlocker block_y(focus_y);
		switch (index) {
		case 0:
			focus_x->setValue(50.0);
			focus_y->setValue(28.0);
			break;
		case 1:
			focus_x->setValue(50.0);
			focus_y->setValue(50.0);
			break;
		case 2:
			focus_x->setValue(35.0);
			focus_y->setValue(28.0);
			break;
		case 3:
			focus_x->setValue(65.0);
			focus_y->setValue(28.0);
			break;
		default:
			break;
		}
		preview->set_focus(static_cast<float>(focus_x->value() / 100.0),
				   static_cast<float>(focus_y->value() / 100.0));
	};
	QObject::connect(preset, &QComboBox::currentIndexChanged, &dialog,
			 set_focus);
	QObject::connect(focus_x, &QDoubleSpinBox::valueChanged, &dialog,
			 [preset, preview, focus_x, focus_y](double) {
				 preset->setCurrentIndex(4);
				 preview->set_focus(
					 static_cast<float>(focus_x->value() /
							    100.0),
					 static_cast<float>(focus_y->value() /
							    100.0));
			 });
	QObject::connect(focus_y, &QDoubleSpinBox::valueChanged, &dialog,
			 [preset, preview, focus_x, focus_y](double) {
				 preset->setCurrentIndex(4);
				 preview->set_focus(
					 static_cast<float>(focus_x->value() /
							    100.0),
					 static_cast<float>(focus_y->value() /
							    100.0));
			 });
	QObject::connect(advanced, &QCheckBox::toggled, &dialog,
			 [form, focus_x, focus_y](const bool visible) {
				 form->setRowVisible(focus_x, visible);
				 form->setRowVisible(focus_y, visible);
			 });
	preview->set_change_callback(
		[focus_x, focus_y, preset, &dirty](const float x, const float y) {
			const QSignalBlocker block_x(focus_x);
			const QSignalBlocker block_y(focus_y);
			focus_x->setValue(x * 100.0);
			focus_y->setValue(y * 100.0);
			preset->setCurrentIndex(4);
			dirty = true;
		});

	const auto near = [](const float left, const float right) {
		return std::abs(left - right) < 0.005F;
	};
	if (near(settings.focus_x, 0.50F) &&
	    near(settings.focus_y, 0.28F))
		preset->setCurrentIndex(0);
	else if (near(settings.focus_x, 0.50F) &&
		 near(settings.focus_y, 0.50F))
		preset->setCurrentIndex(1);
	else if (near(settings.focus_x, 0.35F) &&
		 near(settings.focus_y, 0.28F))
		preset->setCurrentIndex(2);
	else if (near(settings.focus_x, 0.65F) &&
		 near(settings.focus_y, 0.28F))
		preset->setCurrentIndex(3);
	else
		preset->setCurrentIndex(4);

	form->addRow("Zoom amount", zoom);
	form->addRow("Zoom duration", duration);
	form->addRow("Return duration", return_duration);
	form->addRow("Motion style", easing);
	form->addRow("Activation", activation);
	form->addRow("Start delay", start_delay);
	form->addRow("After automatic zoom", completion);
	form->addRow("Hold duration", hold_duration);
	form->addRow(reset_on_exit);
	form->addRow("Focus preset", preset);
	form->addRow(advanced);
	form->addRow("Focus horizontal", focus_x);
	form->addRow("Focus vertical", focus_y);
	form->setRowVisible(focus_x, false);
	form->setRowVisible(focus_y, false);
	settings_scroll->setWidget(settings_panel);
	layout->addWidget(settings_scroll, 1);

	const auto update_automation_controls =
		[activation, start_delay, completion, hold_duration,
		 reset_on_exit]() {
			const bool automatic =
				activation->currentIndex() ==
				static_cast<int>(ActivationMode::SceneActive);
			start_delay->setEnabled(automatic);
			completion->setEnabled(automatic);
			hold_duration->setEnabled(
				automatic &&
				completion->currentIndex() ==
					static_cast<int>(
						CompletionMode::AutoReturn));
			reset_on_exit->setEnabled(automatic);
		};
	QObject::connect(activation, &QComboBox::currentIndexChanged, &dialog,
			 [update_automation_controls](int) {
				 update_automation_controls();
			 });
	QObject::connect(completion, &QComboBox::currentIndexChanged, &dialog,
			 [update_automation_controls](int) {
				 update_automation_controls();
			 });
	update_automation_controls();

	auto collect_settings = [=]() {
		AnimationSettings current;
		current.zoom_factor = static_cast<float>(zoom->value());
		current.zoom_duration = static_cast<float>(duration->value());
		current.return_duration =
			static_cast<float>(return_duration->value());
		current.focus_x = preview->focus_x();
		current.focus_y = preview->focus_y();
		current.easing =
			static_cast<EasingPreset>(easing->currentIndex());
		current.activation =
			static_cast<ActivationMode>(activation->currentIndex());
		current.completion =
			static_cast<CompletionMode>(completion->currentIndex());
		current.start_delay = static_cast<float>(start_delay->value());
		current.hold_duration =
			static_cast<float>(hold_duration->value());
		current.reset_on_scene_exit = reset_on_exit->isChecked();
		return current;
	};

	const auto apply_settings =
		[=](const AnimationSettings &current) {
			const QSignalBlocker block_zoom(zoom);
			const QSignalBlocker block_duration(duration);
			const QSignalBlocker block_return(return_duration);
			const QSignalBlocker block_easing(easing);
			const QSignalBlocker block_activation(activation);
			const QSignalBlocker block_start_delay(start_delay);
			const QSignalBlocker block_completion(completion);
			const QSignalBlocker block_hold(hold_duration);
			const QSignalBlocker block_reset(reset_on_exit);
			const QSignalBlocker block_preset(preset);
			const QSignalBlocker block_x(focus_x);
			const QSignalBlocker block_y(focus_y);

			zoom->setValue(current.zoom_factor);
			duration->setValue(current.zoom_duration);
			return_duration->setValue(current.return_duration);
			easing->setCurrentIndex(static_cast<int>(current.easing));
			activation->setCurrentIndex(
				static_cast<int>(current.activation));
			start_delay->setValue(current.start_delay);
			completion->setCurrentIndex(
				static_cast<int>(current.completion));
			hold_duration->setValue(current.hold_duration);
			reset_on_exit->setChecked(current.reset_on_scene_exit);
			focus_x->setValue(current.focus_x * 100.0);
			focus_y->setValue(current.focus_y * 100.0);
			preview->set_focus(current.focus_x, current.focus_y);

			if (near(current.focus_x, 0.50F) &&
			    near(current.focus_y, 0.28F))
				preset->setCurrentIndex(0);
			else if (near(current.focus_x, 0.50F) &&
				 near(current.focus_y, 0.50F))
				preset->setCurrentIndex(1);
			else if (near(current.focus_x, 0.35F) &&
				 near(current.focus_y, 0.28F))
				preset->setCurrentIndex(2);
			else if (near(current.focus_x, 0.65F) &&
				 near(current.focus_y, 0.28F))
				preset->setCurrentIndex(3);
			else
				preset->setCurrentIndex(4);
			update_automation_controls();
		};

	const auto mark_dirty = [&dirty, save_status]() {
		dirty = true;
		save_status->setText("Unsaved changes");
	};
	QObject::connect(zoom, &QDoubleSpinBox::valueChanged, &dialog,
			 [mark_dirty](double) { mark_dirty(); });
	QObject::connect(duration, &QDoubleSpinBox::valueChanged, &dialog,
			 [mark_dirty](double) { mark_dirty(); });
	QObject::connect(return_duration, &QDoubleSpinBox::valueChanged,
			 &dialog, [mark_dirty](double) { mark_dirty(); });
	QObject::connect(easing, &QComboBox::currentIndexChanged, &dialog,
			 [mark_dirty](int) { mark_dirty(); });
	QObject::connect(activation, &QComboBox::currentIndexChanged, &dialog,
			 [mark_dirty](int) { mark_dirty(); });
	QObject::connect(start_delay, &QDoubleSpinBox::valueChanged, &dialog,
			 [mark_dirty](double) { mark_dirty(); });
	QObject::connect(completion, &QComboBox::currentIndexChanged, &dialog,
			 [mark_dirty](int) { mark_dirty(); });
	QObject::connect(hold_duration, &QDoubleSpinBox::valueChanged, &dialog,
			 [mark_dirty](double) { mark_dirty(); });
	QObject::connect(reset_on_exit, &QCheckBox::toggled, &dialog,
			 [mark_dirty](bool) { mark_dirty(); });
	QObject::connect(preset, &QComboBox::currentIndexChanged, &dialog,
			 [mark_dirty](int) { mark_dirty(); });
	QObject::connect(focus_x, &QDoubleSpinBox::valueChanged, &dialog,
			 [mark_dirty](double) { mark_dirty(); });
	QObject::connect(focus_y, &QDoubleSpinBox::valueChanged, &dialog,
			 [mark_dirty](double) { mark_dirty(); });

	const auto refresh_target_labels =
		[&targets, target, &automatic_target_item_id]() {
			const QSignalBlocker blocker(target);
			for (size_t index = 0; index < targets.size(); ++index) {
				QString label =
					QString::fromStdString(targets[index].name);
				if (targets[index].item_id ==
				    automatic_target_item_id)
					label += " — automatic target";
				target->setItemText(static_cast<int>(index),
						    label);
			}
		};

	const auto save_current =
		[&]() {
			const AnimationSettings current = collect_settings();
			if (!save_settings(current_target_id, current))
				return false;
			if (current.activation ==
			    ActivationMode::SceneActive)
				automatic_target_item_id = current_target_id;
			else if (automatic_target_item_id == current_target_id)
				automatic_target_item_id = -1;
			refresh_target_labels();
			dirty = false;
			save_status->setText(
				QString("Applied to %1")
					.arg(QString::fromStdString(
						targets[static_cast<size_t>(
							current_target_index)]
							.name)));
			return true;
		};

	QObject::connect(
		target, &QComboBox::currentIndexChanged, &dialog,
		[&](const int index) {
			if (index < 0 ||
			    index >= static_cast<int>(targets.size()))
				return;

			if (dirty) {
				const auto choice = QMessageBox::warning(
					&dialog, "Unsaved Kori changes",
					QString("Save changes to %1?")
						.arg(QString::fromStdString(
							targets[static_cast<size_t>(
								current_target_index)]
								.name)),
					QMessageBox::Save |
						QMessageBox::Discard |
						QMessageBox::Cancel,
					QMessageBox::Save);
				if (choice == QMessageBox::Save &&
				    !save_current()) {
					const QSignalBlocker blocker(target);
					target->setCurrentIndex(
						current_target_index);
					return;
				}
				if (choice == QMessageBox::Cancel) {
					const QSignalBlocker blocker(target);
					target->setCurrentIndex(
						current_target_index);
					return;
				}
				dirty = false;
			}

			const int64_t id = targets[static_cast<size_t>(index)].item_id;
			current_target_index = index;
			current_target_id = id;
			preview->set_source(source_for_target(id));
			apply_settings(load_settings(id));
			dirty = false;
			save_status->setText(
				QString("Loaded %1")
					.arg(QString::fromStdString(
						targets[static_cast<size_t>(index)]
							.name)));
		});

	QObject::connect(play_preview, &QPushButton::clicked, &dialog,
			 [collect_settings, preview_zoom,
			  &current_target_id]() {
				 preview_zoom(current_target_id,
					      collect_settings());
			 });
	QObject::connect(return_preview, &QPushButton::clicked, &dialog,
			 [preview_return]() { preview_return(); });
	auto *separator = new QFrame(&dialog);
	separator->setFrameShape(QFrame::HLine);
	separator->setFrameShadow(QFrame::Sunken);
	layout->addSpacing(8);
	layout->addWidget(separator);
	layout->addSpacing(4);

	auto *buttons =
		new QDialogButtonBox(QDialogButtonBox::Cancel, &dialog);
	auto *apply =
		buttons->addButton("Apply", QDialogButtonBox::ApplyRole);
	auto *save_close =
		buttons->addButton("Save && Close",
				   QDialogButtonBox::AcceptRole);
	QObject::connect(apply, &QPushButton::clicked, &dialog,
			 [save_current]() { save_current(); });
	QObject::connect(save_close, &QPushButton::clicked, &dialog,
			 [&dialog, save_current]() {
				 if (save_current())
					 dialog.accept();
			 });
	QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog,
			 &QDialog::reject);
	layout->addWidget(buttons);

	dialog.exec();
}

} // namespace kori
