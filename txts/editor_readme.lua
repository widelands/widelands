include "scripting/formatting.lua"

set_textdomain("texts")

function a(text)
   return "</p><p font-size=14 font-decoration=underline>" .. text
end

return {
   title = _"README for the Widelands Editor",
   text = rt(
      "<rt><p font-size=38 font-color=2F9131>" .. _[[The Widelands Editor]] .. "<br><br></p>" ..

      h1(_"Introduction") ..

      p(_"This editor is intended for players who would like to design their own maps to use with Widelands. As you can see, this Editor is heavy work in progress and as the Editor becomes better and better, this text will also get longer and more complete.") ..
      p(_"This Editor is intended for players who would like to design their own maps to use with Widelands.") ..
      p(_"As you can see, this Editor is heavy work in progress and as the Editor becomes better and better, this text will also get longer and more complete.") ..
      p(_"Please have a look at our enduser manuals on our wiki pages at: %s"):bformat(a("http://wl.widelands.org/wiki")) ..
      p(_"Those pages might be more complete and up to date than this file, and you will also find a short tutorial about building a map there.") ..

      h1(_"Keyboard shortcuts") ..

      p(
         -- TRANSLATORS: This is a hotkey
         _"SPACE:" .. " " .. _"toggles if building spaces are shown" .. "<br>" ..
         -- TRANSLATORS: This is a hotkey
         _"C:" .. " " .. _"toggles census" .. "<br>" ..
         -- TRANSLATORS: This is a hotkey
         _"F:" .. " " .. _"toggles fullscreen" .. "<br>" ..
         -- TRANSLATORS: This is a hotkey
         _"H:" .. " " .. _"toggles main menu" .. "<br>" ..
         -- TRANSLATORS: This is a hotkey
         _"M:" .. " " .. _"toggles minimap" .. "<br>" ..
         -- TRANSLATORS: This is a hotkey
         _"P:" .. " " .. _"toggles player menu" .. "<br>" ..
         -- TRANSLATORS: This is a hotkey
         _"T:" .. " " .. _"toggles tools menu" .. "<br>" ..
         -- TRANSLATORS: This is a hotkey
         _"I:" .. " " .. _"activates information tool" .. "<br>" ..
         -- TRANSLATORS: This is a hotkey
         _"CTRL+L:" .. " " .. _"load map" .. "<br>" ..
         -- TRANSLATORS: This is a hotkey
         _"CTRL+S:" .. " " .. _"save map" .. "<br>" ..
         -- TRANSLATORS: This is a hotkey
         _"1-0:" .. " " .. _"changes tool size" .. "<br>" ..
         -- TRANSLATORS: This is a hotkey
         _"SHIFT (Hold):" .. " " .. _"selects first alternative tool while pressed" .. "<br>" ..
         -- TRANSLATORS: This is a hotkey
         _"ALT (Hold):" .. " " .. _"Selects second alternative tool while pressed" .. "<br>"
      )
   )
}
