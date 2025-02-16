/*
 * Copyright (C) 2003-2025 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wui/waresdisplay.h"

#include <memory>

#include <SDL_mouse.h>
#include <SDL_timer.h>

#include "base/i18n.h"
#include "base/wexception.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "logic/editor_game_base.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"
#include "ui_basic/button.h"
#include "ui_basic/window.h"

constexpr int kWareMenuInfoSize = 12;

AbstractWaresDisplay::AbstractWaresDisplay(
   UI::Panel* const parent,
   int32_t x,
   int32_t y,
   const Widelands::TribeDescr& tribe,
   Widelands::WareWorker type,
   bool selectable,
   std::function<void(Widelands::DescriptionIndex, bool)> callback_function,
   bool horizontal,
   int32_t hgap,
   int32_t vgap)
   :  // Size is set when layout is called
     UI::Panel(parent, UI::PanelStyle::kWui, "wares_display", x, y, 0, 0),
     tribe_(tribe),

     type_(type),
     indices_(type_ == Widelands::wwWORKER ? tribe_.workers() : tribe_.wares()),
     curware_(this,
              UI::PanelStyle::kWui,
              "current",
              UI::FontStyle::kWuiLabel,
              0,
              get_inner_h() - 25,
              get_inner_w(),
              20,
              "",
              UI::Align::kCenter),

     selectable_(selectable),
     horizontal_(horizontal),
     hgap_(hgap),
     vgap_(vgap),
     selection_anchor_(Widelands::INVALID_INDEX),
     callback_function_(std::move(callback_function)) {
	for (const Widelands::DescriptionIndex& index : indices_) {
		selected_.insert(std::make_pair(index, false));
		hidden_.insert(std::make_pair(index, false));
		in_selection_.insert(std::make_pair(index, false));
		ware_details_cache_.insert(
		   std::make_pair(index, std::make_pair(RGBAColor(0, 0, 0, 0), nullptr)));
	}

	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged& /* note */) { recalc_desired_size(true); });

	recalc_desired_size(false);
}

AbstractWaresDisplay::~AbstractWaresDisplay() {
	NoteThreadSafeFunction::instantiate([this]() { ware_details_cache_.clear(); }, true);
}

Widelands::Extent AbstractWaresDisplay::get_extent() const {
	int16_t columns = 0;
	int16_t rows = 0;
	for (const auto& pair : icons_order_coords()) {
		columns = std::max(columns, pair.second.x);
		rows = std::max(rows, pair.second.y);
	}
	// We cound from 0 up
	++columns;
	++rows;

	if (horizontal_) {
		const int16_t s = columns;
		columns = rows;
		rows = s;
	}
	return Widelands::Extent(columns, rows);
}

void AbstractWaresDisplay::set_hgap(int32_t gap, bool relayout) {
	hgap_ = gap;
	recalc_desired_size(relayout);
}

void AbstractWaresDisplay::set_vgap(int32_t gap, bool relayout) {
	vgap_ = gap;
	recalc_desired_size(relayout);
}

void AbstractWaresDisplay::recalc_desired_size(bool relayout) {
	relayout_icons_order_coords();

	// Find out geometry from icons_order
	const Widelands::Extent size = get_extent();

	// 25 is height of curware_ text
	set_desired_size(size.w * (kWareMenuPicWidth + hgap_) - hgap_ + 5,
	                 size.h * (kWareMenuPicHeight + kWareMenuInfoSize + vgap_) - vgap_ + 1 + 25);

	if (relayout) {
		// Since we are usually stacked deep within other panels, we need to tell our highest parent
		// window to relayout
		// TODO(GunChleoc): Window::on_resolution_changed_note can't shift these properly due to the
		// changing dimensions.
		UI::Panel* p = this;
		while (p->get_parent() != nullptr) {
			p = p->get_parent();
			if (dynamic_cast<UI::Window*>(p) != nullptr) {
				p->layout();
				return;
			}
		}
	}
}

