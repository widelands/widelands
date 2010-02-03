-- ===============================
-- Send a bunch of story messages 
-- ===============================
-- All will be send right after each other

function briefing_messages() 
   -- sleeptime = 15000
   sleeptime = 2
-- Briefing message
title = _"The story begins"
msg = "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Thron sighs..." ..
"</p></rt><rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "It's been months and we are still hiding where the forests are old and dark.<br> My warriors hunt at day and lie awake at night - listening to the sounds of the cruel slaughter echoing from afar amongst the ancient trees." ]] .. 
"</p></rt>"
p:message_box(title, msg)
coroutine.yield(wl.game.get_time() + sleeptime)

msg = 
"<rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "We can see the raging flames that swallow Al'thunran from here, miles away.<br> The red lights flash in the darkness and dance to the rhythm of the wardrums that haunt me even in my nightmares." ]] .. 
"</p></rt>"
p:message_box(title, msg)
-- TODO: jump to position, reveal position
coroutine.yield(wl.game.get_time() + sleeptime)

msg = "<rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "My father's bones lay peacefully in the grounds he cleared from the senseless bloodspilling that has once risen among us.<br> Boldreth, my loyal companion and friend is a source of peace and comfort to me in these dark times. He keeps my spirits high and those of my warriors awake, not to let greed or despair destroy our bonds as well." ]] ..
"</p></rt>"
p:message_box(title, msg)
-- TODO: jump to position, reveal position
coroutine.yield(wl.game.get_time() + sleeptime)

msg = "<rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "As father told me, there will be times to fight and times to lie and wait, trying not to sleep or die before the right time comes. And so I do... wait." ]] .. 
"</p></rt>"
p:message_box(title, msg)
-- TODO: jump home
coroutine.yield(wl.game.get_time() + sleeptime)

use("map", "khantrukhs_talking")
end


wl.game.run_coroutine(coroutine.create(briefing_messages))

