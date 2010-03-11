-- ======================
-- Messages by Khantrukh 
-- ======================

use("map", "texts")
use("aux", "objective_utils")

function check_quarries() 
   while not check_for_buildings(wl.map.Field(8,13):region(3), {quarry = 2}) do
         coroutine.yield(wl.game.get_time() + 5000)
   end
   p.objectives.quarries.done = true
end

function check_ranger() 
   while not check_for_buildings(wl.map.Field(17,11):region(3), {rangers_hut = 1}) do
         coroutine.yield(wl.game.get_time() + 5000)
   end
   p.objectives.ranger.done = true
end

function tutorial_thread()
   show_story_box(_"Somebody comes upon to you", khantrukh_1)
   show_story_box(_"The advisor", khantrukh_2, nil, 80, 80)
   o = start_lumberjack_01(p)
   -- Wait till the hut is build.
   while not check_for_buildings(
      wl.map.Field(15,11):region(2), {constructionsite = 1})
      do sleep(5000) end

   show_story_box(_"The advisor", khantrukh_3, nil, 80, 80)

   -- Wait till the hut is build.
   while not check_for_buildings(
      wl.map.Field(15,11):region(2), {lumberjacks_hut = 1}) do
      coroutine.yield(wl.game.get_time() + 5000)
   end
   o.done = true

   p:message_box(_"The advisor", khantrukh_4)
   p:message_box(_"The advisor", khantrukh_5)
   o = start_lumberjack_02(p)

   -- Wait till the hut is build.
   while not check_for_buildings(
      wl.map.Field(12,13):region(2), {lumberjacks_hut = 1}) do
      coroutine.yield(wl.game.get_time() + 5000)
   end
   o.done = true

   p:message_box(_"The advisor", khantrukh_6)
   p:allow_buildings{"rangers_hut", "quarry"}
   start_quarries(p)
   start_ranger(p)
   wl.game.run_coroutine(coroutine.create(check_ranger))
   wl.game.run_coroutine(coroutine.create(check_quarries))

   p:message_box(_"The advisor", khantrukh_7)
   while not (p.objectives.ranger.done and p.objectives.quarries.done) do
      coroutine.yield(wl.game.get_time() + 5000)
   end

   p:reveal_scenario("barbariantut01")
   p:message_box(_"Mission Complete", khantrukh_8)
end

k = coroutine.create(tutorial_thread)

wl.game.run_coroutine(k)