bool AbstractWaresDisplay::handle_mousemove(
   uint8_t state, int32_t x, int32_t y, int32_t /*xdiff*/, int32_t /*ydiff*/) {
	const Widelands::DescriptionIndex index = ware_at_point(x, y);

	curware_.set_fixed_width(get_inner_w());

	curware_.set_text(index != Widelands::INVALID_INDEX ?
	                     (type_ == Widelands::wwWORKER ? tribe_.get_worker_descr(index)->descname() :
	                                                     tribe_.get_ware_descr(index)->descname()) :
	                     "");
	if (selection_anchor_ != Widelands::INVALID_INDEX) {
		// Ensure mouse button is still pressed as some
		// mouse release events do not reach us
		if ((state ^ SDL_BUTTON_LMASK) != 0) {
			// TODO(unknown): We should call another function that will not pass that events
			// to our Panel superclass
			handle_mouserelease(SDL_BUTTON_LEFT, x, y);
			return true;
		}
		update_anchor_selection(x, y);
	}
	return true;
}

bool AbstractWaresDisplay::handle_mousepress(uint8_t btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_LEFT) {
		Widelands::DescriptionIndex ware = ware_at_point(x, y);

		if (!tribe_.has_ware(ware) && !tribe_.has_worker(ware)) {
			return false;
		}
		if (!selectable_) {
			return true;
		}
		if (selection_anchor_ == Widelands::INVALID_INDEX) {
			// Create the selection anchor to be able to select
			// multiple ware by dragging.
			selection_anchor_ = ware;
			in_selection_[ware] = true;
			need_texture_update_ = true;
		} else {
			// A mouse release has been missed
		}
		return true;
	}

	return UI::Panel::handle_mousepress(btn, x, y);
}

bool AbstractWaresDisplay::handle_mouserelease(uint8_t btn, int32_t x, int32_t y) {
	if (btn != SDL_BUTTON_LEFT || selection_anchor_ == Widelands::INVALID_INDEX) {
		return UI::Panel::handle_mouserelease(btn, x, y);
	}
	finalize_anchor_selection();
	return true;
}

/**
 * Returns the index of the ware under the given coordinates, or
 * DescriptionIndex::null() if the given point is outside the range.
 */
Widelands::DescriptionIndex AbstractWaresDisplay::ware_at_point(int32_t x, int32_t y) const {
	// Graphical offset
	x -= 2;
	y -= 2;

	if (x < 0 || y < 0) {
		return Widelands::INVALID_INDEX;
	}

	int i = x / (kWareMenuPicWidth + hgap_);
	int j = y / (kWareMenuPicHeight + kWareMenuInfoSize + vgap_);
	if (kWareMenuPicWidth * (i + 1) + hgap_ * i < x ||
	    (kWareMenuPicHeight + kWareMenuInfoSize) * (j + 1) + vgap_ * j < y) {
		// Not on the ware, but on the space between
		return Widelands::INVALID_INDEX;
	}
	if (horizontal_) {
		int s = i;
		i = j;
		j = s;
	}
	for (const auto& pair : icons_order_coords()) {
		if (pair.second.x == i && pair.second.y == j) {
			assert(hidden_.count(pair.first) == 1);
			if (!(hidden_.find(pair.first)->second)) {
				return pair.first;
			}
			break;
		}
	}

	return Widelands::INVALID_INDEX;
}

Widelands::DescriptionIndex AbstractWaresDisplay::ware_at_coords(int16_t x, int16_t y) const {
	for (const auto& pair : icons_order_coords()) {
		if (pair.second.x == x && pair.second.y == y) {
			return pair.first;
		}
	}
	return Widelands::INVALID_INDEX;
}

uint16_t AbstractWaresDisplay::column_length(int16_t x) const {
	uint16_t l = 0;
	for (const auto& pair : icons_order_coords()) {
		if (pair.second.x == x) {
			l = std::max(l, static_cast<uint16_t>(pair.second.y + 1));
		}
	}
	return l;
}

