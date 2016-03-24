set_textdomain("widelands_editor")

include "scripting/formatting.lua"
include "txts/help/common_helptexts.lua"

return {
   title = _"General Help",
   text =
      title(_[[The Widelands Editor]]) ..
      rt(
         h1(_"Introduction") ..

         p(_"This editor is intended for players who would like to design their own maps to use with Widelands.") ..
         p(_"As you can see, this editor is heavy work in progress and as the editor becomes better and better, this text will also get longer and more complete.") ..

         h1(_"Keyboard Shortcuts") ..

         p(
            -- TRANSLATORS: This is an access key combination. Do not change the key.
            dl(_"Space:", _"Toggle building spaces") ..
            -- TRANSLATORS: This is an access key combination. Do not change the key.
            dl(_"C:", _"Toggle census") ..
            -- TRANSLATORS: This is an access key combination. Do not change the key.
            dl(_"F:", _"Toggle fullscreen") ..
            -- TRANSLATORS: This is an access key combination. Do not change the key.
            dl(_"H:", _"Toggle main menu") ..
            -- TRANSLATORS: This is an access key combination. Do not change the key.
            dl(_"M:", _"Toggle minimap") ..
            -- TRANSLATORS: This is an access key combination. Do not change the key.
            dl(_"P:", _"Toggle player menu") ..
            -- TRANSLATORS: This is an access key combination. Do not change the key.
            dl(_"T:", _"Toggle tools menu") ..
            -- TRANSLATORS: This is an access key combination. Do not change the key.
            dl(_"I:", _"Activate information tool") ..
            -- TRANSLATORS: This is an access key combination. Do not change the key.
            dl(_"Ctrl + L:", _"Load map") ..
            -- TRANSLATORS: This is an access key combination. Do not change the key.
            dl(_"Ctrl + S:", _"Save map") ..
            -- TRANSLATORS: This is an access key combination. Do not change the key.
            dl(_"1-0:", _"Change tool size") ..
            -- TRANSLATORS: This is an access key combination. Do not change the key.
            dl(_"Shift (Hold):", _"First alternative tool while pressed") ..
            -- TRANSLATORS: This is an access key combination. Do not change the key.
            dl(_"Alt (Hold):", _"Second alternative tool while pressed")
         ) ..

         h2(_"Editor Help") ..
         -- TRANSLATORS: %s is a representation of the ? button.
         p(_"For a detailed description of the trees and terrains, use the %s button on the bottom right."):bformat(b("?")) ..
         help_online_help() ..
         p(_"The wiki also includes a short tutorial on how to build a map.")
      )
}
