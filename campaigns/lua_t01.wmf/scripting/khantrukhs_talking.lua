-- ======================
-- Messages by Khantrukh 
-- ======================

-- TODO: gebabbel vom haeuptling
-- TODO: reveal campaign mission

use("map", "texts")

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

function check_quarries() 
   while not exist_buildings(wl.map.Field(8,13):region(3), {quarry = 2}) do
         coroutine.yield(wl.game.get_time() + 5000)
   end
   p.objectives.quarries.done = true
end

function check_ranger() 
   while not exist_buildings(wl.map.Field(17,11):region(3), {rangers_hut = 1}) do
         coroutine.yield(wl.game.get_time() + 5000)
   end
   p.objectives.ranger.done = true
end

function tutorial_thread()
send_msg(_"Somebody comes upon to you", khantrukh_1)
send_msg(_"The advisor", khantrukh_2, home)
o = start_lumberjack_01(p)
-- Wait till the hut is build.
while not exist_buildings(
   wl.map.Field(15,11):region(2), {constructionsite = 1})
   do sleep(5000) end

send_msg(_"The advisor", khantrukh_3, home)

-- Wait till the hut is build.
while not exist_buildings(
   wl.map.Field(15,11):region(2), {lumberjacks_hut = 1}) do
   coroutine.yield(wl.game.get_time() + 5000)
end
o.done = true

p:message_box(_"The advisor", khantrukh_4)
p:message_box(_"The advisor", khantrukh_5)
o = start_lumberjack_02(p)

-- Wait till the hut is build.
while not exist_buildings(
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

p:message_box(_"Mission Complete", khantrukh_8)

end

k = coroutine.create(tutorial_thread)

wl.game.run_coroutine(k)
