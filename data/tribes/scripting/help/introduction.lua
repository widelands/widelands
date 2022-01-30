push_textdomain("tribes_encyclopedia")

include "scripting/richtext.lua"
include "txts/help/common_helptexts.lua"

local r = {
   title = _("About Widelands"),
   text =
         help_introduction() ..
         help_online_help()
}
pop_textdomain()
return r
