/*
 * Copyright (C) 2007-2020 by the Widelands Development Team
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

#include "wui/interactive_gamebase.h"

#include <memory>

#include "base/log.h"
#include "base/macros.h"
#include "base/multithreading.h"
#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/map.h"
#include "logic/map_objects/findbob.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/player.h"
#include "network/gamehost.h"
#include "wlapplication_options.h"
#include "wui/constructionsitewindow.h"
#include "wui/dismantlesitewindow.h"
#include "wui/game_chat_menu.h"
#include "wui/game_client_disconnected.h"
#include "wui/game_exit_confirm_box.h"
#include "wui/game_main_menu_save_game.h"
#include "wui/game_options_sound_menu.h"
#include "wui/game_summary.h"
#include "wui/interactive_player.h"
#include "wui/militarysitewindow.h"
#include "wui/productionsitewindow.h"
#include "wui/shipwindow.h"
#include "wui/trainingsitewindow.h"
#include "wui/unique_window_handler.h"
#include "wui/warehousewindow.h"

namespace {

std::string speed_string(int const speed) {
	if (speed) {
		return (boost::format("%u.%ux") % (speed / 1000) % (speed / 100 % 10)).str();
	}
	return _("PAUSE");
}

constexpr uint8_t kSpeedSlow = 250;
constexpr uint16_t kSpeedDefault = 1000;
constexpr uint16_t kSpeedFast = 10000;
}  // namespace

InteractiveGameBase::InteractiveGameBase(Widelands::Game& g,
                                         Section& global_s,
                                         bool const multiplayer,
                                         ChatProvider* chat_provider)
   : InteractiveBase(g, global_s),
     chat_provider_(chat_provider),
     multiplayer_(multiplayer),
     showhidemenu_(toolbar(),
                   "dropdown_menu_showhide",
                   0,
                   0,
                   34U,
                   10,
                   34U,
                   /** TRANSLATORS: Title for a menu button in the game. This menu will show/hide
                      building spaces, census, statistics */
                   _("Show / Hide"),
                   UI::DropdownType::kPictorialMenu,
                   UI::PanelStyle::kWui,
                   UI::ButtonStyle::kWuiPrimary),
     mainmenu_(toolbar(),
               "dropdown_menu_main",
               0,
               0,
               34U,
               10,
               34U,
               /** TRANSLATORS: Title for the main menu button in the game */
               as_tooltip_text_with_hotkey(_("Main Menu"), pgettext("hotkey", "Esc")),
               UI::DropdownType::kPictorialMenu,
               UI::PanelStyle::kWui,
               UI::ButtonStyle::kWuiPrimary),
     gamespeedmenu_(toolbar(),
                    "dropdown_menu_gamespeed",
                    0,
                    0,
                    34U,
                    10,
                    34U,
                    /** TRANSLATORS: Title for a menu button in the game. This menu will show
                       options o increase/decrease the gamespeed, and to pause the game */
                    _("Game Speed"),
                    UI::DropdownType::kPictorialMenu,
                    UI::PanelStyle::kWui,
                    UI::ButtonStyle::kWuiPrimary) {
	buildingnotes_subscriber_ = Notifications::subscribe<Widelands::NoteBuilding>(
	   [this](const Widelands::NoteBuilding& note) {
		   switch (note.action) {
		   case Widelands::NoteBuilding::Action::kFinishWarp: {
			   if (upcast(
			          Widelands::Building const, building, game().objects().get_object(note.serial))) {
				   const Widelands::Coords coords = building->get_position();
				   // Check whether the window is wanted
				   if (wanted_building_windows_.count(coords.hash()) == 1) {
					   const WantedBuildingWindow& wanted_building_window =
					      *wanted_building_windows_.at(coords.hash());
					   UI::UniqueWindow* building_window =
					      show_building_window(coords, true, wanted_building_window.show_workarea);
					   building_window->set_pos(wanted_building_window.window_position);
					   if (wanted_building_window.minimize) {
						   building_window->minimize();
					   }
					   building_window->set_pinned(wanted_building_window.pin);
					   wanted_building_windows_.erase(coords.hash());
				   }
			   }
		   } break;
		   default:
			   break;
		   }
	   });

	if (chat_provider_ != nullptr) {
		chat_overlay()->set_chat_provider(*chat_provider_);
	}
}