// Update the anchored selection. An anchor has been created by mouse
// press. Mouse move call this function with the current mouse position.
// This function will temporary store all wares in the rectangle between anchor
// and current pos to allow their selection on mouse release
void AbstractWaresDisplay::update_anchor_selection(int32_t x, int32_t y) {
	if (selection_anchor_ == Widelands::INVALID_INDEX || x < 0 || y < 0) {
		return;
	}

	need_texture_update_ = true;

	for (auto& resetme : in_selection_) {
		in_selection_[resetme.first] = false;
	}

	Vector2i anchor_pos = ware_position(selection_anchor_);
	// Add an offset to make sure the anchor line and column will be
	// selected when selecting in topleft direction
	int32_t anchor_x = anchor_pos.x + kWareMenuPicWidth / 2;
	int32_t anchor_y = anchor_pos.y + kWareMenuPicHeight / 2;

	unsigned int left_ware_idx = anchor_x / (kWareMenuPicWidth + hgap_);
	unsigned int top_ware_idx = anchor_y / (kWareMenuPicHeight + kWareMenuInfoSize + vgap_);
	unsigned int right_ware_idx = x / (kWareMenuPicWidth + hgap_);
	unsigned int bottoware_idx_ = y / (kWareMenuPicHeight + kWareMenuInfoSize + vgap_);
	unsigned int tmp;

	// Reverse col/row and anchor/endpoint if needed
	if (horizontal_) {
		tmp = left_ware_idx;
		left_ware_idx = top_ware_idx;
		top_ware_idx = tmp;
		tmp = right_ware_idx;
		right_ware_idx = bottoware_idx_;
		bottoware_idx_ = tmp;
	}
	if (left_ware_idx > right_ware_idx) {
		tmp = left_ware_idx;
		left_ware_idx = right_ware_idx;
		right_ware_idx = tmp;
	}
	if (top_ware_idx > bottoware_idx_) {
		tmp = top_ware_idx;
		top_ware_idx = bottoware_idx_;
		bottoware_idx_ = tmp;
	}

	for (unsigned int cur_ware_x = left_ware_idx; cur_ware_x <= right_ware_idx; cur_ware_x++) {
		if (cur_ware_x < icons_order_coords().size()) {
			for (unsigned cur_ware_y = top_ware_idx; cur_ware_y <= bottoware_idx_; cur_ware_y++) {
				if (cur_ware_y < static_cast<unsigned>(column_length(cur_ware_x))) {
					Widelands::DescriptionIndex ware = ware_at_coords(cur_ware_x, cur_ware_y);
					if (!hidden_[ware]) {
						in_selection_[ware] = true;
					}
				}
			}
		}
	}
}

void AbstractWaresDisplay::finalize_anchor_selection() {
	if (selection_anchor_ == Widelands::INVALID_INDEX) {
		return;
	}

	bool to_be_selected = !ware_selected(selection_anchor_);

	for (const Widelands::DescriptionIndex& index : indices_) {
		if (in_selection_[index]) {
			if (to_be_selected) {
				select_ware(index);
			} else {
				unselect_ware(index);
			}
		}
	}

	// Release anchor, empty selection
	selection_anchor_ = Widelands::INVALID_INDEX;
	for (auto& resetme : in_selection_) {
		in_selection_[resetme.first] = false;
	}

	need_texture_update_ = true;
}

void AbstractWaresDisplay::layout() {
	curware_.set_pos(Vector2i(0, get_inner_h() - 22));
	curware_.set_size(get_inner_w(), 20);
}

void AbstractWaresDisplay::draw(RenderTarget& dst) {
	draw_ware_backgrounds(dst);

	for (const Widelands::DescriptionIndex& index : indices_) {
		if (!hidden_[index]) {
			draw_ware(dst, index);
		}
	}
}

const Widelands::TribeDescr::WaresOrder& AbstractWaresDisplay::icons_order() const {
	switch (type_) {
	case Widelands::wwWARE:
		return tribe_.wares_order();
	case Widelands::wwWORKER:
		return tribe_.workers_order();
	default:
		NEVER_HERE();
	}
}

const WaresOrderCoords& AbstractWaresDisplay::icons_order_coords() const {
	assert(!order_coords_.empty());
	return order_coords_;
}

void AbstractWaresDisplay::relayout_icons_order_coords() {
	order_coords_.clear();
	const int column_number = icons_order().size();
	const int column_max_size = std::max(1, (g_gr->get_yres() - min_free_vertical_space_) /
	                                           (kWareMenuPicHeight + vgap_ + kWareMenuInfoSize));

	int16_t column_index_to_apply = 0;
	for (int16_t column_index = 0; column_index < column_number; ++column_index) {
		const std::vector<Widelands::DescriptionIndex>& column = icons_order().at(column_index);
		const int row_number = column.size();
		int16_t row_index_to_apply = 0;
		for (int16_t row_index = 0; row_index < row_number; ++row_index) {
			order_coords_.emplace(
			   column.at(row_index), Widelands::Coords(column_index_to_apply, row_index_to_apply));
			++row_index_to_apply;
			if (row_index_to_apply >= column_max_size) {
				row_index_to_apply = 0;
				++column_index_to_apply;
			}
		}
		if (row_index_to_apply > 0) {
			++column_index_to_apply;
		}
	}
}

