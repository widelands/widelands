-- =======================================================================
--                            Main Mission Thread                           
-- =======================================================================

use("map", "mission_thread_texts")
use("aux", "smooth_move")
use("aux", "table")

function send_msg(t) 
   p:message_box( t.title, t.body, t)
end

function add_obj(t)
   local o = p:add_objective(t.name, t.title, t.body)
   if t.check then
      run(t.check, o)
   end
   return o
end

-- TODO: this function was also used in mission 01, make it public someplace
function exist_buildings(region, t) 
   carr = {}
   for idx,f in ipairs(region) do
      if f.immovable then
         if carr[f.immovable.name] == nil then
            carr[f.immovable.name] = 1
         else
            carr[f.immovable.name] = carr[f.immovable.name] + 1
         end
      end
   end
   for house,count in pairs(t) do
      if carr[house] == nil or carr[house] < count then
         return false
      end
   end
   return true
end

function introduction_thread()
   sleep(2000)

   send_msg(briefing_msg_1)
   send_msg(briefing_msg_2)
   send_msg(briefing_msg_3)

   send_msg(order_msg_1)
   send_msg(order_msg_2)

   -- Reveal the rocks
   add_obj(obj_claim_northeastern_rocks)

   local rocks = wl.map.Field(27, 48)
   p:reveal_fields(rocks:region(6))
   pts = smooth_move(rocks, p, 3000)
   sleep(3000)

   send_msg(order_msg_3)
   send_msg(order_msg_4)

   -- Move back to HQ
   timed_move(reverse(pts), p, 10)
   sleep(1000)
end

function mines_and_food_thread()
   local f1 = wl.map.Field(30, 15)
   local f2 = wl.map.Field(24, 61)

   -- Sleep until we see the mountains
   while not p:seen_field(f1) and not p:seen_field(f2) do
      sleep(4000)
   end

   -- Send a msg and add the objective
   send_msg(order_msg_6_geologist)
   o = add_obj(obj_build_mines)
   p:allow_buildings{
      "coalmine", 
      "oremine", 
      "goldmine", 
      "granitemine"
   }
      
   -- Wait for completion 
   local fields = array_combine(
      wl.map.Field(32,16):region(8),
      wl.map.Field(37,8):region(8)
   )

   while not exist_buildings(fields, {coalmine = 1, oremine = 1}) do
      sleep(5000)
   end
   o.done = true
   send_msg(order_msg_7_mines_up)
   send_msg(order_msg_8_mines_up)
   send_msg(order_msg_9_hunter)
   send_msg(order_msg_10_bread)

   -- TODO: checking for these to finish
   add_obj(obj_basic_food)
   add_obj(obj_begin_farming)

   -- Enable food production
   p:allow_buildings{
      "hunters_hut",
      "gamekeepers_hut",
      "tavern",
      "farm",
      "well",
      "bakery",
   }


end

run(introduction_thread)
run(mines_and_food_thread)
