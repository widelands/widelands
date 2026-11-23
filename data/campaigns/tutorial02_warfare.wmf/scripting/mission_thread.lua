-- ================
-- Mission thread
-- ================

function intro()
   reveal_concentric(plr, sf, 15)
   sleep(1000)
   campaign_message_box(introduction)

   training()
end

local hq = plr:get_buildings("barbarians_headquarters")[1]
hq:set_warehouse_policies("barbarians_soldier", "remove")

local trainingcamp_done = false
local battlearena_done = false
local scouting_done = false

local arena = nil
local tcamp = nil
local trainingsites = {}

function training2()
   -- Teach about trainingsites and soldiers' abilities - concurrent part 2
   sleep(60*1000)
   local o = campaign_message_with_objective(trainingcamp1, obj_trainingcamp)
   tcamp = wait_build_one(plr, "barbarians_trainingcamp")
   set_objective_done(o)
   campaign_message_box(trainingcamp2)
   trainingcamp_done = true
end

function scouting()
   -- Teach player about scouting
   sleep(2*60*1000)
   local o = campaign_message_with_objective(scouting1, obj_scouting1)
   while #plr:get_buildings("barbarians_scouts_hut") == 0 do sleep(500) end
   set_objective_done(o)

   exploring()
end

function exploring()
   local pois = sf:region(30, 29)
   local o = campaign_message_with_objective(scouting2, obj_scouting2)
   while not any_field_seen(plr, pois) do sleep(2000) end
   set_objective_done(o)
   campaign_message_box(scouting3)
   scouting_done = true
end

function training()
   -- Teach about trainingsites and soldiers' abilities
   sleep(5000) -- to let soldiers walk

   campaign_message_box(abilities)
   local o = campaign_message_with_objective(battlearena1, obj_battlearena)
   run(training2)
   run(scouting)

   arena = wait_build_one(plr, "barbarians_battlearena")
   set_objective_done(o, 0)
   campaign_message_box(battlearena2)
   battlearena_done = true
end

function military_buildings()
   while not scouting_done do sleep(3000) end

   sleep(5000)

   campaign_message_box(heroes_rookies)
   campaign_message_box(soldier_capacity)

   local o = campaign_message_with_objective(preference_demo, obj_preference)

   while heroes_in_sentries(plr) < 1 do sleep(1000) end
   set_objective_done(o)

   sleep(20000)  -- let the player experiment

   campaign_message_box(hq_soldiers)

   hq:set_warehouse_policies("barbarians_soldier", "normal")

   o = campaign_message_with_objective(hq_status, obj_hq_garrison)

   while hq:get_soldiers("present") < 5 do sleep(500) end
   set_objective_done(o)

   sleep(5000)

   o = campaign_message_with_objective(dismantle, obj_dismantle)

   while #plr:get_buildings("barbarians_sentry") > 1 do sleep(200) end
   set_objective_done(o)

   while not (trainingcamp_done and battlearena_done) do
      sleep(3000)
   end

   sleep(10000)

   -- The citadel will need lots of wares on the roads, slowing down the supply of
   -- the training sites, which may not even have been filled up yet. But we need
   -- full trainingsites for the demonstration of the training preferences, so
   -- we cheat a little.
   trainingsites = { arena, tcamp }
   fill_inputs(trainingsites)

   local o = campaign_message_with_objective(fortress_enhancement, obj_fortress)

   -- wait until the player has actually started the upgrade
   while not (citadel_field.immovable and
      citadel_field.immovable.descr.name == "constructionsite" and
      citadel_field.immovable.building == "barbarians_citadel") do sleep(800) end

   -- TODO(tothxa): this causes a segfault in the building window code, and the
   --               window is most likely still open at this point
   -- citadel_field.immovable.destruction_blocked = true

   sleep(5000)

   -- then we use the waiting time for some demonstration
   run(training_preference)

   while not (citadel_field.immovable and
      citadel_field.immovable.descr.name == "barbarians_citadel") do sleep(800) end
   set_objective_done(o, 0)

   create_enemy()

   -- Wait for soldiers to move in
   local citadel = citadel_field.immovable
   local break_out = false
   while not break_out do
      for k,v in pairs(citadel:get_soldiers("all")) do
         break_out = true
         break -- Break out if there is at least one soldier here
      end

      sleep(500)
   end

   sleep(300)

   attack()
