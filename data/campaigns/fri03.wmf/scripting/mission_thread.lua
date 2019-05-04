include "scripting/messages.lua"
include "scripting/field_animations.lua"

farm_connect_done = false

function steady_supply(player, wares)
   while not player.defeated do
      sleep(60000)
      local whs = array_combine(
         player:get_buildings("HQ_atterdag"),
         player:get_buildings("HQ_ravenstrupp"),
         player:get_buildings(player.tribe_name .. "_headquarters"),
         player:get_buildings(player.tribe_name .. "_warehouse"))
      -- If the player is low on a ware, he'll certainly get it replaced;
      -- otherwise, there's a random chance that he'll get it anyway.
      for ware,n in pairs(wares) do
         local wh = whs[math.random(#whs)]
         if wh:get_wares(ware) < n then
            wh:set_wares(ware, n)
         end
      end
   end
end

function farms()
   campaign_message_box(farms_1)
   local o = add_campaign_objective(obj_connect_farms)
   local nr_farms = #p1:get_buildings("frisians_farm")
   local hq = p1:get_buildings("frisians_headquarters")[1].flag
   sleep(300000)
   while true do
      sleep(90000)
      local ok = true
      for i,farm in pairs(array_combine(
         p1:get_buildings("frisians_farm"),
         p1:get_buildings("frisians_farm_new")
      )) do
         if not hq:get_distance(farm.flag) then
            ok = false
            p1:send_message(unconnected_farm.title, unconnected_farm.body, {
               heading = unconnected_farm.heading,
               field = farm.fields[1],
               icon = farm.descr.representative_image,
            })
            break
         end
         sleep(5)
      end
      if ok then
         set_objective_done(o)
         farm_connect_done = true
         p1:allow_buildings("all")
         p1:forbid_buildings {
            "frisians_hunters_house",
            "frisians_quarry",
            "frisians_foresters_house",
            "frisians_woodcutters_house",
            "frisians_farm",
            "frisians_fortress",
            "frisians_warehouse",
            "frisians_port",
            "frisians_shipyard",
         }
         return
      end
   end
end

function save_atterdag()
   -- Waldemar Atterdag must not be defeated, therefore rescue him when it’s going badly for him
   local hq = p2:get_buildings("empire_headquarters")[1]
   while true do
      sleep(5000)
      local danger = 0
      for i,f in pairs(map.player_slots[2].starting_field:region(21)) do
         if f.immovable and f.immovable.descr.type_name == "militarysite" and (f.owner == p1 or f.owner == p3) then
            danger = danger + f.immovable.descr.max_number_of_soldiers
         end
      end
      if danger > 0 then
         -- Fill his milsites with soldiers
         local ok = false
         for i,f in pairs(map.player_slots[2].starting_field:region(21)) do
            if f.immovable and f.immovable.descr.type_name == "militarysite" then
               if f.immovable:get_soldiers({4, 4, 0, 2}) < f.immovable.descr.max_number_of_soldiers then
                  f.immovable:set_soldiers({4, 4, 0, 2}, f.immovable.descr.max_number_of_soldiers)
                  ok = true
               end
            end
         end
         if not ok then
            -- No milsites to fill? Straight into the HQ then
            local soldiers = 0
            for descr,n in pairs(hq:get_soldiers("all")) do
               soldiers = soldiers + n
            end
            hq:set_soldiers({4, 4, 0, 2}, soldiers + danger)
         end
         sleep(90000)
      end
   end
end

function run_witch(witch)
   while witch do
      for i,f in pairs(witch.field:region(1)) do
         if f.owner.team == 2 and f.immovable and f.immovable.descr.type_name == "productionsite" then
            f.immovable:destroy()
            break
         end
      end
      sleep(math.random(900, 24000))
   end
end

function mission_thread()

   p1.team = 1
   p2.team = 2
   p3.team = 1
   p4.team = 2

   for i,f in pairs(p3_start) do
      map:place_immovable("debris00", f, "world")
   end

   local land = {}
   for x=0, map.width - 1 do
      for y=0, map.height - 1 do
         local f = map:get_field(x, y)
         if f.terr ~= "winter_water" or f.terd ~= "winter_water" then
            table.insert(land, f)
            table.insert(land, f.brn)
            table.insert(land, f.bln)
            table.insert(land, f.trn)
            table.insert(land, f.tln)
            table.insert(land, f.rn)
            table.insert(land, f.ln)
         end
      end
   end
   p1:reveal_fields(land)
   p1:conquer(map.player_slots [1].starting_field, map.width)
   include "map:scripting/starting_conditions.lua"

   run(steady_supply, p1, {
      log = 40,
      granite = 20,
      iron_ore = 20,
      gold_ore = 10,
   })
   run(steady_supply, p2, {
      log = 60,
      granite = 30,
      marble = 80,
      iron_ore = 30,
      gold_ore = 20,
   })
   run(steady_supply, p4, {
      log = 70,
      granite = 40,
      iron_ore = 30,
      gold_ore = 20,
   })
   scroll_to_field(map.player_slots[1].starting_field)


-- NOCOM
p1.see_all = true
sleep(9999999)

   -- Introduction
   sleep(3000)
   campaign_message_box(intro_1)
   sleep(3000)
   campaign_message_box(intro_2)
   scroll_to_field(map.player_slots[2].starting_field)
   sleep(3000)
   campaign_message_box(intro_3)
   scroll_to_field(map.player_slots[4].starting_field)
   sleep(3000)
   campaign_message_box(intro_4)
   scroll_to_field(map.player_slots[1].starting_field)
   sleep(3000)
   campaign_message_box(intro_5)
   local o = add_campaign_objective(obj_wait_for_reinforcements)

   sleep(5000)
   campaign_message_box(intro_6)

   sleep(5000)
   run(save_atterdag)
   run(farms)

   -- Give the player some extra work to keep him busy
   while not farm_connect_done do sleep(1000) end
   -- Now we wait until the enemy is nearly at the Castle
   local fields = map.player_slots[1].starting_field:region(21, 16)
   while true do
      sleep(10000)
      local enemy = false
      for i,f in pairs(fields) do
         if f.owner and f.owner.team == 2 then
            enemy = true
            break
         end
      end
      if #p1:get_buildings("frisians_headquarters") == 0 then
         scroll_to_field(map.player_slots[1].starting_field)
         sleep(2000)
         campaign_message_box(defeated_1)
         wl.ui.MapView():close()
         return
      end
      if enemy then break end
   end

   for i,f in pairs(p3_start) do
      f.immovable:remove()
   end
   p3:place_ship(map:get_field(344, 208))
   scroll_to_field(map.player_slots[3].starting_field)
   sleep(3000)
   campaign_message_box(help_arrives_1)

   local port = p3:place_building("frisians_port", map.player_slots[3].starting_field, false, true)
   port:set_wares {
      log = 40,
      granite = 50,
      thatch_reed = 50,
      brick = 80,
      clay = 30,
      water = 10,
      fish = 10,
      meat = 10,
      fruit = 10,
      barley = 5,
      ration = 20,
      honey = 10,
      smoked_meat = 5,
      smoked_fish = 5,
      mead = 5,
      meal = 2,
      coal = 20,
      iron = 5,
      gold = 4,
      iron_ore = 10,
      bread_frisians = 15,
      honey_bread = 5,
      beer = 5,
      cloth = 5,
      fur = 10,
      fur_garment = 5,
      sword_short = 5,
      hammer = 5,
      fire_tongs = 2,
      bread_paddle = 2,
      kitchen_tools = 2,
      felling_ax = 3,
      needles = 2,
      basket = 2,
      pick = 5,
      shovel = 5,
      scythe = 3,
      hunting_spear = 2,
      fishing_net = 3,
   }
   port:set_workers {
      frisians_blacksmith = 3,
      frisians_baker = 1,
      frisians_brewer = 1,
      frisians_builder = 10,
      frisians_charcoal_burner = 1,
      frisians_claydigger = 2,
      frisians_brickmaker = 2,
      frisians_carrier = 40,
      frisians_reed_farmer = 2,
      frisians_berry_farmer = 2,
      frisians_fruit_collector = 2,
      frisians_farmer = 1,
      frisians_landlady = 1,
      frisians_smoker = 1,
      frisians_geologist = 4,
      frisians_woodcutter = 3,
      frisians_beekeeper = 1,
      frisians_miner = 4,
      frisians_miner_master = 2,
      frisians_forester = 2,
      frisians_stonemason = 2,
      frisians_reindeer = 5,
      frisians_trainer = 3,
   }
   port:set_soldiers {
      [{0,0,0,0}] = 40,
      [{2,3,2,0}] = 10,
   }
   p3:allow_buildings("all")
   p3:forbid_buildings {
      "frisians_hunters_house",
      "frisians_quarry",
      "frisians_foresters_house",
      "frisians_woodcutters_house",
      "frisians_fortress",
      "frisians_port",
      "frisians_shipyard",
   }
   run(steady_supply, p3, {
      log = 30,
      granite = 20,
      iron_ore = 10,
      gold_ore = 5,
   })
   set_objective_done(o)

   sleep(1000)
   campaign_message_box(help_arrives_2)
   p1:switchplayer(3)
   campaign_message_box(help_arrives_3)
   o = add_campaign_objective(obj_rescue)

   -- Wait until the enemy is pushed well back
   fields = map.player_slots[1].starting_field:region(60)
   while true do
      local enemy = false
      for i,f in pairs(fields) do
         if f.owner and f.owner.team == 2 then
            enemy = true
            break
         end
         sleep(10)
      end
      if p3.defeated or #p1:get_buildings("frisians_headquarters") == 0 then
         scroll_to_field(map.player_slots[1].starting_field)
         sleep(2000)
         campaign_message_box(defeated_1)
         wl.ui.MapView():close()
         return
      end
      if enemy then break end
   end
   set_objective_done(o)

   local critters = {
      "bunny",
      "sheep",
      "wisent",
      "wildboar",
      "chamois",
      "deer",
      "reindeer",
      "stag",
      "elk",
      "marten",
      "badger",
      "lynx",
      "fox",
      "wolf",
      "brownbear"
   }
   local witch = {}
   for i=1,64 do
      local f = fields[math.random(#fields)]
      table.insert(witch, map:place_critter(f, critters[math.random(#critters)]))
   end
   witch = witch[math.random(#witch)]

   -- Now we have placed an evil witch in disguise on the map.
   -- She'll walk around and cause buildings to spontaneously burst into flames.

   local destroyed = 0
   local created_flames = {}
   while true do
      local did_destroy = false

      local fires_removed = {}
      for i,f in pairs(created_flames) do
         if (not f.immovable) or f.immovable.descr.name ~= "destroyed_building" then
            table.insert(fires_removed, i)
         end
      end
      for i,index in pairs(fires_removed) do
         table.remove(index)
      end

      for i,f in pairs(witch.field:region(1)) do
         if f.owner.team == 1 and f.immovable and f.immovable.descr.type_name == "productionsite" then
            f.immovable:destroy()
            table.insert(created_flames, f)
            did_destroy = true
            if destroyed ~= nil then
               destroyed = destroyed + 1
               scroll_to_field(f)
               if destroyed > 3 then
                  destroyed = nil
                  campaign_message_box(witchhunt_1)
                  campaign_message_box(witchhunt_2)
                  campaign_message_box(witchhunt_3)
                  o = add_campaign_objective(obj_witchhunt)
               end
            end
            break
         end
      end

      if did_destroy then
         -- Give the witch some time to get away from the flames
         sleep(3600)
      else
         -- Check if the player killed or conjured the witch
         local next_to_fire = false
         for i,f in pairs(witch.field:region(1)) do
            if f.tln.immovable and f.tln.immovable.descr.name == "destroyed_building" then
               -- Check if this flame was created by the witch or not
               local w = false
               for j,field in pairs(f:region(1)) do
                  for k,flame in pairs(created_flames) do
                     if field == flame then
                        w = true
                        break
                     end
                  end
                  if w then break end
               end
               if w then
                  -- At least one adjacent fire was created by the witch, so she's safe for now
                  next_to_fire = false
                  break
               else
                  next_to_fire = true
               end
            end
         end

         if next_to_fire then
            -- The witch is killed
            scroll_to_field(witch.field)
            witch:destroy()
            witch = nil
            sleep(1000)
            campaign_message_box(witchhunt_kill)
            set_objective_done(o)
            break
         -- else
            -- check if she's conjured

            -- TODO(NOCOM)(Nordfriese): Conjure the witch

         end
      end

      sleep(math.random(600, 18000))
   end
   if witch then run(run_witch, witch) end

   campaign_message_box(next_attack_1)
   o = add_campaign_objective(obj_defeat_ravenstrupp)
   while not p4.defeated do sleep(4321) end
   set_objective_done(o)

   sleep(5000)
   -- TODO(NOCOM)(Nordfriese): place ships for Atterdag and give him lots and lots of soldiers
   scroll_to_field(map.player_slots[2].starting_field)
   campaign_message_box(next_attack_2)
   sleep(1000)
   campaign_message_box(next_attack_3)
   campaign_message_box(next_attack_4)
   o = add_campaign_objective(obj_flee)

   p3:allow_buildings{
      "frisians_port",
      "frisians_shipyard",
   }

   -- Wait until an expedition ship is ready
   local expedition = nil
   while not expedition do
      if p3.defeated or #p1:get_buildings("frisians_headquarters") == 0 then
         scroll_to_field(map.player_slots[1].starting_field)
         sleep(2000)
         campaign_message_box(defeated_1)
         wl.ui.MapView():close()
         return
      end
      for i,port in pairs(p3:get_buildings("frisians_port")) do
         if port.expedition_in_progress then
            expedition = port
            break
         end
      end
      sleep(1149)
   end

   -- We escaped!
   scroll_to_field(expedition.fields[1])
   local persist = {}
   for descr,n in pairs(expedition:get_soldiers("all")) do
      persist[descr[1] .. descr[2] .. descr[3]] = n
   end
   -- We save a table of all soldiers we can take with us.
   -- The syntax is the same as in fri01.
   game:save_campaign_data("frisians", "fri03", persist)
   sleep(1000)
   campaign_message_box(victory_1)
   p3:reveal_scenario("frisians03")
   -- END OF MISSION 3

end

run(mission_thread)
