/*
 * Copyright (C) 2021 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "ui_fsmenu/addons/remote_interaction.h"

#include <memory>

#include <boost/format.hpp>

#include "base/log.h"
#include "graphic/font_handler.h"
#include "graphic/image_cache.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "ui_basic/color_chooser.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/addons/manager.h"

namespace FsMenu {
namespace AddOnsUI {

std::map<std::pair<std::string, std::string>, std::string>
   RemoteInteractionWindow::downloaded_screenshots_cache_;

/* CommentRow implementation */

CommentRow::CommentRow(AddOnsCtrl& ctrl,
                       std::shared_ptr<AddOns::AddOnInfo> info,
                       RemoteInteractionWindow& r,
                       UI::Panel& parent,
                       const std::string& text,
                       const int64_t index)
   : UI::MultilineTextarea(&parent,
                           0,
                           0,
                           0,
                           0,
                           UI::PanelStyle::kFsMenu,
                           text,
                           UI::Align::kLeft,
                           UI::MultilineTextarea::ScrollMode::kNoScrolling),
     ctrl_(ctrl),
     info_(info),
     index_(index),
     edit_(this, "edit", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("Edit…")) {
	edit_.sigclicked.connect([this, &r]() {
		if (ctrl_.username().empty()) {
			return;
		}
		CommentEditor m(ctrl_, info_, index_);
		if (m.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
			r.update_data();
		}
	});
	layout();
	update_edit_enabled();
	initialization_complete();
}

void CommentRow::update_edit_enabled() {
	/* Admins can edit all posts; normal users only their own posts and only if the post was
	 * never edited by an admin yet. */
	edit_.set_visible(
	   !ctrl_.username().empty() &&
	   (ctrl_.net().is_admin() || (info_->user_comments[index_].username == ctrl_.username() &&
	                               (info_->user_comments[index_].editor.empty() ||
	                                info_->user_comments[index_].editor == ctrl_.username()))));
}

void CommentRow::layout() {
	UI::MultilineTextarea::layout();
	edit_.set_pos(Vector2i(get_w() - edit_.get_w(), 0));
}

/* CommentEditor implementation */

