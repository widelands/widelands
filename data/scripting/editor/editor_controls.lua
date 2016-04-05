set_textdomain("widelands_editor")

include "scripting/formatting.lua"
include "txts/help/common_helptexts.lua"

return {
   title = _"Controls",
   text =
      rt(
         h2(_"Keyboard Shortcuts") ..
         p(
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey("F1"), _"Help") ..
            -- TRANSLATORS: This is an access key combination.
            dl(help_format_hotkey("H"), _"Toggle main menu") ..
            -- TRANSLATORS: This is an access key combination. The hotkey is 't'
            dl(help_format_hotkey("T"), _"Toggle tools menu") ..
            help_toggle_minimap_hotkey() ..
            help_toggle_building_spaces_hotkey() ..
            -- TRANSLATORS: This is an access key combination. The hotkey is 'p'
            dl(help_format_hotkey("P"), _"Toggle player menu") ..
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey("Ctrl + Z"), _"Undo") ..
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey("Ctrl + Y"), _"Redo") ..
            -- TRANSLATORS: This is an access key combination. The hotkey is 'i'
            dl(help_format_hotkey("I"), _"Activate information tool") ..
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey(pgettext("hotkey", "Ctrl + L")), _"Load map") ..
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey(pgettext("hotkey", "Ctrl + S")), _"Save map") ..
            help_toggle_fullscreen_hotkey()
         ) ..

         h2(_"Tools") ..
         p(
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey(pgettext("hotkey", "Click")), _"Place new elements on the map, or increase map elements by the selected value") ..
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey(pgettext("hotkey", "1-0")), _"Change tool size")
         ) ..

         h3(_"Height and Ressources Tools") ..
         p(
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey(pgettext("hotkey", "Shift + Click")), _"Decrease map elements by the value selected by ‘Increase/Decrease value’") ..
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Click")), _"Set map elements to the value selected by ‘Set Value’")
          ) ..

          h3(_"Noise Height Tool") ..
          p(
            -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
            dl(help_format_hotkey(pgettext("hotkey", "Shift or Ctrl + Click")), _"Set height to the value selected by ‘Set Value’")
          )
      )
}
