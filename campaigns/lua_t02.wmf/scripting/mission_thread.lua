-- =======================================================================
--                            Main Mission Thread                           
-- =======================================================================

use("map", "mission_thread_texts")
use("aux", "smooth_move")
use("aux", "table")

function send_msg(t) 
   p:message_box( t.title, t.body, t)
end


function mission_thread()
   sleep(2000)

   send_msg(briefing_msg_1)
   send_msg(briefing_msg_2)
   send_msg(briefing_msg_3)

   send_msg(order_msg_1)
   send_msg(order_msg_2)

   -- Reveal the rocks
   local rocks = wl.map.Field(27, 48)
   p:reveal_fields(rocks:region(6))

   pts = smooth_move(rocks, p, 3000)
   sleep(3000)

   send_msg(order_msg_3)
   send_msg(order_msg_4)

   timed_move(reverse(pts), p, 10)
   sleep(1000)

   -- TODO: objective!
end


run(mission_thread)
