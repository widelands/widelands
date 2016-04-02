set_textdomain("tribes_encyclopedia")

include "scripting/formatting.lua"
include "txts/help/common_helptexts.lua"

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
               help_toggle_minimap_hotkey() ..
               help_toggle_building_spaces_hotkey() ..
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
               dl(help_format_hotkey(pgettext("hotkey", "(Ctrl +) 0-9")), _"Remember and go to previously remembered locations") ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey(","), _"Go to the previous location") ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey("."), _"Go to the next location") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + F11")), _"Take a screenshot") ..
               help_toggle_fullscreen_hotkey() ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "Ctrl + F10")), _"Quit the game immediately") ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey(pgettext("hotkey", "F6")), _"Show the debug console (only in debug-builds)")
         ) ..

         h3(_"In the message window, the following additional shortcuts are available:") ..
         p(
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey("0"), _"Show all messages") ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey("1"), _"Show geologists’ messages only") ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey("2"), _"Show economy messages only") ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey("3"), _"Show seafaring messages only") ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey("4"), _"Show warfare messages only") ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey("5"), _"Show scenario messages only") ..
               -- TRANSLATORS: This is an access key combination.
               dl(help_format_hotkey("G"), _"Jump to the location corresponding to the current message") ..
               -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
               dl(help_format_hotkey(pgettext("hotkey", "Delete")), _"Archive/Restore the current message")
          )
      )
}