void InteractiveGameBase::add_main_menu() {
	mainmenu_.set_image(g_image_cache->get("images/wui/menus/main_menu.png"));
	toolbar()->add(&mainmenu_);

#ifndef NDEBUG  //  only in debug builds
	/** TRANSLATORS: An entry in the game's main menu */
	mainmenu_.add(_("Script Console"), MainMenuEntry::kScriptConsole,
	              g_image_cache->get("images/wui/menus/lua.png"), false,
	              /** TRANSLATORS: Tooltip for Script Console in the game's main menu */
	              "", pgettext("hotkey", "F6"));
#endif

	menu_windows_.sound_options.open_window = [this] {
		new GameOptionsSoundMenu(*this, menu_windows_.sound_options);
	};
	/** TRANSLATORS: An entry in the game's main menu */
	mainmenu_.add(_("Sound Options"), MainMenuEntry::kOptions,
	              g_image_cache->get("images/wui/menus/options.png"), false,
	              /** TRANSLATORS: Tooltip for Sound Options in the game's main menu */
	              _("Set sound effect and music options"));

	menu_windows_.savegame.open_window = [this] {
		new GameMainMenuSaveGame(*this, menu_windows_.savegame);
	};
	/** TRANSLATORS: An entry in the game's main menu */
	mainmenu_.add(_("Save Game"), MainMenuEntry::kSaveMap,
	              g_image_cache->get("images/wui/menus/save_game.png"));

	mainmenu_.add(
	   /** TRANSLATORS: An entry in the game's main menu */
	   _("Exit Game"), MainMenuEntry::kExitGame, g_image_cache->get("images/wui/menus/exit.png"));

	mainmenu_.selected.connect([this] { main_menu_selected(mainmenu_.get_selected()); });
}

void InteractiveGameBase::main_menu_selected(MainMenuEntry entry) {
	switch (entry) {
#ifndef NDEBUG  //  only in debug builds
	case MainMenuEntry::kScriptConsole: {
		GameChatMenu::create_script_console(this, debugconsole_, *DebugConsole::get_chat_provider());
	} break;
#endif
	case MainMenuEntry::kOptions: {
		menu_windows_.sound_options.toggle();
	} break;
	case MainMenuEntry::kSaveMap: {
		menu_windows_.savegame.toggle();
	} break;
	case MainMenuEntry::kExitGame: {
		if (SDL_GetModState() & KMOD_CTRL) {
			end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
		} else {
			new GameExitConfirmBox(*this, *this);
		}
	} break;
	}
}

void InteractiveGameBase::add_showhide_menu() {
	showhidemenu_.set_image(g_image_cache->get("images/wui/menus/showhide.png"));
	toolbar()->add(&showhidemenu_);

	rebuild_showhide_menu();
	showhidemenu_.selected.connect([this] { showhide_menu_selected(showhidemenu_.get_selected()); });
}

void InteractiveGameBase::rebuild_showhide_menu() {
	const ShowHideEntry last_selection =
	   showhidemenu_.has_selection() ? showhidemenu_.get_selected() : ShowHideEntry::kBuildingSpaces;
	showhidemenu_.clear();

	/** TRANSLATORS: An entry in the game's show/hide menu to toggle whether building spaces are
	 * shown */
	showhidemenu_.add(buildhelp() ? _("Hide Building Spaces") : _("Show Building Spaces"),
	                  ShowHideEntry::kBuildingSpaces,
	                  g_image_cache->get("images/wui/menus/toggle_buildhelp.png"), false, "",
	                  pgettext("hotkey", "Space"));

	/** TRANSLATORS: An entry in the game's show/hide menu to toggle whether building names are shown
	 */
	showhidemenu_.add(get_display_flag(dfShowCensus) ? _("Hide Census") : _("Show Census"),
	                  ShowHideEntry::kCensus,
	                  g_image_cache->get("images/wui/menus/toggle_census.png"), false, "", "C");

	showhidemenu_.add(get_display_flag(dfShowStatistics) ?
	                     /** TRANSLATORS: An entry in the game's show/hide menu to toggle whether
	                      * building statistics are shown */
	                     _("Hide Statistics") :
	                     _("Show Statistics"),
	                  ShowHideEntry::kStatistics,
	                  g_image_cache->get("images/wui/menus/toggle_statistics.png"), false, "", "S");

	showhidemenu_.add(get_display_flag(dfShowSoldierLevels) ?
	                     /** TRANSLATORS: An entry in the game's show/hide menu to toggle whether
	                      * level information is shown above soldiers' heads */
	                     _("Hide Soldier Levels") :
	                     _("Show Soldier Levels"),
	                  ShowHideEntry::kSoldierLevels,
	                  g_image_cache->get("images/wui/menus/toggle_soldier_levels.png"), false, "",
	                  "L");

	showhidemenu_.add(get_display_flag(dfShowBuildings) ?
	                     /** TRANSLATORS: An entry in the game's show/hide menu to toggle whether
	                      * buildings are greyed out */
	                     _("Hide Buildings") :
	                     _("Show Buildings"),
	                  ShowHideEntry::kBuildings,
	                  g_image_cache->get("images/wui/stats/genstats_nrbuildings.png"), false, "",
	                  "U");

	showhidemenu_.select(last_selection);
}

