-- ===============================
-- Send a bunch of story messages 
-- ===============================

--use("aux", "formatting")
use("aux", "format_scenario")

function thron(title, text)
   return speech("map:chieftain.png", "2F9131", title, text)
end


function briefing_messages() 
-- Briefing message
title = _"The story begins"
msg = thron(_"Thron sighs...",
_[[It's been months and we are still hiding where the forests are old and dark.<br> My warriors hunt at day and lie awake at night - listening to the sounds of the cruel slaughter echoing from afar amongst the ancient trees.]]) 
show_story_box(title, msg)

msg = 
thron(
_[[We can see the raging flames that swallow Al'thunran from here, miles away.<br> The red lights flash in the darkness and dance to the rhythm of the wardrums that haunt me even in my nightmares.]])
show_story_box(title, msg, al_thunran)

msg = thron(
_[[My father's bones rest peacefully in the grounds on which he once ended the senseless bloodspilling that has risen among us. It pains me that his peace only endured for one generation.<br> Boldreth, my loyal companion and friend is a source of peace and comfort to me in these dark times. He keeps my spirits high and those of my warriors awake, not to let greed or despair destroy the bonds between us as well.]])
show_story_box(title, msg, grave)

msg = thron(
_[[As father told me, there are times to fight and times to lie and wait, trying not to fall asleep or die before the right time comes. And so I do... wait.]])
show_story_box(title, msg)
sleep(2000)

use("map", "khantrukhs_talking")
end


run(briefing_messages)