end

local preference = nil
local timer_first_run = true

function check_and_remind_training_watch()
   -- unfortunately there's no way to set a delay for the first run of a timer
   if timer_first_run then
      timer_first_run = false
      return
   end

   local remind = { needed = false }
   if not check_training_windows(trainingsites) then
      remind.windows = true
      remind.needed = true
   end
   if not check_training_prefs(trainingsites, preference) then
      remind.preferences = preference
      remind.needed = true
   end
   if not check_training_caps(trainingsites) then
      remind.capacities = true
      remind.needed = true
   end

   if remind.needed then
      local mv = wl.ui.MapView()
      -- give the player some time to read the message
      mv:get_plugin_timer("check_training_watch").active = false

      campaign_message_box(remind_training(remind))

      mv:get_plugin_timer("check_training_watch").next_run = ticks() + 20 * 1000
      mv:get_plugin_timer("check_training_watch").active = true
   end
end

function wait_watch(pref, duration)
   preference = pref
   timer_first_run = true

   -- nag the player every 20 seconds (realtime) if things are not done as requested
   mv:add_plugin_timer("check_and_remind_training_watch()", 20 * 1000, "check_training_watch")

   -- make sure the player obeyed
   while not check_training_windows(trainingsites) do sleep(1000) end

   sleep(duration)

   -- remove rather than disable, because set_objective_done() saves the game, so this way
   -- the timer will be recreated in reloaded games too
   mv:remove_plugin_timer("check_training_watch")
end

function training_preference()
   -- we have about 10 minutes until the citadel is finished, but we need more time for the
   -- demo, so we prevent it from being finished before we're done
   citadel_field.immovable:set_desired_fill("granite", 3)
   local watch_duration = 8 * 60 * 1000

   campaign_message_box(training_preference_intro)

   local o_watch = campaign_message_with_objective(watch_training, obj_watch_training)

   local o = campaign_message_with_objective(train_heroes, obj_train_heroes)
   wait_watch("heroes", watch_duration)
   set_objective_done(o)

   o = campaign_message_with_objective(train_rookies, obj_train_rookies)
   wait_watch("rookies", watch_duration)
   set_objective_done(o)

   o = campaign_message_with_objective(train_any, obj_train_any)
   wait_watch("any", watch_duration)
   set_objective_done(o)

   set_objective_done(o_watch)

   -- let the builder finish the citadel
   local construction = citadel_field.immovable
   if construction and construction.descr.name == "constructionsite" then
      construction:set_desired_fill("granite", 4)
   end

   campaign_message_box(end_watching)
end

function create_enemy()
   local map = wl.Game().map
   local p2 = wl.Game().players[2]
   prefilled_buildings(p2,
      {"empire_barrier", 24, 7},
      {"empire_sentry", 29, 16},
      {"empire_tower", 30, 21},
      {"empire_headquarters", 30, 27,
         soldiers = {
            [{0,0,0,0}] = 15,
         }
      }
   )
   connected_road("normal", p2,map:get_field(29,17).immovable,"tr,tl|tl,tl|tl,tl|tl,tl|tl,l")
   connected_road("normal", p2,map:get_field(31,22).immovable,"tr,tl|tl,tl,tl")
   connected_road("normal", p2,map:get_field(31,28).immovable,"tr,tr|tr,tl|tl,tl")
   p2:forbid_buildings("all")
end

function attack()
   local o = campaign_message_with_objective(attack_enemy, obj_attack)

   local plr2 = wl.Game().players[2]
   while #plr2:get_buildings("empire_headquarters") > 0 do
      sleep(3000)
   end
   set_objective_done(o)

   conclusion()
end

function conclusion()
   campaign_message_box(conclude_tutorial)
end

run(intro)
run(military_buildings)
