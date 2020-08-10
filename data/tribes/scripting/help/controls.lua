push_textdomain("tribes_encyclopedia")

include "scripting/richtext.lua"
include "txts/help/common_helptexts.lua"

local toggle_minimap_hotkey = help_toggle_minimap_hotkey()
local toggle_building_spaces_hotkey = help_toggle_building_spaces_hotkey()
local toggle_fullscreen_hotkey = help_toggle_fullscreen_hotkey()

local r = {
   title = _"Controls",
   text =
         h2(_"Window Control") ..
         p(
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Right-click")), _"Close window") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Middle-click or Ctrl + Left-click")), _"Minimize/Maximize window") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Left-click on Button")), _"Skip confirmation dialog")) ..

         h2(_"Table Control") ..
         h3(_"In tables that allow the selection of multiple entries, the following key combinations are available:") ..
         p(
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Click")), pgettext("table_control", "Select multiple entries")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Shift + Click")), pgettext("table_control", "Select a range of entries")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + A")), pgettext("table_control", "Select all entries"))) ..

         h2(_"Road Control") ..
         p(
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Left-click")), _"While connecting two flags: Place flags automatically") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Left-click")), _"While removing a flag: Remove all flags up to the first junction")) ..

         h2(_"Keyboard Shortcuts") ..
            p(
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Page Up")), _"Increase game speed") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Page Down")), _"Decrease game speed") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Page Up")), _"Increase game speed by 10x") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Page Down")), _"Decrease game speed by 10x") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Shift + Page Up")), _"Increase game speed by 0.25x") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Shift + Page Down")), _"Decrease game speed by 0.25x") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Pause")), _"Pause the game") ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'c'
               dl(help_format_hotkey("C"), _"Toggle census") ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 's'
               dl(help_format_hotkey("S"), _"Toggle statistics") ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'l'
               dl(help_format_hotkey("L"), _"Toggle soldier health bars and level icons") ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'u'
               dl(help_format_hotkey("U"), _"Toggle building visibility") ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'w'
               dl(help_format_hotkey("W"), _"Toggle workarea overlap indicators") ..
               toggle_minimap_hotkey ..
               toggle_building_spaces_hotkey ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'o'
               dl(help_format_hotkey("O"), _"Toggle objectives") ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'n'
               dl(help_format_hotkey("N"), _"Toggle messages (‘news’)") ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey("F1"), _"Tribal Encyclopedia") ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'i'
               dl(help_format_hotkey("I"), _"Toggle stock inventory") ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'b'
               dl(help_format_hotkey("B"), _"Toggle building statistics") ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'x'
               dl(help_format_hotkey("X"), _"Toggle soldier statistics") ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'e'
               dl(help_format_hotkey("E"), _"Toggle seafaring statistics") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Arrow keys")), _"Move the map") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Arrow keys")), _"Move the map fast") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Shift + Arrow keys")), _"Move the map slowly") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Home")), _"Center main mapview on starting location") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + 1-9")), _"Remember current location") ..
               dl(help_format_hotkey(pgettext("hotkey", "1-9")), _"Go to previously remembered location") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", ", (comma)")), _"Go to previous location") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", ". (period)")), _"Go to next location") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + +")), _"Increase zoom") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + -")), _"Decrease zoom") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + 0")), _"Reset zoom") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + F11")), _"Take a screenshot") ..
               toggle_fullscreen_hotkey ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "F6")), _"Show the debug console (only in debug-builds)")
         ) ..

         -- TRANSLATORS: Heading in "Controls" help
         h2(_"Message Window") ..
         p(
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Alt + 0")), _"Show all messages") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Alt + 1")), _"Show geologists’ messages only") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Alt + 2")), _"Show economy messages only") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Alt + 3")), _"Show seafaring messages only") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Alt + 4")), _"Show warfare messages only") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Alt + 5")), _"Show scenario messages only") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey("G"), _"Jump to the location corresponding to the current message") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Delete")), _"Archive/Restore the current message")
          ) ..

         -- TRANSLATORS: Heading in "Controls" help
         h2(_"Ship Statistics") ..
         p(
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Alt + 0")), _"Show all ships") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Alt + 1")), _"Show idle ships") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Alt + 2")), _"Show ships shipping wares and workers") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Alt + 3")), _"Show waiting expeditions") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Alt + 4")), _"Show scouting expeditions") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Alt + 5")), _"Show expeditions that have found a port space or are founding a colony") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey("G"), _"Center the map on the selected ship") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey("O"), _"Open the selected ship’s window") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey("CTRL + O"), _"Go to the selected ship and open its window") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey("W"), _"Watch the selected ship")
         )
}
pop_textdomain()
return r