Vector2i AbstractWaresDisplay::ware_position(Widelands::DescriptionIndex id) const {
	Vector2i p(2, 2);
	if (horizontal_) {
		p.x += icons_order_coords().at(id).y * (kWareMenuPicWidth + hgap_);
		p.y += icons_order_coords().at(id).x * (kWareMenuPicHeight + vgap_ + kWareMenuInfoSize);
	} else {
		p.x += icons_order_coords().at(id).x * (kWareMenuPicWidth + hgap_);
		p.y += icons_order_coords().at(id).y * (kWareMenuPicHeight + vgap_ + kWareMenuInfoSize);
	}
	return p;
}

bool AbstractWaresDisplay::draw_ware_as_selected(Widelands::DescriptionIndex id) const {
	bool draw_selected = selected_.at(id);
	if (selection_anchor_ != Widelands::INVALID_INDEX) {
		// Draw the temporary selected wares as if they were selected.
		// TODO(unknown): Use another pic for the temporary selection
		if (!ware_selected(selection_anchor_)) {
			draw_selected |= in_selection_.at(id);
		} else {
			draw_selected &= !in_selection_.at(id);
		}
	}
	return draw_selected;
}

void AbstractWaresDisplay::draw_ware_backgrounds(RenderTarget& dst) {
	const uint32_t time = SDL_GetTicks();
	const bool update = time - last_ware_details_cache_update_ > 250;

	if (update) {
		last_ware_details_cache_update_ = time;

		for (auto& pair : ware_details_cache_) {
			if (!hidden_[pair.first]) {
				RGBAColor rgba = draw_ware_background_overlay(pair.first);
				if (pair.second.first != rgba) {
					need_texture_update_ = true;
					pair.second.first = rgba;
				}
			}
		}
	}

	/* Blitting many small icons is super expensive.
	 * Instead we blit them all once to a cached texture that is regenerated only when needed.
	 * Similary, rendering the info texts is also performance-hungry and not done in every frame.
	 */
	if (need_texture_update_ || background_texture_ == nullptr) {
		need_texture_update_ = false;
		const int imgw = get_inner_w();
		const int imgh = get_inner_h();
		background_texture_.reset(new Texture(imgw, imgh));
		background_texture_->fill_rect(
		   Rectf(0, 0, imgw, imgh), RGBAColor(0, 0, 0, 0), BlendMode::Copy);

		for (const Widelands::DescriptionIndex& id : indices_) {
			if (!hidden_[id]) {
				const bool draw_selected = draw_ware_as_selected(id);

				const UI::WareInfoStyleInfo& style =
				   draw_selected ? g_style_manager->ware_info_style(UI::WareInfoStyle::kHighlight) :
				                   g_style_manager->ware_info_style(UI::WareInfoStyle::kNormal);

				if (update) {
					ware_details_cache_[id].second =
					   UI::g_fh->render(as_richtext_paragraph(info_for_ware(id), style.info_font()));
				}

				const Image& bg = *style.icon_background_image();
				uint16_t w = bg.width();
				uint16_t h = bg.height();

				const Vector2i p = ware_position(id);
				background_texture_->blit(
				   Rectf(p, w, h), bg, Rectf(0, 0, w, h), 1.f, BlendMode::Default);

				if (const RGBAColor& overlay = ware_details_cache_[id].first; overlay.a != 0) {
					background_texture_->fill_rect(Rectf(p, w, h), overlay, BlendMode::Default);
				}

				const Image* icon = type_ == Widelands::wwWARE ? tribe_.get_ware_descr(id)->icon() :
				                                                 tribe_.get_worker_descr(id)->icon();
				background_texture_->blit(
				   Rectf(p.x + (w - kWareMenuPicWidth) / 2.f, p.y + 1, icon->width(), icon->height()),
				   *icon, Rectf(0, 0, icon->width(), icon->height()), 1.f, BlendMode::Default);
			}
		}
	} else if (update) {
		for (const Widelands::DescriptionIndex& id : indices_) {
			if (!hidden_[id]) {
				const UI::WareInfoStyleInfo& style =
				   draw_ware_as_selected(id) ?
				      g_style_manager->ware_info_style(UI::WareInfoStyle::kHighlight) :
				      g_style_manager->ware_info_style(UI::WareInfoStyle::kNormal);
				ware_details_cache_[id].second =
				   UI::g_fh->render(as_richtext_paragraph(info_for_ware(id), style.info_font()));
			}
		}
	}

	dst.blit(Vector2i::zero(), background_texture_.get(), BlendMode::Default);
}

