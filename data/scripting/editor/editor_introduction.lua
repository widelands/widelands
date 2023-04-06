include "scripting/richtext.lua"
include "txts/help/common_helptexts.lua"

local online_help = help_online_help()

push_textdomain("widelands_editor")
local result = {
   title = _("The Widelands Editor"),
   text =
      h1(_("Introduction")) ..

      p(_("This editor is intended for players who would like to design their own maps to use with Widelands.")) ..
      p(_("As you can see, this editor is heavy work in progress and as the editor becomes better and better, this text will also get longer and more complete.")) ..

      online_help ..
      p(_("The wiki also includes a short tutorial on how to build a map."))
}

pop_textdomain()
return result
