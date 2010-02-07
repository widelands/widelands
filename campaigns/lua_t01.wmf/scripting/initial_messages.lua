-- ===============================
-- Send a bunch of story messages 
-- ===============================
use("aux", "smooth_move")
use("aux", "coroutine")

function send_msg(t, m, pos)
   plr = wl.game.Player(1)
   posx = nil
   posy = nil
   if pos ~= nil then
      plr:reveal_fields(pos:region(8))
      smooth_move(pos)
      sleep(1200)
      posx = 0
      posy = 80
   end
   plr:message_box(t, m, { posx = posx, posy = posy })
   sleep(500)
end


function briefing_messages() 
-- Briefing message
title = _"The story begins"
msg = "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Thron sighs..." ..
"</p></rt><rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "It's been months and we are still hiding where the forests are old and dark.<br> My warriors hunt at day and lie awake at night - listening to the sounds of the cruel slaughter echoing from afar amongst the ancient trees." ]] .. 
"</p></rt>"
send_msg(title, msg)

msg = 
"<rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "We can see the raging flames that swallow Al'thunran from here, miles away.<br> The red lights flash in the darkness and dance to the rhythm of the wardrums that haunt me even in my nightmares." ]] .. 
"</p></rt>"
local home = wl.map.Field(12,10)
local al_thunran = wl.map.Field(53, 43)
send_msg(title, msg, al_thunran)
smooth_move(home)
sleep(1200)

msg = "<rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "My father's bones lay peacefully in the grounds he cleared from the senseless bloodspilling that has once risen among us.<br> Boldreth, my loyal companion and friend is a source of peace and comfort to me in these dark times. He keeps my spirits high and those of my warriors awake, not to let greed or despair destroy our bonds as well." ]] ..
"</p></rt>"
local grave = wl.map.Field(25,22)
send_msg(title, msg, grave)

msg = "<rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "As father told me, there will be times to fight and times to lie and wait, trying not to sleep or die before the right time comes. And so I do... wait." ]] .. 
"</p></rt>"
send_msg(title, msg, home)
sleep(2000)

use("map", "khantrukhs_talking")
end


run(briefing_messages)

