-- ======================
-- Messages by Khantrukh
-- ======================

include "map:scripting/texts.lua"
include "scripting/coroutine.lua"
include "scripting/objective_utils.lua"

function check_quarries()
   while not check_for_buildings(p, {quarry = 2},
      wl.Game().map:get_field(8,13):region(3)) do sleep(5000) end
   objq.done = true
end

function check_ranger()
   while not check_for_buildings(p, {rangers_hut = 1},
      wl.Game().map:get_field(17,11):region(3)) do sleep(5000) end
   objr.done = true
end

function tutorial_thread()
   p = wl.Game().players[1]
   show_story_box(_"Somebody Comes up to You", khantrukh_1)
   show_story_box(_"The Advisor", khantrukh_2, nil, 80, 80)
   local o = add_obj(start_lumberjack_01, p)

   -- Wait till the hut is build.
   while not check_for_buildings(p, {constructionsite = 1},
      wl.Game().map:get_field(15,11):region(2)) do sleep(5000) end

   show_story_box(_"The Advisor", khantrukh_3, nil, 80, 80)

   -- Wait till the hut is build.
   while not check_for_buildings(p, {lumberjacks_hut = 1},
      wl.Game().map:get_field(15,11):region(2)) do sleep(5000) end
   o.done = true

   p:message_box(_"The Advisor", khantrukh_4, { h = 400 })
   p:message_box(_"The Advisor", khantrukh_5, { h = 400 })
   local o = add_obj(start_lumberjack_02, p)

   -- Wait till the hut is build.
   while not check_for_buildings(p, {lumberjacks_hut = 1},
         wl.Game().map:get_field(12,13):region(2)) do sleep(5000) end
   o.done = true

   p:message_box(_"The Advisor", khantrukh_6, { h = 400 })
   p:allow_buildings{"rangers_hut", "quarry"}
   objq = add_obj(start_quarries, p)
   objr = add_obj(start_ranger, p)
   run(check_ranger)
   run(check_quarries)

   p:message_box(_"The Advisor", khantrukh_7, { h = 400 })
   while not (objr.done and objq.done) do
      sleep(5000)
   end

   p:reveal_scenario("barbariantut01")
   p:message_box(_"Mission Complete", khantrukh_8, { h = 400 })
end

run(tutorial_thread)
