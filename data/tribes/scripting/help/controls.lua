set_textdomain("tribes_encyclopedia")

include "scripting/formatting.lua"
include "txts/help/common_helptexts.lua"

local toggle_minimap_hotkey = help_toggle_minimap_hotkey()
local toggle_building_spaces_hotkey = help_toggle_building_spaces_hotkey()
local toggle_fullscreen_hotkey = help_toggle_fullscreen_hotkey()

set_textdomain("tribes_encyclopedia")

return {
   title = _"Controls",
   text =
      rt(
         h2(_"Window Control") ..
         p(
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Right-click")), _"Close window") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Middle-click or Ctrl + Left-click")), _"Minimize/Maximize window") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Left-click on Button")), _"Skip confirmation dialog")) ..

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
               dl(help_format_hotkey(pgettext("hotkey", "Pause")), _"Pause the game") ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 'c'
               dl(help_format_hotkey("C"), _"Toggle census") ..
               -- TRANSLATORS: This is an access key combination. The hotkey is 's'
               dl(help_format_hotkey("S"), _"Toggle statistics") ..
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
               -- TRANSLATORS: This is an access key combination. The hotkey is 'p'
               dl(help_format_hotkey("E"), _"Toggle seafaring statistics") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Home")), _"Center main mapview on starting location") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + 1-9")), _"Remember current location") ..
               dl(help_format_hotkey(pgettext("hotkey", "1-9")), _"Go to previously remembered location") ..
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
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + F10")), _"Quit the game immediately") ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "F6")), _"Show the debug console (only in debug-builds)")
         ) ..

         -- TRANSLATORS: Heading in "Controls" help
         h3(_"Message Window") ..
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
         h3(_"Ship Statistics") ..
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
               dl(help_format_hotkey(pgettext("hotkey", "Alt + 5")), _"Show expeditions with port space found") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey("G"), _"Center the map on the selected ship") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey("O"), _"Go to the selected ship and open its window") ..
               -- TRANSLATORS: This is the helptext for an access key combination.
               dl(help_format_hotkey("W"), _"Watch the selected ship")
          )
      )
}
