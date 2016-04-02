set_textdomain("widelands_editor")

include "scripting/formatting.lua"
include "txts/help/common_helptexts.lua"

return {
   title = _"Keyboard Shortcuts",
   text =
      rt(
         p(
            -- TRANSLATORS: This is an access key combination.
            dl(help_format_hotkey("H"), _"Toggle main menu") ..
            -- TRANSLATORS: This is an access key combination. The hotkey is 't'
            dl(help_format_hotkey("T"), _"Toggle tools menu") ..
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey(pgettext("hotkey", "1-0")), _"Change tool size") ..
            help_toggle_minimap_hotkey() ..
            help_toggle_building_spaces_hotkey() ..
            -- TRANSLATORS: This is an access key combination. The hotkey is 'p'
            dl(help_format_hotkey("P"), _"Toggle player menu") ..
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey("Ctrl + Z"), _"Undo") ..
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey("Ctrl + Y"), _"Redo") ..
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey("F1"), _"Help") ..
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey(pgettext("hotkey", "Shift (Hold)")), _"First alternative tool while pressed") ..
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey(pgettext("hotkey", "Alt (Hold)")), _"Second alternative tool while pressed") ..
            -- TRANSLATORS: This is an access key combination. The hotkey is 'i'
            dl(help_format_hotkey("I"), _"Activate information tool") ..
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey(pgettext("hotkey", "Ctrl + L")), _"Load map") ..
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey(pgettext("hotkey", "Ctrl + S")), _"Save map") ..
            help_toggle_fullscreen_hotkey()
         )
      )
}
