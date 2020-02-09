include "scripting/richtext.lua"

set_textdomain("texts")
tips = {
   {
      -- TRANSLATORS: %s = URL to the Widelands website
      text = (_"Our computer players are too boring? Visit our website at %s and meet other players to play online."):bformat(u("widelands.org")),
      seconds = 6
   },
}
return tips
