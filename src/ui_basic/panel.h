/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_PANEL_H
#define WL_UI_BASIC_PANEL_H

#include <atomic>
#include <deque>
#include <list>
#include <memory>
#include <set>
#include <vector>

#include <SDL_keyboard.h>

#include "base/macros.h"
#include "base/multithreading.h"
#include "base/rect.h"
#include "base/vector.h"
#include "base/wexception.h"
#include "graphic/styles/panel_styles.h"
#include "notifications/signal.h"
#include "sound/constants.h"

class FileWrite;
class RenderTarget;
namespace Widelands {
struct MapObjectSaver;
}

namespace UI {

/**
 * Panel is a basic rectangular UI element.
 *
 * Every panel has an outer rectangle and an inner rectangle. The inner rectangle
 * is always equal to the outer rectangle minus the border size. Child panel coordinates
 * are always relative to the inner rectangle.
 *
 * Every panel has an actual size and a desired size. In general, a panel should never
 * change its own actual size, but only its desired size. It is up to the parent panel
 * to adjust the actual size based on the desired size.
 *
 * The desired size is a quantity that is computed in \ref update_desired_size based
 * only on the attributes of this panel and of its children. The actual size may differ
 * from this, and when it does, a panel may have to adapt also its children's positions
 * and actual sizes in \ref layout.
 *
 * If a panel is the top-level panel, or if has \ref set_layout_toplevel, then whenever
 * its desired size changes, this automatically changes the actual size (which then invokes
 * \ref layout and \ref move_inside_parent).
 */
class Panel {
public:
	// Panel flags. The `Panel::flags_` attribute is a bitset of these flags.
	enum {
		pf_handle_mouse = 1 << 0,  ///< receive mouse events
		pf_thinks = 1 << 1,        ///< call think() function during run
		pf_top_on_click = 1 << 2,  ///< bring panel on top when clicked inside it
		pf_die = 1 << 3,           ///< this panel needs to die
		pf_child_die = 1 << 4,     ///< a child needs to die
		pf_visible = 1 << 5,       ///< render the panel
		pf_can_focus = 1 << 6,     ///< can receive the keyboard focus
		/// children should snap only when overlapping the snap target
		pf_snap_windows_only_when_overlapping = 1 << 7,
		/// children should snap to the edges of this panel
		pf_dock_windows_to_edges = 1 << 8,
		/// whether any change in the desired size should propagate to the actual size
		pf_layout_toplevel = 1 << 9,
		/// whether widget wants to receive unicode textinput messages
		pf_handle_textinput = 1 << 10,
		/// whether widget and its children will handle any key presses
		pf_handle_keypresses = 1 << 11,
		// has a non-empty logic_think() implementation
		pf_logic_think = 1 << 12,
		// Do not draw any tooltips and stuff
		pf_hide_all_overlays = 1 << 13,
	};

	Panel(Panel* const nparent,
	      UI::PanelStyle,
	      int32_t const nx,
	      int32_t const ny,
	      int const nw,
	      int const nh,
	      const std::string& tooltip_text = std::string());
	virtual ~Panel();

	Notifications::Signal<> clicked;
	Notifications::Signal<> position_changed;

	Panel* get_parent() const {
		return parent_;
	}

	void free_children();

	// Checks whether this panel will be deleted in the next think cycle.
	// This check is used to stop panels from thinking and performing other
	// activities to prevent undesired side-effects.
	bool is_dying() const {
		return flags_ & pf_die;
	}

	// Modal
	enum class Returncodes { kBack, kOk };

	template <typename Returncode> Returncode run() {
		return static_cast<Returncode>(do_run());
	}
	int do_run();

	/**
	 * Cause run() to return as soon as possible, with the given return code
	 */
	template <typename Returncode> void end_modal(const Returncode& code) {
		running_ = false;
		return_code_ = static_cast<int>(code);
	}

	bool is_modal() const;

	virtual void start();
	virtual void end();
	virtual void become_modal_again(Panel& prevmodal);

	// Geometry
	virtual void set_size(int nw, int nh);
	virtual void set_desired_size(int w, int h);
	virtual void set_pos(Vector2i);
	virtual void move_inside_parent();
	virtual void layout();

	void set_layout_toplevel(bool ltl);
	bool get_layout_toplevel() const;

	void get_desired_size(int* w, int* h) const;

	int32_t get_x() const {
		return x_;
	}
	int32_t get_y() const {
		return y_;
	}
	Vector2i get_pos() const {
		return Vector2i(x_, y_);
	}
	// int instead of uint because of overflow situations
	int32_t get_w() const {
		return w_;
	}
	int32_t get_h() const {
		return h_;
	}

