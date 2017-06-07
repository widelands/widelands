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
               -- TRANSLATORS: This is an access key combination. The hotkey is 'l'
               dl(help_format_hotkey("L"), _"Toggle soldier health bars and level icons") ..
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

         h2(_"Table Control") ..
         h3(_"In tables that allow the selection of multiple entries, the following key combinations are available:") ..
         p(
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Click")), pgettext("table_control", "Select multiple entries")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Shift + Click")), pgettext("table_control", "Select a range of entries")) ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + A")), pgettext("table_control", "Select all entries"))) ..

         h2(_"Message Window") ..
         h3(_"In the message window, the following additional shortcuts are available:") ..
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
          )
      )
}