void InteractiveGameBase::showhide_menu_selected(ShowHideEntry entry) {
	switch (entry) {
	case ShowHideEntry::kBuildingSpaces: {
		toggle_buildhelp();
	} break;
	case ShowHideEntry::kCensus: {
		set_display_flag(dfShowCensus, !get_display_flag(dfShowCensus));
	} break;
	case ShowHideEntry::kStatistics: {
		set_display_flag(dfShowStatistics, !get_display_flag(dfShowStatistics));
	} break;
	case ShowHideEntry::kSoldierLevels: {
		set_display_flag(dfShowSoldierLevels, !get_display_flag(dfShowSoldierLevels));
	} break;
	case ShowHideEntry::kBuildings: {
		set_display_flag(dfShowBuildings, !get_display_flag(dfShowBuildings));
	} break;
	case ShowHideEntry::kWorkareaOverlap: {
		set_display_flag(dfShowWorkareaOverlap, !get_display_flag(dfShowWorkareaOverlap));
	} break;
	}
	showhidemenu_.toggle();
}

void InteractiveGameBase::add_gamespeed_menu() {
	gamespeedmenu_.set_image(g_image_cache->get("images/wui/menus/gamespeed.png"));
	toolbar()->add(&gamespeedmenu_);
	rebuild_gamespeed_menu();
	gamespeedmenu_.selected.connect(
	   [this] { gamespeed_menu_selected(gamespeedmenu_.get_selected()); });
}

void InteractiveGameBase::rebuild_gamespeed_menu() {
	const GameSpeedEntry last_selection =
	   gamespeedmenu_.has_selection() ? gamespeedmenu_.get_selected() : GameSpeedEntry::kIncrease;

	gamespeedmenu_.clear();

	gamespeedmenu_.add(_("Speed +"), GameSpeedEntry::kIncrease,
	                   g_image_cache->get("images/wui/menus/gamespeed_increase.png"), false,
	                   /** TRANSLATORS: Tooltip for Speed + in the game's game speed menu */
	                   _("Increase the game speed"), pgettext("hotkey", "Page Up"));

	gamespeedmenu_.add(_("Speed -"), GameSpeedEntry::kDecrease,
	                   g_image_cache->get("images/wui/menus/gamespeed_decrease.png"), false,
	                   /** TRANSLATORS: Tooltip for Speed - in the game's game speed menu */
	                   _("Decrease the game speed"), pgettext("hotkey", "Page Down"));

	if (!is_multiplayer()) {
		if (get_game()->game_controller() && get_game()->game_controller()->is_paused()) {
			gamespeedmenu_.add(_("Resume"), GameSpeedEntry::kPause,
			                   g_image_cache->get("images/wui/menus/gamespeed_resume.png"), false,
			                   /** TRANSLATORS: Tooltip for Pause in the game's game speed menu */
			                   _("Resume the Game"), pgettext("hotkey", "Pause"));
		} else {
			gamespeedmenu_.add(_("Pause"), GameSpeedEntry::kPause,
			                   g_image_cache->get("images/wui/menus/gamespeed_pause.png"), false,
			                   /** TRANSLATORS: Tooltip for Pause in the game's game speed menu */
			                   _("Pause the Game"), pgettext("hotkey", "Pause"));
		}
	}

	gamespeedmenu_.select(last_selection);
}

void InteractiveGameBase::gamespeed_menu_selected(GameSpeedEntry entry) {
	switch (entry) {
	case GameSpeedEntry::kIncrease: {
		increase_gamespeed(SDL_GetModState() & KMOD_SHIFT ?
		                      kSpeedSlow :
		                      SDL_GetModState() & KMOD_CTRL ? kSpeedFast : kSpeedDefault);
		// Keep the window open so that the player can click this multiple times
		gamespeedmenu_.toggle();
	} break;
	case GameSpeedEntry::kDecrease: {
		decrease_gamespeed(SDL_GetModState() & KMOD_SHIFT ?
		                      kSpeedSlow :
		                      SDL_GetModState() & KMOD_CTRL ? kSpeedFast : kSpeedDefault);
		// Keep the window open so that the player can click this multiple times
		gamespeedmenu_.toggle();
	} break;
	case GameSpeedEntry::kPause: {
		if (!is_multiplayer()) {
			toggle_game_paused();
			gamespeedmenu_.toggle();
		}
	} break;
	}
}