	Vector2i to_parent(const Vector2i&) const;

	virtual bool is_snap_target() const {
		return false;
	}
	uint16_t get_border_snap_distance() const {
		return border_snap_distance_;
	}
	void set_border_snap_distance(uint8_t const value) {
		border_snap_distance_ = value;
	}
	uint8_t get_panel_snap_distance() const {
		return panel_snap_distance_;
	}
	void set_panel_snap_distance(uint8_t const value) {
		panel_snap_distance_ = value;
	}
	bool get_snap_windows_only_when_overlapping() const {
		return flags_ & pf_snap_windows_only_when_overlapping;
	}
	void set_snap_windows_only_when_overlapping(const bool on = true);
	bool get_dock_windows_to_edges() const {
		return flags_ & pf_dock_windows_to_edges;
	}
	void set_dock_windows_to_edges(const bool on = true);
	void set_inner_size(int nw, int nh);
	void set_border(int l, int r, int t, int b);

	int get_lborder() const {
		return lborder_;
	}
	int get_rborder() const {
		return rborder_;
	}
	int get_tborder() const {
		return tborder_;
	}
	int get_bborder() const {
		return bborder_;
	}

	int get_inner_w() const;
	int get_inner_h() const;

	const Panel* get_next_sibling() const {
		return next_;
	}
	Panel* get_next_sibling() {
		return next_;
	}
	const Panel* get_prev_sibling() const {
		return prev_;
	}
	Panel* get_prev_sibling() {
		return prev_;
	}
	const Panel* get_first_child() const {
		return first_child_;
	}
	Panel* get_first_child() {
		return first_child_;
	}
	const Panel* get_last_child() const {
		return last_child_;
	}
	Panel* get_last_child() {
		return last_child_;
	}

	void move_to_top();

	// Drawing, visibility
	bool is_visible() const {
		return flags_ & pf_visible;
	}
	void set_visible(bool on);

	virtual void draw(RenderTarget&);
	virtual void draw_border(RenderTarget&);
	virtual void draw_overlay(RenderTarget&);

	// Events
	virtual void think();
	virtual void game_logic_think() {
		// Overridden only by InteractiveBase
	}

	void set_logic_think() {
		flags_ |= pf_logic_think;
	}
	void set_hide_all_overlays() {
		flags_ |= pf_hide_all_overlays;
	}

	Vector2i get_mouse_position() const;
	void set_mouse_pos(Vector2i);
	void center_mouse();

	virtual void handle_mousein(bool inside);
	virtual bool handle_mousepress(uint8_t btn, int32_t x, int32_t y);
	virtual bool handle_mouserelease(uint8_t btn, int32_t x, int32_t y);
	virtual bool handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	virtual bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate);
	virtual bool handle_key(bool down, SDL_Keysym);
	virtual bool handle_textinput(const std::string& text);
	virtual bool handle_tooltip();

	/// \returns whether a certain given is currently down.
	///
	/// \note Never call this function from a keyboard event handler (a function
	/// that overrides bool handle_key(bool, SDL_Keysym code)) to get the state
	/// of a modifier key. In that case code.mod must be used. It contains the
	/// state of the modifier keys at the time of the event. Unfortunately there
	/// is no information about modifier key states in mouse events (tracker
	/// item #1916453). That is "a huge oversight" in SDL 1.2 and a fix is
	/// promised in SDL 1.3:
	/// http://lists.libsdl.org/pipermail/sdl-libsdl.org/2008-March/064560.html
	bool get_key_state(SDL_Scancode) const;

	void set_handle_mouse(bool yes);
	void grab_mouse(bool grab);

	void set_can_focus(bool yes);
	bool get_can_focus() const {
		return flags_ & pf_can_focus;
	}
	bool has_focus() const {
		return (get_can_focus() && parent_->focus_ == this);
	}
	virtual void focus(bool topcaller = true);
	Panel* focused_child() const {
		return focus_;
	}

	void set_top_on_click(bool const on) {
		if (on)
			flags_ |= pf_top_on_click;
		else
			flags_ &= ~pf_top_on_click;
	}
	bool get_top_on_click() const {
		return flags_ & pf_top_on_click;
	}

	static void set_allow_user_input(bool const t) {
		allow_user_input_ = t;
	}
	static bool allow_user_input() {
		return allow_user_input_;
	}

	void set_tooltip(const std::string&);
	const std::string& tooltip() const {
		return tooltip_;
	}