/*
===============
WaresDisplay::draw_ware [virtual]

Draw one ware icon + additional information.
===============
*/
void AbstractWaresDisplay::draw_ware(RenderTarget& dst, Widelands::DescriptionIndex id) {
	const bool draw_selected = draw_ware_as_selected(id);

	const UI::WareInfoStyleInfo& style =
	   draw_selected ? g_style_manager->ware_info_style(UI::WareInfoStyle::kHighlight) :
	                   g_style_manager->ware_info_style(UI::WareInfoStyle::kNormal);

	uint16_t w = style.icon_background_image()->width();

	const Vector2i p = ware_position(id);

	dst.fill_rect(
	   Recti(p + Vector2i(0, kWareMenuPicHeight), w, kWareMenuInfoSize), info_color_for_ware(id));

	const auto& rendered_text = *ware_details_cache_[id].second;
	rendered_text.draw(
	   dst, Vector2i(p.x + w - rendered_text.width() - 1,
	                 p.y + kWareMenuPicHeight + kWareMenuInfoSize + 1 - rendered_text.height()));
}

// Wares highlighting/selecting
void AbstractWaresDisplay::select_ware(Widelands::DescriptionIndex ware) {
	if (selected_[ware]) {
		return;
	}

	selected_[ware] = true;
	need_texture_update_ = true;
	if (callback_function_) {
		callback_function_(ware, true);
	}
}

void AbstractWaresDisplay::unselect_ware(Widelands::DescriptionIndex ware) {
	if (!selected_[ware]) {
		return;
	}

	selected_[ware] = false;
	need_texture_update_ = true;
	if (callback_function_) {
		callback_function_(ware, false);
	}
}

bool AbstractWaresDisplay::ware_selected(Widelands::DescriptionIndex ware) const {
	return selected_.at(ware);
}

// Wares hiding
void AbstractWaresDisplay::set_hidden(Widelands::DescriptionIndex ware, bool hide) {
	if (hidden_[ware] != hide) {
		hidden_[ware] = hide;
		if (hide) {
			selected_[ware] = false;
		} else {
			last_ware_details_cache_update_ = 0;
		}
		need_texture_update_ = true;
	}
}

bool AbstractWaresDisplay::is_ware_hidden(Widelands::DescriptionIndex ware) const {
	return hidden_.at(ware);
}

WaresDisplay::WaresDisplay(UI::Panel* const parent,
                           int32_t x,
                           int32_t y,
                           const Widelands::TribeDescr& tribe,
                           Widelands::WareWorker type,
                           bool selectable)
   : AbstractWaresDisplay(parent, x, y, tribe, type, selectable) {
}

StockMenuWaresDisplay::StockMenuWaresDisplay(UI::Panel* const parent,
                                             const int32_t x,
                                             const int32_t y,
                                             const Widelands::Player& p,
                                             const Widelands::WareWorker type,
                                             bool total)
   : WaresDisplay(parent, x, y, p.tribe(), type, false), player_(p), show_total_stock_(total) {
}