void InteractiveGameBase::add_chat_ui() {
	add_toolbar_button("wui/menus/chat", "chat", _("Chat"), &chat_, true);
	chat_.open_window = [this] {
		if (chat_provider_) {
			GameChatMenu::create_chat_console(this, chat_, *chat_provider_);
		}
	};
}

void InteractiveGameBase::increase_gamespeed(uint16_t speed) {
	if (GameController* const ctrl = get_game()->game_controller()) {
		uint32_t const current_speed = ctrl->desired_speed();
		ctrl->set_desired_speed(current_speed + speed);
	}
}

void InteractiveGameBase::decrease_gamespeed(uint16_t speed) {
	if (GameController* const ctrl = get_game()->game_controller()) {
		uint32_t const current_speed = ctrl->desired_speed();
		ctrl->set_desired_speed(current_speed > speed ? current_speed - speed : 0);
	}
}

void InteractiveGameBase::reset_gamespeed() {
	if (GameController* const ctrl = get_game()->game_controller()) {
		ctrl->set_desired_speed(kSpeedDefault);
	}
}

void InteractiveGameBase::toggle_game_paused() {
	if (GameController* const ctrl = get_game()->game_controller()) {
		ctrl->toggle_paused();
		// Toggle Pause / Resume in the menu
		rebuild_gamespeed_menu();
	}
}

bool InteractiveGameBase::handle_key(bool down, SDL_Keysym code) {
	if (InteractiveBase::handle_key(down, code)) {
		return true;
	}
	const bool numpad_diagonalscrolling = get_config_bool("numpad_diagonalscrolling", false);

	if (down) {
		switch (code.sym) {
		case SDLK_KP_9:
		case SDLK_PAGEUP:
			if (code.sym == SDLK_KP_9 && ((code.mod & KMOD_NUM) || numpad_diagonalscrolling)) {
				break;
			}
			increase_gamespeed(
			   code.mod & KMOD_SHIFT ? kSpeedSlow : code.mod & KMOD_CTRL ? kSpeedFast : kSpeedDefault);
			return true;
		case SDLK_PAUSE:
			if (code.mod & KMOD_SHIFT) {
				reset_gamespeed();
			} else {
				toggle_game_paused();
			}
			return true;
		case SDLK_KP_3:
		case SDLK_PAGEDOWN:
			if (code.sym == SDLK_KP_3 && ((code.mod & KMOD_NUM) || numpad_diagonalscrolling)) {
				break;
			}
			decrease_gamespeed(
			   code.mod & KMOD_SHIFT ? kSpeedSlow : code.mod & KMOD_CTRL ? kSpeedFast : kSpeedDefault);
			return true;

		case SDLK_c:
			set_display_flag(
			   InteractiveBase::dfShowCensus, !get_display_flag(InteractiveBase::dfShowCensus));
			return true;

		case SDLK_g:
			menu_windows_.stats_general.toggle();
			return true;

		case SDLK_l:
			set_display_flag(dfShowSoldierLevels, !get_display_flag(dfShowSoldierLevels));
			return true;

		case SDLK_u:
			set_display_flag(dfShowBuildings, !get_display_flag(dfShowBuildings));
			return true;

		case SDLK_s:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
				new GameMainMenuSaveGame(*this, menu_windows_.savegame);
			} else {
				set_display_flag(dfShowStatistics, !get_display_flag(dfShowStatistics));
			}
			return true;

		case SDLK_ESCAPE:
			InteractiveGameBase::toggle_mainmenu();
			return true;

		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			if (chat_provider_) {
				if (!chat_.window) {
					GameChatMenu::create_chat_console(this, chat_, *chat_provider_);
				}
				return dynamic_cast<GameChatMenu*>(chat_.window)->enter_chat_message();
			}
			break;

		default:
			break;
		}
	}
	return false;
}

/// \return a pointer to the running \ref Game instance.
Widelands::Game* InteractiveGameBase::get_game() const {
	return dynamic_cast<Widelands::Game*>(&egbase());
}

