include "scripting/richtext.lua"
include "txts/help/common_helptexts.lua"

push_textdomain("texts")
tips = {
   {
      text = _("Widelands is creating an emergency save, so you can continue playing exactly where the game closed."),
      seconds = 60
   },
}
pop_textdomain()
return tips