std::string StockMenuWaresDisplay::info_for_ware(const Widelands::DescriptionIndex di) {
	std::string text = WaresDisplay::info_for_ware(di);
	if (solid_icon_backgrounds_) {
		return text;
	}

	const uint32_t current_amount = amount_of(di);
	if (current_amount >= 1000) {
		// Not enough space to show an indicator.
		// This only happens with very large amounts where the trend is not super-important anyway…
		return text;
	}

	// Indicate trend over the last 5 minutes
	const std::vector<uint32_t>& history = get_type() == Widelands::wwWARE ?
	                                          *player_.get_ware_stock_statistics(di) :
	                                          *player_.get_worker_stock_statistics(di);
	const size_t nr_entries = history.size();

	if (nr_entries == 0u) {
		// No records yet
		return text;
	}

	const size_t kSampleEntriesForTrend = 5 * 60 * 1000 / Widelands::kStatisticsSampleTime.get();
	const uint32_t last_amount =
	   history[nr_entries < kSampleEntriesForTrend ? nr_entries - 1 :
	                                                 nr_entries - kSampleEntriesForTrend];

	const UI::BuildingStatisticsStyleInfo& colors = g_style_manager->building_statistics_style();
	const std::string indicator =
	   current_amount < last_amount ?
	      StyleManager::color_tag(_("↓"), colors.alternative_low_color()) :
	   current_amount > last_amount ?
	      StyleManager::color_tag(_("↑"), colors.alternative_high_color()) :
	      StyleManager::color_tag(_("="), colors.alternative_medium_color());
	/** TRANSLATORS: The first placeholder is the stock amount of a ware/worker, and the second is an
	 * icon indicating a trend. Very little space is available. */
	return format(_("%1$s%2$s"), text, indicator);
}

RGBAColor
StockMenuWaresDisplay::draw_ware_background_overlay(const Widelands::DescriptionIndex di) {
	if (solid_icon_backgrounds_) {
		return WaresDisplay::draw_ware_background_overlay(di);
	}
	if (get_type() == Widelands::wwWARE) {
		if (!player_.tribe().get_ware_descr(di)->has_demand_check(player_.tribe().name())) {
			return WaresDisplay::draw_ware_background_overlay(di);
		}
	} else {
		if (!player_.tribe().get_worker_descr(di)->has_demand_check()) {
			return WaresDisplay::draw_ware_background_overlay(di);
		}
	}

	const uint32_t amount = amount_of(di);
	const uint32_t target = player_.get_total_economy_target(get_type(), di);
	const RGBColor& color =
	   amount < target ? g_style_manager->building_statistics_style().alternative_low_color() :
	   amount > target ? g_style_manager->building_statistics_style().alternative_high_color() :
	                     g_style_manager->building_statistics_style().alternative_medium_color();
	return RGBAColor(color.r, color.g, color.b, 160);
}

RGBColor AbstractWaresDisplay::info_color_for_ware(Widelands::DescriptionIndex /* ware */) {
	return g_style_manager->ware_info_style(UI::WareInfoStyle::kNormal).info_background();
}

static const char* unit_suffixes[] = {
   "%1%",
   /** TRANSLATORS: This is a large number with a suffix (e.g. 50k = 50,000). */
   /** TRANSLATORS: Space is limited, use only 1 letter for the suffix and no whitespace. */
   gettext_noop("%1%k"),
   /** TRANSLATORS: This is a large number with a suffix (e.g. 5M = 5,000,000). */
   /** TRANSLATORS: Space is limited, use only 1 letter for the suffix and no whitespace. */
   gettext_noop("%1%M"),
   /** TRANSLATORS: This is a large number with a suffix (e.g. 5G = 5,000,000,000). */
   /** TRANSLATORS: Space is limited, use only 1 letter for the suffix and no whitespace. */
   gettext_noop("%1%G")};
std::string get_amount_string(uint32_t amount, bool cutoff1k) {
	uint8_t size = 0;
	while (amount >= ((size != 0u) || cutoff1k ? 1000 : 10000)) {
		amount /= 1000;
		size++;
	}
	return format(_(unit_suffixes[size]), amount);
}

uint32_t StockMenuWaresDisplay::amount_of(const Widelands::DescriptionIndex ware) {
	MutexLock m(MutexLock::ID::kObjects);

	uint32_t totalstock = 0;
	for (const auto& economy : player_.economies()) {
		if (economy.second->type() == get_type()) {
			if (show_total_stock_) {
				totalstock += economy.second->stock_ware_or_worker(ware);
			} else {
				for (const Widelands::Warehouse* warehouse : economy.second->warehouses()) {
					totalstock += (get_type() == Widelands::wwWARE ? warehouse->get_wares() :
					                                                 warehouse->get_workers())
					                 .stock(ware);
				}
			}
		}
	}

	return totalstock;
}