Widelands::Game& InteractiveGameBase::game() const {
	return dynamic_cast<Widelands::Game&>(egbase());
}

void InteractiveGameBase::draw_overlay(RenderTarget& dst) {
	InteractiveBase::draw_overlay(dst);

	GameController* game_controller = game().game_controller();
	// Display the gamespeed.
	if (game_controller != nullptr) {
		std::string game_speed;
		uint32_t const real = game_controller->real_speed();
		uint32_t const desired = game_controller->desired_speed();
		if (real == desired) {
			if (real != 1000) {
				game_speed = speed_string(real);
			}
		} else {
			game_speed = (boost::format
			              /** TRANSLATORS: actual_speed (desired_speed) */
			              (_("%1$s (%2$s)")) %
			              speed_string(real) % speed_string(desired))
			                .str();
		}

		if (!game_speed.empty()) {
			std::shared_ptr<const UI::RenderedText> rendered_text = UI::g_fh->render(
			   as_richtext_paragraph(game_speed, UI::FontStyle::kWuiGameSpeedAndCoordinates));
			rendered_text->draw(dst, Vector2i(get_w() - 5, 5), UI::Align::kRight);
		}
	}
}

void InteractiveGameBase::set_sel_pos(Widelands::NodeAndTriangle<> const center) {
	InteractiveBase::set_sel_pos(center);

	const Widelands::Map& map = egbase().map();

	// If we have an immovable, we might want to show a tooltip
	Widelands::BaseImmovable* imm = map[center.node].get_immovable();
	if (imm == nullptr) {
		return set_tooltip("");
	}

	// If we have a player, only show tooltips if he sees the field
	const Widelands::Player* player = nullptr;
	if (upcast(InteractivePlayer, iplayer, this)) {
		player = iplayer->get_player();
		if (player != nullptr && !player->see_all() &&
		    (!player->is_seeing(Widelands::Map::get_index(center.node, map.get_width())))) {
			return set_tooltip("");
		}
	}

	if (imm->descr().type() == Widelands::MapObjectType::IMMOVABLE) {
		// Trees, Resource Indicators, fields ...
		return set_tooltip(imm->descr().descname());
	} else if (upcast(Widelands::ProductionSite, productionsite, imm)) {
		// No productionsite tips for hostile players
		if (player == nullptr || !player->is_hostile(*productionsite->get_owner())) {
			return set_tooltip(
			   productionsite->info_string(Widelands::Building::InfoStringFormat::kTooltip));
		}
	}
	set_tooltip("");
}

/**
 * Called for every game after loading (from a savegame or just from a map
 * during single/multiplayer/scenario).
 */
void InteractiveGameBase::postload() {
	// Recalc whole map for changed owner stuff
	egbase().mutable_map()->recalc_whole_map(egbase());

	// Close game-relevant UI windows (but keep main menu open)
	fieldaction_.destroy();
	hide_minimap();
}

void InteractiveGameBase::start() {
	// Multiplayer games don't save the view position, so we go to the starting position instead
	if (is_multiplayer()) {
		Widelands::PlayerNumber pln = player_number();
		const Widelands::PlayerNumber max = game().map().get_nrplayers();
		if (pln == 0) {
			// Spectator, use the view of the first viable player
			for (pln = 1; pln <= max; ++pln) {
				if (game().get_player(pln)) {
					break;
				}
			}
		}
		// Adding a check, just in case there was no viable player found for spectator
		if (game().get_player(pln)) {
			map_view()->scroll_to_field(game().map().get_starting_pos(pln), MapView::Transition::Jump);
		}
	}
}

void InteractiveGameBase::toggle_mainmenu() {
	mainmenu_.toggle();
}

void InteractiveGameBase::add_wanted_building_window(const Widelands::Coords& coords,
                                                     const Vector2i point,
                                                     bool was_minimal,
                                                     bool was_pinned) {
	wanted_building_windows_.insert(std::make_pair(
	   coords.hash(), std::unique_ptr<const WantedBuildingWindow>(new WantedBuildingWindow(
	                     point, was_minimal, was_pinned, has_workarea_preview(coords)))));
}