	virtual void die();
	static void register_click();

	static void logic_thread();

	// Internal notification to reduce an active mutex lock's scope.
	void clear_current_think_mutex();

	/*
	 * Every panel that is semantically a toplevel panel needs to call
	 * this at least once, ideally near the end of the most derived class's
	 * constructor. This will make this panel and all its children visible
	 * and allow them to start thinking and handling user input.
	 */
	void initialization_complete();

	// Notify this panel's parent of our death, then immediately delete us.
	void do_delete();

	// overridden by InteractiveBase
	virtual bool extended_tooltip_accessibility_mode() const {
		return false;
	}

	void find_all_children_at(int16_t x, int16_t y, std::vector<Panel*>& result) const;

	Panel& get_topmost_forefather();

	struct ModalGuard {
		explicit ModalGuard(Panel& p);
		~ModalGuard();

	private:
		Panel* bottom_panel_;
		Panel& top_panel_;
		DISALLOW_COPY_AND_ASSIGN(ModalGuard);
	};

	// Call this on the topmost panel after you changed the template directory
	void template_directory_changed();

	enum class SaveType {  // Do not change the order – these indices are stored in savegames!
		kNone = 0,          ///< This panel is not saveable.
		kBuildingWindow,
		kWatchWindow,
		kConfigureEconomy,
		kStockMenu,
		kGeneralStats,
		kWareStats,
		kBuildingStats,
		kSoldierStats,
		kSeafaringStats,
		kMessages,
		kObjectives,
		kMinimap,
		kEncyclopedia,
		kShipWindow,
		kAttackWindow,
	};
	virtual SaveType save_type() const {
		return SaveType::kNone;
	}
	virtual void save(FileWrite&, Widelands::MapObjectSaver&) const {
		NEVER_HERE();
	}

protected:
	// This panel will never receive keypresses (do_key), instead
	// textinput will be passed on (do_textinput).
	void set_handle_textinput(bool const on = true) {
		if (on) {
			flags_ |= pf_handle_textinput;
		} else {
			flags_ &= ~pf_handle_textinput;
		}
	}

	/*
	 * Only call when you know what you are doing.
	 * Usually, you can rely on our focus() mechanism to activate/deactivate text handling
	 * but there might be corner cases where disabling text input specifically is needed.
	 */
	void disable_sdl_textinput();

	// If this is set to 'true', this panel ad its children will never receive keypresses (do_key) or
	// textinput (do_textinput).
	void set_handle_keypresses(bool const on) {
		if (on) {
			flags_ |= pf_handle_keypresses;
		} else {
			flags_ &= ~pf_handle_keypresses;
		}
	}

	// Defines if think() should be called repeatedly. This is true on construction.
	void set_thinks(bool yes);

	bool keyboard_free() {
		return !(focus_);
	}

	virtual void update_desired_size();

	static void play_click();

	static bool
	draw_tooltip(const std::string& text, PanelStyle, Vector2i pos = Vector2i::invalid());
	void draw_background(RenderTarget& dst, const UI::PanelStyleInfo&);
	void draw_background(RenderTarget& dst, Recti rect, const UI::PanelStyleInfo&) const;

	// called after the template directory was changed
	virtual void update_template() {
	}

	virtual Panel* get_open_dropdown();

	virtual bool is_focus_toplevel() const;

	virtual std::vector<Recti> focus_overlay_rects();
	// Convenience functions for overriding focus_overlay_rects()
	std::vector<Recti> focus_overlay_rects(int off_x, int off_y, int strength_diff) const;

	// Wait until the current logic frame has ended
	void wait_for_current_logic_frame();
	// Can be called during a lengthy wait to ensure that
	// notes are still handled and the graphics refreshed.
	// Does _not_ handle user input (this is on purpose!!)
	void stay_responsive();

	const PanelStyle panel_style_;

	/** Never call this function, except when you need Widelands to stay responsive
	 *  during a costly operation and you can guarantee that it will not interfere
	 *  with the "normal" graphics refreshing done periodically from `Panel::do_run`.
	 *  If the argument is not empty, the screen will be greyed out with the
	 *  provided message to the user drawn in the screen center.
	 *  May be called only by the initializer thread.
	 */
	void do_redraw_now(bool handle_input = true,
	                   const std::string& message_to_display = std::string());

private:
	bool initialized_;

	bool handles_mouse() const {
		return (flags_ & pf_handle_mouse) != 0;
	}