std::string WaresDisplay::info_for_ware(Widelands::DescriptionIndex ware) {
	return get_amount_string(amount_of(ware));
}

UI::Box& TradeProposalWaresDisplay::create(UI::Panel* parent,
                                           const Widelands::TribeDescr& tribe,
                                           const std::string& heading,
                                           int spacing,
                                           TradeProposalWaresDisplay** result_pointer) {
	UI::Box& box = *new UI::Box(parent, UI::PanelStyle::kWui, "vbox", 0, 0, UI::Box::Vertical);
	TradeProposalWaresDisplay* impl = new TradeProposalWaresDisplay(&box, tribe);

	box.add(new UI::Textarea(&box, UI::PanelStyle::kWui, "label",
	                         UI::FontStyle::kWuiInfoPanelHeading, heading, UI::Align::kCenter),
	        UI::Box::Resizing::kFullSize);
	box.add_space(spacing);
	box.add(impl, UI::Box::Resizing::kExpandBoth);

	*result_pointer = impl;
	return box;
}

TradeProposalWaresDisplay::TradeProposalWaresDisplay(UI::Panel* parent,
                                                     const Widelands::TribeDescr& tribe)
   : WaresDisplay(parent, 0, 0, tribe, Widelands::wwWARE, true) {
	set_other(nullptr);
}

uint32_t TradeProposalWaresDisplay::amount_of(Widelands::DescriptionIndex di) {
	const auto it = config_.find(di);
	return it != config_.end() ? it->second : 0;
}

void TradeProposalWaresDisplay::set_zero() {
	config_.clear();
	for (Widelands::DescriptionIndex di : tribe().wares()) {
		unselect_ware(di);
	}
}

void TradeProposalWaresDisplay::set_other(const Widelands::TribeDescr* other) {
	for (auto it = config_.begin(); it != config_.end();) {
		if (other == nullptr || !other->has_ware(it->first)) {
			it = config_.erase(it);
		} else {
			++it;
		}
	}

	for (Widelands::DescriptionIndex di : tribe().wares()) {
		set_hidden(di, other == nullptr || !other->has_ware(di));
	}
}

void TradeProposalWaresDisplay::change(const int delta) {
	for (Widelands::DescriptionIndex di : tribe().wares()) {
		if (ware_selected(di)) {
			auto it = config_.find(di);
			if (it == config_.end()) {
				if (delta > 0) {
					config_.emplace(di, delta);
				}
			} else {
				if (it->second + delta > 0) {
					it->second += delta;
				} else {
					config_.erase(it);
				}
			}
		}
	}
}

bool TradeProposalWaresDisplay::anything_selected() const {
	return std::any_of(
	   config_.begin(), config_.end(),
	   [](const std::pair<Widelands::DescriptionIndex, int>& pair) { return pair.second > 0; });
}

Widelands::BillOfMaterials TradeProposalWaresDisplay::get_selection() const {
	Widelands::BillOfMaterials bill;
	for (const auto& pair : config_) {
		if (pair.second > 0) {
			bill.emplace_back(pair.first, pair.second);
		}
	}
	return bill;
}

void TradeProposalWaresDisplay::draw(RenderTarget& dst) {
	dst.brighten_rect(Recti(0, 0, get_inner_w(), get_inner_h()), -32);
	WaresDisplay::draw(dst);
}

std::string waremap_to_richtext(const Widelands::TribeDescr& tribe,
                                const std::map<Widelands::DescriptionIndex, uint8_t>& map) {
	std::string ret;

	const UI::WareInfoStyleInfo& style =
	   g_style_manager->ware_info_style(UI::WareInfoStyle::kNormal);

	for (const auto& column : tribe.wares_order()) {
		for (Widelands::DescriptionIndex di : column) {
			if (const auto ware_amount = map.find(di); ware_amount != map.end()) {
				ret += "<div width=30 padding=2><p align=center>"
				       "<div width=26 background=" +
				       style.icon_background().hex_value() + "><p align=center><img src=\"" +
				       tribe.get_ware_descr(ware_amount->first)->icon_filename() +
				       "\"></p></div><div width=26 background=" + style.info_background().hex_value() +
				       "><p>" + style.info_font().as_font_tag(get_amount_string(ware_amount->second)) +
				       "</p></div></p></div>";
			}
		}
	}
	return ret;
}