UI::UniqueWindow* InteractiveGameBase::show_building_window(const Widelands::Coords& coord,
                                                            bool avoid_fastclick,
                                                            bool workarea_preview_wanted) {
	Widelands::BaseImmovable* immovable = game().map().get_immovable(coord);
	upcast(Widelands::Building, building, immovable);
	assert(building);
	UI::UniqueWindow::Registry& registry =
	   unique_windows().get_registry((boost::format("building_%d") % building->serial()).str());

	switch (building->descr().type()) {
	case Widelands::MapObjectType::CONSTRUCTIONSITE:
		registry.open_window = [this, &registry, building, avoid_fastclick, workarea_preview_wanted] {
			new ConstructionSiteWindow(*this, registry,
			                           *dynamic_cast<Widelands::ConstructionSite*>(building),
			                           avoid_fastclick, workarea_preview_wanted);
		};
		break;
	case Widelands::MapObjectType::DISMANTLESITE:
		registry.open_window = [this, &registry, building, avoid_fastclick] {
			new DismantleSiteWindow(
			   *this, registry, *dynamic_cast<Widelands::DismantleSite*>(building), avoid_fastclick);
		};
		break;
	case Widelands::MapObjectType::MILITARYSITE:
		registry.open_window = [this, &registry, building, avoid_fastclick, workarea_preview_wanted] {
			new MilitarySiteWindow(*this, registry, *dynamic_cast<Widelands::MilitarySite*>(building),
			                       avoid_fastclick, workarea_preview_wanted);
		};
		break;
	case Widelands::MapObjectType::PRODUCTIONSITE:
		registry.open_window = [this, &registry, building, avoid_fastclick, workarea_preview_wanted] {
			new ProductionSiteWindow(*this, registry,
			                         *dynamic_cast<Widelands::ProductionSite*>(building),
			                         avoid_fastclick, workarea_preview_wanted);
		};
		break;
	case Widelands::MapObjectType::TRAININGSITE:
		registry.open_window = [this, &registry, building, avoid_fastclick, workarea_preview_wanted] {
			new TrainingSiteWindow(*this, registry, *dynamic_cast<Widelands::TrainingSite*>(building),
			                       avoid_fastclick, workarea_preview_wanted);
		};
		break;
	case Widelands::MapObjectType::WAREHOUSE:
		registry.open_window = [this, &registry, building, avoid_fastclick, workarea_preview_wanted] {
			new WarehouseWindow(*this, registry, *dynamic_cast<Widelands::Warehouse*>(building),
			                    avoid_fastclick, workarea_preview_wanted);
		};
		break;
	// TODO(sirver,trading): Add UI for market.
	default:
		log_err_time(egbase().get_gametime(), "Unable to show window for building '%s', type '%s'.\n",
		             building->descr().name().c_str(), to_string(building->descr().type()).c_str());
		NEVER_HERE();
	}
	NoteThreadSafeFunction::instantiate([&registry]() { registry.create(); }, true);
	return registry.window;
}

/**
 * See if we can reasonably open a ship window at the current selection position.
 * If so, do it and return true; otherwise, return false.
 */
bool InteractiveGameBase::try_show_ship_window() {
	const Widelands::Map& map = game().map();
	Widelands::Area<Widelands::FCoords> area(map.get_fcoords(get_sel_pos().node), 1);

	if (!(area.field->nodecaps() & Widelands::MOVECAPS_SWIM)) {
		return false;
	}

	std::vector<Widelands::Bob*> ships;
	if (map.find_bobs(egbase(), area, &ships, Widelands::FindBobShip())) {
		for (Widelands::Bob* ship : ships) {
			if (can_see(ship->owner().player_number())) {
				// FindBobShip should have returned only ships
				assert(ship->descr().type() == Widelands::MapObjectType::SHIP);
				show_ship_window(dynamic_cast<Widelands::Ship*>(ship));
				return true;
			}
		}
	}
	return false;
}

void InteractiveGameBase::show_ship_window(Widelands::Ship* ship) {
	UI::UniqueWindow::Registry& registry =
	   unique_windows().get_registry((boost::format("ship_%d") % ship->serial()).str());
	registry.open_window = [this, &registry, ship] { new ShipWindow(*this, registry, ship); };
	registry.create();
}

void InteractiveGameBase::show_game_summary() {
	NoteThreadSafeFunction::instantiate(
	   [this]() {
		   if (game_summary_.window) {
			   game_summary_.window->set_visible(true);
			   game_summary_.window->think();
			   return;
		   }
		   new GameSummaryScreen(this, &game_summary_);
	   },
	   true);
}

bool InteractiveGameBase::show_game_client_disconnected() {
	assert(is_a(GameHost, get_game()->game_controller()));
	if (!client_disconnected_.window) {
		if (upcast(GameHost, host, get_game()->game_controller())) {
			new GameClientDisconnected(this, client_disconnected_, host);
			return true;
		}
	}
	return false;
}