CommentEditor::CommentEditor(AddOnsCtrl& ctrl,
                             std::shared_ptr<AddOns::AddOnInfo> info,
                             const int64_t index)
   : UI::Window(&ctrl.get_topmost_forefather(),
                UI::WindowStyle::kFsMenu,
                "write_comment",
                0,
                0,
                100,
                100,
                index < 0 ? _("Write Comment") : _("Edit Comment")),
     info_(info),
     index_(index),
     main_box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     markup_box_(&main_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     buttons_box_(&main_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     preview_(&main_box_, 0, 0, 300, 150, UI::PanelStyle::kFsMenu, "", UI::Align::kLeft),
     text_(new UI::MultilineEditbox(&main_box_, 0, 0, 450, 200, UI::PanelStyle::kFsMenu)),
     ok_(&buttons_box_,
         "ok",
         0,
         0,
         kRowButtonSize,
         kRowButtonSize,
         UI::ButtonStyle::kFsMenuPrimary,
         _("OK")),
     reset_(&buttons_box_,
            "reset",
            0,
            0,
            kRowButtonSize,
            kRowButtonSize,
            UI::ButtonStyle::kFsMenuSecondary,
            _("Reset")),
     cancel_(&buttons_box_,
             "cancel",
             0,
             0,
             kRowButtonSize,
             kRowButtonSize,
             UI::ButtonStyle::kFsMenuSecondary,
             _("Cancel")) {
	if (ctrl.username().empty()) {
		die();
		return;
	}

	buttons_box_.add(&cancel_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kRowButtonSpacing);
	buttons_box_.add(&reset_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kRowButtonSpacing);
	buttons_box_.add(&ok_, UI::Box::Resizing::kExpandBoth);

	auto markup_button = [this](const std::string& name, const std::string& open,
	                            const std::string& close, const std::string& title,
	                            const std::string& tt) {
		UI::Button* b = new UI::Button(&markup_box_, name, 0, 0, kRowButtonSize, kRowButtonSize,
		                               UI::ButtonStyle::kFsMenuMenu, title, tt);
		b->sigclicked.connect([this, open, close]() { apply_format(open, close); });
		return b;
	};
	markup_box_.add(markup_button("markup_bold", "<font bold=true>", "</font>",
	                              /** TRANSLATORS: Short for Bold text markup */
	                              pgettext("markup", "B"), _("Bold")));
	markup_box_.add_space(kRowButtonSpacing);
	markup_box_.add(markup_button("markup_italic", "<font italic=true>", "</font>",
	                              /** TRANSLATORS: Short for Italic text markup */
	                              pgettext("markup", "I"), _("Italic")));
	markup_box_.add_space(kRowButtonSpacing);
	markup_box_.add(markup_button("markup_line", "<font underline=true>", "</font>",
	                              /** TRANSLATORS: Short for Underline text markup */
	                              pgettext("markup", "_"), _("Underline")));
	markup_box_.add_space(kRowButtonSpacing);
	markup_box_.add(markup_button("markup_shadow", "<font shadow=true>", "</font>",
	                              /** TRANSLATORS: Short for Shadow text markup */
	                              pgettext("markup", "S"), _("Shadow")));
	markup_box_.add_space(kRowButtonSpacing);
	{
		UI::Button* b = new UI::Button(&markup_box_, "markup_color", 0, 0, kRowButtonSize,
		                               kRowButtonSize, UI::ButtonStyle::kFsMenuMenu,
		                               /** TRANSLATORS: Short for Color text markup */
		                               pgettext("markup", "C"), _("Color…"));
		b->sigclicked.connect([this]() {
			UI::ColorChooser c(
			   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, RGBColor(0xffffff), nullptr);
			if (c.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
				std::string str = "<font color=";
				str += c.get_color().hex_value();
				str += ">";
				apply_format(str, "</font>");
			}
		});
		markup_box_.add(b);
	}
	markup_box_.add_space(kRowButtonSpacing);
	markup_box_.add(markup_button("markup_tiny", "<font size=8>", "</font>",
	                              /** TRANSLATORS: Short for Tiny text markup */
	                              pgettext("markup", "1"), _("Tiny")));
	markup_box_.add_space(kRowButtonSpacing);
	markup_box_.add(markup_button("markup_small", "<font size=11>", "</font>",
	                              /** TRANSLATORS: Short for Small text markup */
	                              pgettext("markup", "2"), _("Small")));
	markup_box_.add_space(kRowButtonSpacing);
	markup_box_.add(markup_button("markup_normal", "<font size=14>", "</font>",
	                              /** TRANSLATORS: Short for Medium text markup */
	                              pgettext("markup", "3"), _("Medium")));
	markup_box_.add_space(kRowButtonSpacing);
	markup_box_.add(markup_button("markup_large", "<font size=18>", "</font>",
	                              /** TRANSLATORS: Short for Large text markup */
	                              pgettext("markup", "4"), _("Large")));
	markup_box_.add_space(kRowButtonSpacing);
	markup_box_.add(markup_button("markup_huge", "<font size=24>", "</font>",
	                              /** TRANSLATORS: Short for Huge text markup */
	                              pgettext("markup", "5"), _("Huge")));
	markup_box_.add_space(kRowButtonSpacing);

	main_box_.add(&markup_box_, UI::Box::Resizing::kAlign, UI::Align::kRight);
	main_box_.add_space(kRowButtonSpacing);
	main_box_.add(text_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(kRowButtonSpacing);
	main_box_.add(&preview_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(kRowButtonSpacing);
	main_box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);

	cancel_.sigclicked.connect([this]() { die(); });
	reset_.sigclicked.connect([this]() { reset_text(); });
	ok_.sigclicked.connect([this, &ctrl]() {
		try {
			std::string message = text_->get_text();
			if (message.empty()) {
				return;
			}
			ctrl.net().comment(*info_, message, index_);
			*info_ = ctrl.net().fetch_one_remote(info_->internal_name);
			end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
		} catch (const std::exception& e) {
			log_err(
			   "Edit comment #%" PRId64 " for %s: %s", index_, info_->internal_name.c_str(), e.what());
			UI::WLMessageBox m(
			   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Error"),
			   (boost::format(_("The comment could not be submitted.\n\nError Message:\n%s")) %
			    e.what())
			      .str(),
			   UI::WLMessageBox::MBoxType::kOk);
			m.run<UI::Panel::Returncodes>();
		}
	});

	reset_text();
	set_center_panel(&main_box_);
	center_to_parent();
	text_->focus();

	initialization_complete();
}

void CommentEditor::think() {
	UI::Window::think();

	std::string p = "<rt><p>";
	p += g_style_manager->font_style(UI::FontStyle::kItalic).as_font_tag(_("Preview:"));
	p += "</p><p>";

	std::string message = text_->get_text();
	for (;;) {
		size_t pos = message.find('\n');
		if (pos == std::string::npos) {
			break;
		}
		message = message.replace(pos, 1, "<br>");
	}

	p += g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph).as_font_tag(message);
	p += "</p></rt>";

	ok_.set_enabled(!message.empty());
	try {
		// TODO(Nordfriese): The font renderer produces memory leaks if the text
		// is invalid (which is exactly what we're trying to check here). There
		// must be a safer (and faster) way to check whether the text is valid.
		UI::g_fh->render(p, preview_.get_inner_w());
	} catch (const std::exception& e) {
		ok_.set_enabled(false);
		p = "<rt><p>";
		p += g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
		        .as_font_tag(_("The comment contains invalid richtext markup:"));
		p += "</p><p>";
		p += g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
		        .as_font_tag(richtext_escape(e.what()));
		p += "</p></rt>";
	}

	if (preview_.get_text() != p) {
		preview_.set_text(p);
	}
}

void CommentEditor::apply_format(const std::string& open_tag, const std::string& close_tag) {
	const size_t caret = text_->get_caret_pos();
	if (text_->has_selection()) {
		std::string str = open_tag;
		str += text_->get_selected_text();
		str += close_tag;
		text_->replace_selected_text(str);
	} else {
		std::string str = text_->get_text().substr(0, caret);
		str += open_tag;
		str += close_tag;
		str += text_->get_text().substr(caret);
		text_->set_text(str);
		text_->set_caret_pos(caret + open_tag.size());
	}
	text_->focus();
}

void CommentEditor::reset_text() {
	text_->set_text(index_ < 0 ? "" : info_->user_comments[index_].message);
	think();
}

/* RemoteInteractionWindow implementation */

RemoteInteractionWindow::RemoteInteractionWindow(AddOnsCtrl& parent,
                                                 std::shared_ptr<AddOns::AddOnInfo> info)
   : UI::Window(parent.get_parent(),
                UI::WindowStyle::kFsMenu,
                info->internal_name,
                parent.get_x() + kRowButtonSize,
                parent.get_y() + kRowButtonSize,
                parent.get_inner_w() - 2 * kRowButtonSize,
                parent.get_inner_h() - 2 * kRowButtonSize,
                info->descname()),
     parent_(parent),
     info_(info),
     current_screenshot_(0),
     nr_screenshots_(info->screenshots.size()),
     current_vote_(-1),

     main_box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     tabs_(&main_box_, UI::TabPanelStyle::kFsMenu),
     box_comments_(&tabs_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     box_comment_rows_(&box_comments_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     box_screenies_(&tabs_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     box_screenies_buttons_(&box_screenies_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     box_votes_(&tabs_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     voting_stats_(&box_votes_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     box_comment_rows_placeholder_(&box_comments_, UI::PanelStyle::kFsMenu, 0, 0, 0, 0),
     comments_header_(&box_comments_,
                      0,
                      0,
                      0,
                      0,
                      UI::PanelStyle::kFsMenu,
                      "",
                      UI::Align::kLeft,
                      UI::MultilineTextarea::ScrollMode::kNoScrolling),
     screenshot_(&box_screenies_, UI::PanelStyle::kFsMenu, 0, 0, 0, 0, nullptr),
     own_voting_(&box_votes_,
                 "voting",
                 0,
                 0,
                 0,
                 11,
                 kRowButtonSize - kRowButtonSpacing,
                 _("Your vote"),
                 UI::DropdownType::kTextual,
                 UI::PanelStyle::kFsMenu,
                 UI::ButtonStyle::kFsMenuSecondary),
     screenshot_stats_(&box_screenies_buttons_,
                       UI::PanelStyle::kFsMenu,
                       UI::FontStyle::kFsMenuLabel,
                       "",
                       UI::Align::kCenter),
     screenshot_descr_(&box_screenies_,
                       UI::PanelStyle::kFsMenu,
                       UI::FontStyle::kFsMenuLabel,
                       "",
                       UI::Align::kCenter),
     voting_stats_summary_(
        &box_votes_, UI::PanelStyle::kFsMenu, UI::FontStyle::kFsMenuLabel, "", UI::Align::kCenter),
     screenshot_next_(&box_screenies_buttons_,
                      "next_screenshot",
                      0,
                      0,
                      48,
                      24,
                      UI::ButtonStyle::kFsMenuSecondary,
                      g_image_cache->get("images/ui_basic/scrollbar_right.png"),
                      _("Next screenshot")),
     screenshot_prev_(&box_screenies_buttons_,
                      "prev_screenshot",
                      0,
                      0,
                      48,
                      24,
                      UI::ButtonStyle::kFsMenuSecondary,
                      g_image_cache->get("images/ui_basic/scrollbar_left.png"),
                      _("Previous screenshot")),
     write_comment_(&box_comments_,
                    "write_comment",
                    0,
                    0,
                    0,
                    0,
                    UI::ButtonStyle::kFsMenuSecondary,
                    _("Write a comment…")),
     ok_(&main_box_, "ok", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("OK")),
     login_button_(this, "login", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, "") {

	ok_.sigclicked.connect([this]() { end_modal(UI::Panel::Returncodes::kBack); });

	own_voting_.add(_("Not voted"), 0, nullptr, true);
	for (unsigned i = 1; i <= 10; ++i) {
		own_voting_.add(std::to_string(i), i);
	}
	own_voting_.selected.connect([this]() {
		current_vote_ = own_voting_.get_selected();
		try {
			parent_.net().vote(info_->internal_name, current_vote_);
			*info_ = parent_.net().fetch_one_remote(info_->internal_name);
		} catch (const std::exception& e) {
			UI::WLMessageBox w(
			   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Error"),
			   (boost::format(_("The vote could not be submitted.\nError code: %s")) % e.what()).str(),
			   UI::WLMessageBox::MBoxType::kOk);
			w.run<UI::Panel::Returncodes>();
			return;
		}
		update_data();
		parent_.rebuild();
	});
	write_comment_.sigclicked.connect([this]() {
		CommentEditor m(parent_, info_, -1);
		if (m.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
			update_data();
			parent_.rebuild();
		}
	});

	box_screenies_buttons_.add(&screenshot_prev_, UI::Box::Resizing::kFullSize);
	box_screenies_buttons_.add(&screenshot_stats_, UI::Box::Resizing::kExpandBoth);
	box_screenies_buttons_.add(&screenshot_next_, UI::Box::Resizing::kFullSize);

	box_screenies_.add_space(kRowButtonSpacing);
	box_screenies_.add(&box_screenies_buttons_, UI::Box::Resizing::kFullSize);
	box_screenies_.add_space(kRowButtonSpacing);
	box_screenies_.add(&screenshot_, UI::Box::Resizing::kExpandBoth);
	box_screenies_.add_space(kRowButtonSpacing);
	box_screenies_.add(&screenshot_descr_, UI::Box::Resizing::kFullSize);

	box_comment_rows_.set_force_scrolling(true);
	box_comments_.add(&comments_header_, UI::Box::Resizing::kFullSize);
	box_comments_.add_space(kRowButtonSpacing);
	box_comments_.add(&box_comment_rows_placeholder_, UI::Box::Resizing::kExpandBoth);
	box_comments_.add_space(kRowButtonSpacing);
	box_comments_.add(&write_comment_, UI::Box::Resizing::kFullSize);

	voting_stats_.add_inf_space();
	for (unsigned i = 0; i < AddOns::kMaxRating; ++i) {
		UI::Box* box = new UI::Box(&voting_stats_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical);
		voting_bars_[i] = new UI::ProgressBar(
		   box, UI::PanelStyle::kFsMenu, 0, 0, kRowButtonSize * 3 / 2, 0, UI::ProgressBar::Vertical);
		voting_bars_[i]->set_show_percent(false);
		voting_txt_[i] = new UI::Textarea(
		   box, UI::PanelStyle::kFsMenu, UI::FontStyle::kFsMenuLabel, "", UI::Align::kCenter);

		box->add(voting_bars_[i], UI::Box::Resizing::kFillSpace, UI::Align::kCenter);
		box->add_space(kRowButtonSpacing);
		box->add(voting_txt_[i], UI::Box::Resizing::kAlign, UI::Align::kCenter);
		voting_stats_.add(box, UI::Box::Resizing::kExpandBoth);
		voting_stats_.add_inf_space();
	}

	box_votes_.add(&voting_stats_summary_, UI::Box::Resizing::kFullSize);
	box_votes_.add_space(kRowButtonSpacing);
	box_votes_.add(&voting_stats_, UI::Box::Resizing::kExpandBoth);
	box_votes_.add_space(kRowButtonSpacing);
	box_votes_.add(&own_voting_, UI::Box::Resizing::kFullSize);
	box_votes_.add_space(kRowButtonSpacing);

	tabs_.add("comments", "", &box_comments_);
	if (nr_screenshots_) {
		tabs_.add("screenshots",
		          (boost::format(_("Screenshots (%u)")) % info_->screenshots.size()).str(),
		          &box_screenies_);
		tabs_.sigclicked.connect([this]() {
			if (tabs_.active() == 1) {
				next_screenshot(0);
			}
		});
	} else {
		box_screenies_.set_visible(false);
	}
	tabs_.add("votes", "", &box_votes_);

	main_box_.add(&tabs_, UI::Box::Resizing::kExpandBoth);
	main_box_.add_space(kRowButtonSpacing);
	main_box_.add(&ok_, UI::Box::Resizing::kFullSize);

	screenshot_next_.set_enabled(nr_screenshots_ > 1);
	screenshot_prev_.set_enabled(nr_screenshots_ > 1);
	screenshot_cache_.resize(nr_screenshots_, nullptr);
	screenshot_next_.sigclicked.connect([this]() { next_screenshot(1); });
	screenshot_prev_.sigclicked.connect([this]() { next_screenshot(-1); });

	login_button_.sigclicked.connect([this]() {
		parent_.login_button_clicked();
		parent_.update_login_button(&login_button_);
		login_changed();
	});
	parent_.update_login_button(&login_button_);
	login_changed();

	update_data();
	layout();

	initialization_complete();
}

void RemoteInteractionWindow::on_resolution_changed_note(const GraphicResolutionChanged& note) {
	UI::Window::on_resolution_changed_note(note);

	set_size(parent_.get_inner_w() - 2 * kRowButtonSize, parent_.get_inner_h() - 2 * kRowButtonSize);
	set_pos(Vector2i(parent_.get_x() + kRowButtonSize, parent_.get_y() + kRowButtonSize));
	main_box_.set_size(get_inner_w(), get_inner_h());
}

void RemoteInteractionWindow::layout() {
	if (!is_minimal()) {
		main_box_.set_size(get_inner_w(), get_inner_h());
		login_button_.set_size(get_inner_w() / 3, login_button_.get_h());
		login_button_.set_pos(Vector2i(get_inner_w() - login_button_.get_w(), 0));
		box_comment_rows_.set_pos(box_comment_rows_placeholder_.get_pos());
		box_comment_rows_.set_size(
		   box_comment_rows_placeholder_.get_w(), box_comment_rows_placeholder_.get_h());
	}
	UI::Window::layout();
}

void RemoteInteractionWindow::update_data() {
	(*tabs_.tabs().begin())
	   ->set_title((boost::format(_("Comments (%u)")) % info_->user_comments.size()).str());
	(*tabs_.tabs().rbegin())
	   ->set_title((boost::format(_("Votes (%u)")) % info_->number_of_votes()).str());

	voting_stats_summary_.set_text(
	   info_->number_of_votes() ?
         (boost::format(ngettext("Average rating: %1$.3f (%2$u vote)",
	                              "Average rating: %1$.3f (%2$u votes)", info_->number_of_votes())) %
	       info_->average_rating() % info_->number_of_votes())
	         .str() :
         _("No votes yet"));

	uint32_t most_votes = 1;
	for (uint32_t v : info_->votes) {
		most_votes = std::max(most_votes, v);
	}
	for (unsigned i = 0; i < AddOns::kMaxRating; ++i) {
		voting_bars_[i]->set_total(most_votes);
		voting_bars_[i]->set_state(info_->votes[i]);
		voting_txt_[i]->set_text(std::to_string(i + 1));
	}

	box_comment_rows_.clear();
	comment_rows_.clear();
	std::string text = "<rt><p>";
	text += g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
	           .as_font_tag(info_->user_comments.empty() ?
                              _("No comments yet.") :
                              (boost::format(ngettext(
	                               "%u comment:", "%u comments:", info_->user_comments.size())) %
	                            info_->user_comments.size())
	                              .str());
	text += "</p></rt>";
	comments_header_.set_text(text);
	int64_t index = 0;
	for (const auto& comment : info_->user_comments) {
		text = "<rt><p>";
		if (comment.editor.empty()) {
			text += g_style_manager->font_style(UI::FontStyle::kItalic)
			           .as_font_tag(time_string(comment.timestamp));
		} else if (comment.editor == comment.username) {
			text +=
			   g_style_manager->font_style(UI::FontStyle::kItalic)
			      .as_font_tag((boost::format(_("%1$s (edited on %2$s)")) %
			                    time_string(comment.timestamp) % time_string(comment.edit_timestamp))
			                      .str());
		} else {
			text += g_style_manager->font_style(UI::FontStyle::kItalic)
			           .as_font_tag((boost::format(_("%1$s (edited by ‘%2$s’ on %3$s)")) %
			                         time_string(comment.timestamp) % comment.editor %
			                         time_string(comment.edit_timestamp))
			                           .str());
		}
		text += "<br>";
		text += g_style_manager->font_style(UI::FontStyle::kItalic)
		           .as_font_tag((boost::format(_("‘%1$s’ commented on version %2$s:")) %
		                         comment.username % AddOns::version_to_string(comment.version))
		                           .str());
		text += "<br>";
		text += g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
		           .as_font_tag(comment.message);
		text += "</p></rt>";

		CommentRow* cr = new CommentRow(parent_, info_, *this, box_comment_rows_, text, index);
		comment_rows_.push_back(std::unique_ptr<CommentRow>(cr));
		box_comment_rows_.add_space(kRowButtonSize);
		box_comment_rows_.add(cr, UI::Box::Resizing::kFullSize);
		++index;
	}
}

void RemoteInteractionWindow::next_screenshot(int8_t delta) {
	assert(nr_screenshots_ > 0);
	while (delta < 0) {
		delta += nr_screenshots_;
	}
	current_screenshot_ = (current_screenshot_ + delta) % nr_screenshots_;
	assert(current_screenshot_ < static_cast<int32_t>(screenshot_cache_.size()));

	auto it = info_->screenshots.begin();
	std::advance(it, current_screenshot_);

	screenshot_stats_.set_text(
	   (boost::format(_("%1$u / %2$u")) % (current_screenshot_ + 1) % nr_screenshots_).str());
	screenshot_descr_.set_text(it->second);
	screenshot_.set_tooltip("");

	if (screenshot_cache_[current_screenshot_]) {
		screenshot_.set_icon(screenshot_cache_[current_screenshot_]);
		return;
	}

	const Image* image = nullptr;
	const std::pair<std::string, std::string> cache_key(info_->internal_name, it->first);
	auto cached = downloaded_screenshots_cache_.find(cache_key);
	if (cached == downloaded_screenshots_cache_.end()) {
		const std::string screenie =
		   parent_.net().download_screenshot(cache_key.first, cache_key.second);
		try {
			if (!screenie.empty()) {
				image = g_image_cache->get(screenie);
			}
			downloaded_screenshots_cache_[cache_key] = screenie;
		} catch (const std::exception& e) {
			log_err("Error downloading screenshot %s for %s: %s", it->first.c_str(),
			        info_->internal_name.c_str(), e.what());
			image = nullptr;
		}
	} else if (!cached->second.empty()) {
		image = g_image_cache->get(cached->second);
	}

	if (image) {
		screenshot_.set_icon(image);
		screenshot_cache_[current_screenshot_] = image;
	} else {
		screenshot_.set_icon(g_image_cache->get("images/ui_basic/stop.png"));
		screenshot_.set_handle_mouse(true);
		screenshot_.set_tooltip(
		   _("This screenshot could not be fetched from the server due to an error."));
	}
}

void RemoteInteractionWindow::login_changed() {
	current_vote_ = parent_.net().get_vote(info_->internal_name);
	if (current_vote_ < 0) {
		write_comment_.set_enabled(false);
		write_comment_.set_tooltip(_("Please log in to comment"));
		own_voting_.set_enabled(false);
		own_voting_.set_tooltip(_("Please log in to vote"));
		own_voting_.select(0);
	} else {
		write_comment_.set_enabled(true);
		write_comment_.set_tooltip("");
		own_voting_.set_enabled(true);
		own_voting_.set_tooltip("");
		own_voting_.select(current_vote_);
	}
	for (auto& cr : comment_rows_) {
		cr->update_edit_enabled();
	}
}

}  // namespace AddOnsUI
}  // namespace FsMenu
