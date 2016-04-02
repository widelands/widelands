set_textdomain("tribes_encyclopedia")

include "scripting/formatting.lua"
include "txts/help/common_helptexts.lua"

return {
   title = _"About Widelands",
   text =
      rt(
         help_introduction() ..
         help_online_help()
      )
}