	bool handles_keypresses() const {
		if (get_parent() != nullptr && !get_parent()->handles_keypresses()) {
			return false;
		}
		return (flags_ & pf_handle_keypresses) != 0;
	}

	bool handles_textinput() const {
		return (flags_ & pf_handle_textinput) != 0;
	}
	bool thinks() const {
		return (flags_ & pf_thinks) != 0;
	}

	void check_child_death();
	virtual void on_death(Panel* p);
	virtual void on_visibility_changed();

	friend struct ProgressWindow;
	friend class Window;
	void do_draw(RenderTarget&);
	void do_draw_inner(RenderTarget&);
	void do_think();

	Panel* child_at_mouse_cursor(int32_t mouse_x, int32_t mouse_y, Panel* child);
	void do_mousein(bool inside);
	bool do_mousepress(const uint8_t btn, int32_t x, int32_t y);
	bool do_mouserelease(const uint8_t btn, int32_t x, int32_t y);
	bool do_mousemove(const uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	bool do_mousewheel(int32_t x, int32_t y, uint16_t modstate, Vector2i rel_mouse_pos);
	bool do_key(bool down, SDL_Keysym code);
	bool do_textinput(const std::string& text);
	bool do_tooltip();

	bool handle_tab_pressed(bool reverse);
	std::deque<Panel*> gather_focusable_children();

	static Panel* ui_trackmouse(int32_t& x, int32_t& y);
	static bool ui_mousepress(const uint8_t button, int32_t x, int32_t y);
	static bool ui_mouserelease(const uint8_t button, int32_t x, int32_t y);
	static bool
	ui_mousemove(const uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	static bool ui_mousewheel(int32_t x, int32_t y, uint16_t modstate);
	static bool ui_key(bool down, SDL_Keysym code);
	static bool ui_textinput(const std::string& text);

	Panel* parent_;
	Panel* next_;
	Panel* prev_;
	Panel* first_child_;
	Panel* last_child_;
	Panel* mousein_child_;  //  child panel that the mouse is in
	Panel* focus_;          //  keyboard focus

	std::atomic<uint32_t> flags_;

	/**
	 * The outer rectangle is defined by (x_, y_, w_, h_)
	 */
	/*@{*/
	int32_t x_, y_;
	int w_, h_;
	/*@}*/
	int lborder_, rborder_, tborder_, bborder_;
	uint8_t border_snap_distance_, panel_snap_distance_;
	int desired_w_, desired_h_;

	friend struct ModalGuard;

	bool running_;
	int return_code_;

	std::string tooltip_;
	static std::atomic<Panel*> modal_;
	static std::atomic<Panel*> mousegrab_;
	static Panel* mousein_;
	static Panel* tooltip_panel_;
	static Vector2i tooltip_fixed_pos_;
	static Recti tooltip_fixed_rect_;
	static bool allow_user_input_;

	static FxId click_fx_;

	enum class LogicThreadState { kFree, kLocked, kEndingRequested, kEndingConfirmed };
	std::atomic<LogicThreadState> logic_thread_locked_;
	static std::atomic_bool logic_thread_running_;

	std::unique_ptr<Notifications::Subscriber<NoteThreadSafeFunction>> subscriber1_;
	std::unique_ptr<Notifications::Subscriber<NoteThreadSafeFunctionHandled>> subscriber2_;
	void handle_notes();
	std::list<NoteThreadSafeFunction> notes_;
	std::set<uint32_t> handled_notes_;
	std::unique_ptr<MutexLock> current_think_mutex_;

	DISALLOW_COPY_AND_ASSIGN(Panel);
};

inline void Panel::set_snap_windows_only_when_overlapping(const bool on) {
	flags_ &= ~pf_snap_windows_only_when_overlapping;
	if (on)
		flags_ |= pf_snap_windows_only_when_overlapping;
}
inline void Panel::set_dock_windows_to_edges(const bool on) {
	flags_ &= ~pf_dock_windows_to_edges;
	if (on)
		flags_ |= pf_dock_windows_to_edges;
}

/**
 * A Panel with a name. Important for scripting
 */
struct NamedPanel : public Panel {
	NamedPanel(Panel* const nparent,
	           UI::PanelStyle s,
	           const std::string& name,
	           int32_t const nx,
	           int32_t const ny,
	           int const nw,
	           int const nh,
	           const std::string& tooltip_text = std::string())
	   : Panel(nparent, s, nx, ny, nw, nh, tooltip_text), name_(name) {
	}

	const std::string& get_name() const {
		return name_;
	}

private:
	std::string name_;
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_PANEL_H
