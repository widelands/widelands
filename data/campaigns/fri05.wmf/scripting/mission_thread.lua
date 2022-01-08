function mission_thread()
   scroll_to_field(map.player_slots[1].starting_field)
   include "map:scripting/starting_conditions.lua"
   local reveal_fields = {}
   for x=0,121 do
      for y=17,240 do
         local f = map:get_field(x, y)
         if p1:sees_field(f) then
            p1:reveal_fields({f})  -- Make currently seen fields permanently visible without hiding them first
         else
            table.insert(reveal_fields, f)
         end
      end
   end
   p2:reveal_fields(reveal_fields)
   reveal_randomly(p1, reveal_fields, 500)
   reveal_fields = nil

   
end

run(mission_thread)
