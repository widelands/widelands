push_textdomain("tribes_encyclopedia")

include "scripting/richtext.lua"
include "txts/help/common_helptexts.lua"

local toggle_minimap_hotkey = help_toggle_minimap_hotkey()
local toggle_building_spaces_hotkey = help_toggle_building_spaces_hotkey()
local toggle_fullscreen_hotkey = help_toggle_fullscreen_hotkey()

local r = {
   title = _("Controls"),
   text =
         h2(_("Window Control")) ..
         p(
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Right-click")), _("Close window")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Middle-click or Ctrl + Left-click")), _("Minimize/Maximize window")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Left-click on Button")), _("Skip confirmation dialog"))) ..

         h2(_("Table Control")) ..
         h3(_("In tables that allow the selection of multiple entries, the following key combinations are available:")) ..
         p(
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Click")), pgettext("table_control", "Select multiple entries")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Shift + Click")), pgettext("table_control", "Select a range of entries")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(wl.ui.get_shortcut("selectall")), pgettext("table_control", "Select all entries"))) ..

         h2(_("Road Control")) ..
         p(
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Left-click")), _("While connecting two flags: Place flags automatically")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Left-click")), _("While removing a flag: Remove all flags up to the first junction"))) ..

         h2(_("Keyboard Shortcuts")) ..
            p(
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_speed_up")), _("Increase game speed")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_speed_down")), _("Decrease game speed")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_speed_up_fast")), _("Increase game speed by 10x")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_speed_down_fast")), _("Decrease game speed by 10x")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_speed_up_slow")), _("Increase game speed by 0.25x")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_speed_down_slow")), _("Decrease game speed by 0.25x")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_pause")), _("Pause the game")) ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'c'
               dl(help_format_hotkey(wl.ui.get_shortcut("game_showhide_census")), _("Toggle census")) ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 's'
               dl(help_format_hotkey(wl.ui.get_shortcut("game_showhide_stats")), _("Toggle status")) ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'l'
               dl(help_format_hotkey(wl.ui.get_shortcut("game_showhide_soldiers")), _("Toggle soldier health bars and level icons")) ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'u'
               dl(help_format_hotkey(wl.ui.get_shortcut("game_showhide_buildings")), _("Toggle building visibility")) ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'w'
               dl(help_format_hotkey(wl.ui.get_shortcut("game_showhide_workareas")), _("Toggle workarea overlap indicators")) ..
               toggle_minimap_hotkey ..
               toggle_building_spaces_hotkey ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'o'
               dl(help_format_hotkey(wl.ui.get_shortcut("game_objectives")), _("Toggle objectives")) ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'n'
               dl(help_format_hotkey(wl.ui.get_shortcut("game_messages")), _("Toggle messages (‘news’)")) ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("encyclopedia")), _("Tribal Encyclopedia")) ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'i'
               dl(help_format_hotkey(wl.ui.get_shortcut("game_stats_stock")), _("Toggle stock inventory")) ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'b'
               dl(help_format_hotkey(wl.ui.get_shortcut("game_stats_buildings")), _("Toggle building statistics")) ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'x'
               dl(help_format_hotkey(wl.ui.get_shortcut("game_stats_soldiers")), _("Toggle soldier statistics")) ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'e'
               dl(help_format_hotkey(wl.ui.get_shortcut("game_stats_seafaring")), _("Toggle seafaring statistics")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Arrow keys")), _("Move the map")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Arrow keys")), _("Move the map fast")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Shift + Arrow keys")), _("Move the map slowly")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_hq")), _("Center main mapview on starting location")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(help_set_landmarks_hotkeys()), _("Remember current location")) ..
               dl(help_format_hotkey(help_goto_landmarks_hotkeys()), _("Go to previously remembered location")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(wl.ui.get_shortcut("quicknav_prev")), _("Go to previous location")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(wl.ui.get_shortcut("quicknav_next")), _("Go to next location")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(wl.ui.get_shortcut("zoom_in")), _("Increase zoom")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(wl.ui.get_shortcut("zoom_out")), _("Decrease zoom")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(wl.ui.get_shortcut("zoom_reset")), _("Reset zoom")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(wl.ui.get_shortcut("screenshot")), _("Take a screenshot")) ..
               toggle_fullscreen_hotkey ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Shift + Space")), _("Show the debug console (only in debug-builds)"))
         ) ..

         -- TRANSLATORS: Heading in "Controls" help
         h2(_("Message Window")) ..
         p(
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_msg_filter_all")), _("Show all messages")) ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_msg_filter_geo")), _("Show geologists’ messages only")) ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_msg_filter_eco")), _("Show economy messages only")) ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_msg_filter_seafaring")), _("Show seafaring messages only")) ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_msg_filter_warfare")), _("Show warfare messages only")) ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_msg_filter_scenario")), _("Show scenario messages only")) ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_msg_goto")), _("Jump to the location corresponding to the current message")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(wl.ui.get_shortcut("delete")), _("Archive/Restore the current message"))
          ) ..

         -- TRANSLATORS: Heading in "Controls" help
         h2(_("Ship Statistics")) ..
         p(
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_sfstats_filter_all")), _("Show all ships")) ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_sfstats_filter_idle")), _("Show idle ships")) ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_sfstats_filter_ship")), _("Show ships shipping wares and workers")) ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_sfstats_filter_wait")), _("Show waiting expeditions")) ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_sfstats_filter_scout")), _("Show scouting expeditions")) ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_sfstats_filter_port")), _("Show expeditions that have found a port space or are founding a colony")) ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_sfstats_goto")), _("Center the map on the selected ship")) ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_sfstats_open")), _("Open the selected ship’s window")) ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_sfstats_open_goto")), _("Go to the selected ship and open its window")) ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(wl.ui.get_shortcut("game_sfstats_watch")), _("Watch the selected ship"))
         )
}
pop_textdomain()
return r
