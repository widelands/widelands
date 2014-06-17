-- =======================================================================
--                           Dummy Mission Map
-- =======================================================================
set_textdomain("scenario_dummy.wmf")

include "scripting/ui.lua"

p1 = wl.Game().players[1]
dummy_msg = {
   title=_ "Not yet implemented",
   body=_ "Sorry, this map is not yet implemented.",
   width=500,
   height=450,
   posy=1,
}
p1:message_box(dummy_msg.title, dummy_msg.body, dummy_msg)
