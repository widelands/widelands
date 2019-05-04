-- This function simulates an entire standalone AI.
-- It is intended to be run for a player that is NOT the human player and set to the empty (!) AI.
-- This AI is designed specifically for the special conditions of this scenario,
-- and only for the empire and barbarian tribe.
-- If you wish to use it for a different purpose, you will need to enhance it significantly.

-- So how does it work?
-- Our first and foremost priority is fighting. If we don't have enough militarysites somewhere, we need to build more.
-- If we are stronger than the enemy somewhere, we attack!
-- If the military looks good, we take care of our economy.
-- If there are shortages of some ware which we can produce, we need to build more buildings for it.

local ai_milsite_border_score_factor = 20
local ai_milsite_border_score_factor_alt = 7
local ai_trainsite_border_score_factor = 40
local ai_trainsite_border_score_offset = 100

function ai(pl)
   if pl.tribe.name == "empire" then
      local eco = pl:get_buildings("empire_headquarters")[1].flag.economy
      eco:set_ware_target_quantity("planks", 25)
      eco:set_ware_target_quantity("marble_column", 5)
      eco:set_ware_target_quantity("coal", 15)
      eco:set_ware_target_quantity("iron", 10)
      eco:set_ware_target_quantity("gold", 5)
      eco:set_worker_target_quantity("empire_soldier", 20)
      eco:set_worker_target_quantity("empire_donkey", 10)
   elseif pl.tribe.name == "barbarians" then
      local eco = pl:get_buildings("barbarians_headquarters")[1].flag.economy
      eco:set_ware_target_quantity("blackwood", 25)
      eco:set_ware_target_quantity("grout", 15)
      eco:set_ware_target_quantity("coal", 15)
      eco:set_ware_target_quantity("iron", 10)
      eco:set_ware_target_quantity("gold", 5)
      eco:set_ware_target_quantity("beer", 0)
      eco:set_worker_target_quantity("barbarians_soldier", 20)
      eco:set_worker_target_quantity("barbarians_ox", 10)
   else
      print("FATAL EXCEPTION: The custom AI for fri03 does not support the tribe '" .. pl.tribe.name .. "'!")
      return
   end
   while not pl.defeated do
      ai_one_loop(pl)
      sleep(ai_speed_1)
   end
end

function ai_one_loop(pl)

   -- We begin by cleaning up our road network. Long roads are broken into shorter sections, dead-ends are removed.
   if ai_flags[pl.number] then
      for i,field in pairs(ai_flags[pl.number]) do
         if field.owner == pl and field.immovable and field.immovable.descr.type_name == "flag" then
            local nroads = 0
            for dir,road in pairs(field.immovable.roads) do
               nroads = nroads + 1
            end
            if nroads < 1 and field.immovable.building then
               if connect(pl, field.immovable, 3) then return end
            elseif nroads < 2 and not field.immovable.building then
               field.immovable:destroy()
               table.remove(ai_flags[pl.number], i)
               return
            else
               for dir,road in pairs(field.immovable.roads) do
                  for j,f in ipairs(road.fields) do
                     local len = 0
                     if f:has_caps("flag") then
                        pl:place_flag(f)
                        table.insert(ai_flags[pl.number], f)
                        return
                     else
                        len = len + 1
                        if len > 3 then
                           road:destroy()
                           return
                        end
                     end
                  end
                  if road.length > 3 then
                     road:destroy()
                     return
                  end
               end
            end
         else
            table.remove(ai_flags[pl.number], i)
         end
      end
   else
      ai_flags[pl.number] = { array_combine(pl:get_buildings("barbarians_headquarters"),
            pl:get_buildings("empire_headquarters"))[1].flag.fields[1] }
   end

   -- Military stuff
   -- ==============
   -- We frequently check for enemy militarysites near our border.
   -- If we see one which can be attacked, we compare strengths:
   --   If we are stronger than the enemy, we attack!
   --   Otherwise, we construct a new militarysite nearby.
   --     (If a new own milsite is already under construction nearby, we ignore that enemy site – for now...)

   sleep(ai_speed_2)

   for i,b in pairs(array_combine(
         p1:get_buildings("frisians_headquarters"),
         p1:get_buildings("frisians_port"),
         p3:get_buildings("frisians_port")
   )) do
      local n = #pl:get_attack_soldiers(b)
      if n > 0 then
         pl:attack(b, n)
         return
      end
   end

   sleep(ai_speed_2)

   for i,b in pairs(array_combine(
         p1:get_buildings("frisians_tower"),
         p1:get_buildings("frisians_outpost"),
         p1:get_buildings("frisians_sentinel"),
         p1:get_buildings("frisians_wooden_tower"),
         p1:get_buildings("frisians_wooden_tower_high"),
         p3:get_buildings("frisians_tower"),
         p3:get_buildings("frisians_outpost"),
         p3:get_buildings("frisians_sentinel"),
         p3:get_buildings("frisians_wooden_tower"),
         p3:get_buildings("frisians_wooden_tower_high")
   )) do
      sleep(ai_speed_2)
      local attack_soldiers = pl:get_attack_soldiers(b)
      if #attack_soldiers > 0 then
         local attackers_score = 0
         local defenders_score = 0
         local attackers = 0
         local milsite_under_construction = false
         for j,f in pairs(b.fields[1]:region(25)) do
            sleep(ai_speed_2)
            if f.owner and f.owner.team ~= pl.team and f.immovable and f.immovable.descr.type_name == "militarysite" then
               for descr,n in pairs(f.immovable.get_soldiers("all")) do
                  -- This approximation is incorrect for several reasons, but they balance each other out fairly well ;)
                  defenders_score = defenders_score + n *
                        ((descr[1] + 1) * (descr[2] + 1) * (descr[3] + 1) * (descr[4] + 1))
               end
            elseif f.owner == pl and f.immovable and f.immovable.descr.type_name == "constructionsite" and
                  game:get_building_description(f.immovable.building).type_name == "militarysite" then
               milsite_under_construction = true
            end
         end
         for j,s in pairs(attack_soldiers) do
            sleep(ai_speed_2)
            attackers_score = attackers_score + (s.attack_level + 1) * (s.evade_level + 1) *
                  (s.health_level + 1) * (s.defense_level + 1)
            attackers = attackers + 1
            if attackers_score > defenders_score then
               break
            end
         end
         if attackers_score < defenders_score then
            if not milsite_under_construction then
               local buildings = nil
               if pl.tribe.name == "empire" then
                  buildings = {
                     game:get_building_description("empire_sentry"),
                     game:get_building_description("empire_outpost"),
                     game:get_building_description("empire_barrier"),
                     game:get_building_description("empire_tower"),
                     game:get_building_description("empire_fortress"),
                  }
               elseif pl.tribe.name == "barbarians" then
                  buildings = {
                     game:get_building_description("barbarians_sentry"),
                     game:get_building_description("barbarians_barrier"),
                     game:get_building_description("barbarians_tower"),
                     game:get_building_description("barbarians_fortress"),
                  }
               end
               if build_best_building(pl, buildings, b.fields[1]:region(21)) then
                  return
               end
            end
         else
            pl:attack(b, attackers)
            return
         end
      end
   end

   sleep(ai_speed_2)

   -- Now, let's take care of our economy. We define our very own "basic economy" below.
   -- If we haven't built everything from there yet, we really need to take care of that.

   local basic_economy
   if pl.tribe.name == "empire" then basic_economy = ai_basic_economy_emp else basic_economy = ai_basic_economy_bar end
   local most_important_missing_build = nil
   local most_important_missing_enhance = nil
   for b,tbl in pairs(basic_economy) do
      sleep(ai_speed_2)
      local buildable = game:get_building_description(b).buildable
      local count = count_buildings(pl, b) < tbl.amount
      if buildable and ((not most_important_missing_build) or
            most_important_missing_build < tbl.importance) and count then
         most_important_missing_build = tbl.importance
      end
      if (not buildable) and ((not most_important_missing_enhance) or
            most_important_missing_enhance < tbl.importance) and count then
         most_important_missing_enhance = tbl.importance
      end
   end
   if most_important_missing_build ~= nil then
      -- There is at least 1 basic building that can be built directly
      local bld = {}
      for b,tbl in pairs(basic_economy) do
         sleep(ai_speed_2)
         local descr = game:get_building_description(b)
         if descr.buildable and tbl.importance >= most_important_missing_build and
               count_buildings(pl, b) < tbl.amount then
            table.insert(bld, descr)
         end
      end
      -- We choose a region around a random building somewhere
      local field = array_combine(
         pl:get_buildings("empire_headquarters"),
         pl:get_buildings("barbarians_headquarters"),
         pl:get_buildings("empire_warehouse"),
         pl:get_buildings("barbarians_warehouse"))
      local size = #field
      field = field[math.random(size)].fields[1]
      if build_best_building(pl, bld, field:region(20 * size)) then
         return
      end
   end
   if most_important_missing_enhance ~= nil then
      for b,tbl in pairs(basic_economy) do
         sleep(ai_speed_2)
         local descr = game:get_building_description(b)
         if not descr.buildable and tbl.importance >= most_important_missing_build and
               count_buildings(pl, b) < tbl.amount then
            local candidates = pl:get_buildings(descr.enhanced_from.name)
            if #candidates > 0 then
               -- If this building needs an experienced worker, we need to check if we have the required
               -- worker either in a warehouse or in this building
               local missing_worker = nil
               if descr.working_positions then
                  for i,worker in pairs(descr.working_positions) do
                     if worker.name == "barbarians_blacksmith_master" or worker.name == "barbarians_brewer_master" then
                        missing_worker = worker.name
                        break
                     end
                  end
                  sleep(ai_speed_2)
                  if missing_worker then
                     for i,wh in pairs(array_combine(
                           pl:get_buildings("barbarians_headquarters"),
                           pl:get_buildings("barbarians_warehouse"))) do
                        if wh:get_workers(missing_worker) > 0 then
                           missing_worker = nil
                           break
                        end
                     end
                  end
                  sleep(ai_speed_2)
                  local choice = nil
                  if missing_worker then
                     for i,bld in pairs(candidates) do
                        if bld:get_workers(missing_worker) > 0 then
                           choice = bld
                           break
                        end
                     end
                  else
                     choice = candidates[math.random(#candidates)]
                  end
                  if choice then choice:enhance() end
                  return
               end
            end
         end
      end
   end

   -- Let's check whether there are two flags that are physically close but far apart in the road network
   print("NOCOM (" .. pl.number .. ") Starting to consider connecting roads")
   local connect_candidates = {}
   for i = 1, #ai_flags[pl.number] do
      local f1 = ai_flags[pl.number][i]
      for j = i + 1, #ai_flags[pl.number] do
         local f2 = ai_flags[pl.number][j]
         local d_short = distance(f1, f2, 8, ai_speed_2)
         local d_real = f1.immovable:get_distance(f2.immovable) / 1800
         if d_short and d_real >= d_short * 3 then
            table.insert(connect_candidates, {f1 = f1, f2 = f2})
         end
      end
      sleep(ai_speed_2)
   end
   print("NOCOM (" .. pl.number .. ") Found " .. #connect_candidates .. " pairs of candidates")
   while #connect_candidates > 0 do
      local i = math.random(#connect_candidates)
      local f1 = connect_candidates[i].f1
      local f2 = connect_candidates[i].f2
      if pl:connect_with_road(f1.immovable, f2.immovable) then
         print("NOCOM (" .. pl.number .. ") built a new road.")
         return
      end
      table.remove(connect_candidates, i)
   end
   print("NOCOM (" .. pl.number .. ") Did NOT build a new road!")

   -- TODO: Do other stuff – micromanage workers and wares, find out if we should build more
   -- productionsites, warehouses, milsites...

   -- We check our borders now. Any field where we can build something and the border is close
   -- is interesting, even more so if immovables nearby are owned by an enemy.
   -- Fields located closely to an enemy warehouse or milsite are also more interesting.
   print("NOCOM (" .. pl.number .. ") Starting to consider building a milsite")
   local interesting_fields = {}
   local best_scored = nil
   local found = 0
   for i,milsite in pairs(array_combine(
      pl:get_buildings("empire_headquarters"),
      pl:get_buildings("empire_sentry"),
      pl:get_buildings("empire_barrier"),
      pl:get_buildings("empire_tower"),
      pl:get_buildings("empire_outpost"),
      pl:get_buildings("empire_fortress"),
      pl:get_buildings("barbarians_headquarters"),
      pl:get_buildings("barbarians_sentry"),
      pl:get_buildings("barbarians_tower"),
      pl:get_buildings("barbarians_barrier"),
      pl:get_buildings("barbarians_fortress"),
      pl:get_buildings("frisians_outpost"),
      pl:get_buildings("frisians_sentinel"),
      pl:get_buildings("frisians_tower"),
      pl:get_buildings("frisians_fortress"),
      pl:get_buildings("frisians_wooden_tower"),
      pl:get_buildings("frisians_wooden_tower_high")
   )) do
      for id,f in pairs(milsite.fields[1]:region(milsite.descr.conquers), 3) do
         if suitability(pl, f, game:get_building_description("barbarians_sentry")) then
            local border_distance = nil
            local d = 1
            while not border_distance and d < ai_milsite_border_score_factor_alt do
               for j,fld in pairs(f:region(d, d - 1)) do
                  if fld.owner and fld.owner.team ~= pl.team then
                     border_distance = d
                     break
                  end
               end
               d = d + 1
            end
            if border_distance then
               score = ai_milsite_border_score_factor_alt - border_distance
               score = score * score * score
               for j,field in pairs(f:region(ai_milsite_border_score_factor_alt)) do
                  if field.immovable then
                     local obj = field.immovable.descr.type_name
                     if field.owner.team ~= pl.team then
                            if obj == "warehouse"      then score = score * 8
                        elseif obj == "militarysite"   then score = score * 6
                        elseif obj == "trainingsite"   then score = score * 4
                        elseif obj == "productionsite" then score = score * 3
                        else                                score = score * 2
                        end
                     elseif field.owner == pl and (obj == "militarysite" or (obj == "constructionsite" and
                           game:get_building_description(field.immovable.building).type_name == "militarysite")) then
                        score = score / 5
                     end
                  end
               end
               interesting_fields[f] = score
               found = found + 1
               print("NOCOM (" .. pl.number .. ") Found field " .. f.x .. "|" .. f.y .. ", scored it " .. score)
               if not best_scored or best_scored < score then
                  best_scored = score
               end
            end
         end
      end
   end
   if found > 0 then
      print("NOCOM (" .. pl.number .. ") Found " .. found .. " fields, best score is ".. best_scored)
      local best_field = {}
      for field,score in pairs(interesting_fields) do
         if score >= best_scored then
            for i,f in pairs(field:region(1)) do
               table.insert(best_field, f)
            end
         end
      end
      print("NOCOM (" .. pl.number .. ") There are " .. #best_field .. " fields with those values")
      local buildings
      if pl.tribe.name == "empire" then
         buildings = {
            game:get_building_description("empire_sentry"),
            game:get_building_description("empire_outpost"),
            game:get_building_description("empire_barrier"),
            game:get_building_description("empire_tower"),
            game:get_building_description("empire_fortress"),
         }
      elseif pl.tribe.name == "barbarians" then
         buildings = {
            game:get_building_description("barbarians_sentry"),
            game:get_building_description("barbarians_barrier"),
            game:get_building_description("barbarians_tower"),
            game:get_building_description("barbarians_fortress"),
         }
      end
      if build_best_building(pl, buildings, best_field) then
      print("NOCOM (" .. pl.number .. ") Built one!")
         return
      end
   else
   print("NOCOM (" .. pl.number .. ") Found no fields!")
   end

   local stock = stock(pl)
   local economy
   if #pl:get_buildings("empire_headquarters") > 0 then
      economy = pl:get_buildings("empire_headquarters")[1]
   elseif #pl:get_buildings("barbarians_headquarters") > 0 then
      economy = pl:get_buildings("barbarians_headquarters")[1]
   elseif #pl:get_buildings("empire_warehouse") > 0 then
      economy = pl:get_buildings("empire_warehouse")[1]
   elseif #pl:get_buildings("barbarians_warehouse") > 0 then
      economy = pl:get_buildings("barbarians_warehouse")[1]
   else
      print("ERROR: " .. pl.name .. " does not seem to have a warehouse!")
      return
   end
   economy = economy.flag.economy
   for ware,x in pairs(ai_ware_preciousness) do
      if pl.tribe:has_ware(ware) then
         local missing = economy:ware_target_quantity(ware) - stock[ware]
         if missing > 0 then
            print("NOCOM (" .. pl.number .. ") Needs " .. missing .. " of ware " .. ware)
            local under_construction = 0
            for i,building in pairs(pl.tribe.buildings) do
               if building.output_ware_types then
                  for j,w in pairs(building.output_ware_types) do
                     if w.name == ware then
                        under_construction = under_construction +
                              count_buildings(pl, building.name) - #pl:get_buildings(building.name)
                        break
                     end
                  end
               end
            end
            print("NOCOM (" .. pl.number .. ") Has " .. under_construction .. " buildings under construction for it")
            if under_construction < 8 * missing then
               local buildings = {}
               for i,building in pairs(pl.tribe.buildings) do
                  if building.output_ware_types then
                     for j,w in pairs(building.output_ware_types) do
                        if w.name == ware then
                           table.insert(buildings, building)
                           break
                        end
                     end
                  end
               end
               if #buildings == 0 then
                  print("ERROR: Tribe " .. pl.tribe.name .. " has no building that produces the ware " .. ware .. "!")
                  return
               end
               local fields = {}
               for i,wh in pairs(array_combine(
                     pl:get_buildings("empire_headquarters"),
                     pl:get_buildings("barbarians_headquarters"),
                     pl:get_buildings("empire_warehouse"),
                     pl:get_buildings("barbarians_warehouse"))) do
                  fields = array_combine(fields, wh.fields[1]:region(9, 2))
               end
               print("NOCOM (" .. pl.number .. ") Considering " .. #fields .. " fields for it")
               if build_best_building(pl, buildings, fields) then return end
               print("NOCOM (" .. pl.number .. ") Built none :(")
            end
         end
      end
   end








   print("NOCOM AI #" .. pl.number .. " is bored :(")
   sleep(ai_speed_1)

end

-- Our own basic economy. Each building has an amount and an importance (higher importance = built earlier)
ai_basic_economy_bar = {
   barbarians_fishers_hut    = { amount = 2, importance = 3 },
   barbarians_well           = { amount = 1, importance = 3 },
   barbarians_reed_yard      = { amount = 1, importance = 8 },
   barbarians_wood_hardener  = { amount = 2, importance = 9 },
   barbarians_lime_kiln      = { amount = 1, importance = 8 },
   barbarians_charcoal_kiln  = { amount = 3, importance = 7 },
   barbarians_bakery         = { amount = 1, importance = 4 },
   barbarians_brewery        = { amount = 1, importance = 2 },
   barbarians_micro_brewery  = { amount = 1, importance = 3 },
   barbarians_smelting_works = { amount = 2, importance = 6 },
   barbarians_metal_workshop = { amount = 2, importance = 5 },
   barbarians_ax_workshop    = { amount = 1, importance = 2 },
   barbarians_warmill        = { amount = 1, importance = 2 },
   barbarians_farm           = { amount = 2, importance = 4 },
   barbarians_cattlefarm     = { amount = 1, importance = 1 },
   barbarians_helmsmithy     = { amount = 1, importance = 2 },
   barbarians_barracks       = { amount = 1, importance = 2 },
   barbarians_battlearena    = { amount = 1, importance = 1 },
   barbarians_trainingcamp   = { amount = 1, importance = 1 },
   barbarians_barrier        = { amount = 1, importance = 5 },
}
ai_basic_economy_emp = {
   empire_fishers_house     = { amount = 2, importance = 3 },
   empire_well              = { amount = 1, importance = 3 },
   empire_stonemasons_house = { amount = 1, importance = 8 },
   empire_sawmill           = { amount = 2, importance = 9 },
   empire_mill              = { amount = 1, importance = 4 },
   empire_bakery            = { amount = 1, importance = 4 },
   empire_charcoal_kiln     = { amount = 3, importance = 7 },
   empire_smelting_works    = { amount = 2, importance = 6 },
   empire_toolsmithy        = { amount = 1, importance = 4 },
   empire_armorsmithy       = { amount = 1, importance = 3 },
   empire_donkeyfarm        = { amount = 1, importance = 1 },
   empire_farm              = { amount = 2, importance = 4 },
   empire_sheepfarm         = { amount = 1, importance = 3 },
   empire_weaving_mill      = { amount = 1, importance = 3 },
   empire_weaponsmithy      = { amount = 1, importance = 2 },
   empire_barracks          = { amount = 1, importance = 2 },
   empire_trainingcamp      = { amount = 1, importance = 1 },
   empire_arena             = { amount = 1, importance = 1 },
   empire_colosseum         = { amount = 1, importance = 1 },
   empire_outpost           = { amount = 1, importance = 7 },
   empire_sentry            = { amount = 1, importance = 4 },
}

-- The preciousness for all wares. The higher the preciousness, the more reluctant we are to spend the ware
ai_ware_preciousness = {
   log = 1,
   granite = 1,
   blackwood = 3,
   grout = 3,
   thatch_reed = 1,
   cloth = 1,
   planks = 2,
   marble = 2,
   marble_column = 4,

   fish = 1,
   meat = 2,
   water = 1,
   wheat = 2,
   flour = 1,
   empire_bread = 2,
   barbarians_bread = 3,
   beer = 1,
   beer_strong = 1,

   coal = 4,
   iron_ore = 3,
   iron = 4,
   gold_ore = 5,
   gold = 6,

   pick = 1,
   felling_ax = 1,
   saw = 1,
   shovel = 1,
   hammer = 1,
   fishing_rod = 1,
   hunting_spear = 1,
   scythe = 1,
   bread_paddle = 1,
   basket = 1,
   kitchen_tools = 1,
   fire_tongs = 1,

   spear_wooden = 1,
   spear = 2,
   spear_advanced = 2,
   spear_heavy = 1,
   spear_war = 1,
   armor_helmet = 1,
   armor = 2,
   armor_chain = 1,
   armor_gilded = 1,
   ax = 1,
   ax_sharp = 3,
   ax_broad = 2,
   ax_bronze = 2,
   ax_battle = 1,
   ax_warriors = 1,
   helmet = 2,
   helmet_mask = 2,
   helmet_warhelm = 1
}

constructionsites = {}
ai_flags = {}

function count_buildings(pl, building)
   local n = #pl:get_buildings(building)
   if constructionsites[building] then
      for i,f in pairs(constructionsites[building]) do
         if f.immovable and f.immovable.descr.type_name == "constructionsite" and
               f.immovable.descr.name == "constructionsite" and f.immovable.building == building then
            n = n + 1
         else
            table.remove(constructionsites[building], i);
         end
      end
   end
   return n
end

-- Returns a table of ware names mapped to the amount of that ware in stock
function stock(pl)
   local tbl = {}
   for i,ware in pairs(pl.tribe.wares) do
      tbl[ware.name] = 0
      for j,wh in pairs(array_combine(
         pl:get_buildings("empire_headquarters"),
         pl:get_buildings("empire_warehouse"),
         pl:get_buildings("barbarians_headquarters"),
         pl:get_buildings("barbarians_warehouse"))) do
         tbl[ware.name] = tbl[ware.name] + wh:get_wares(ware.name)
      end
   end
   return tbl
end

function suitability(pl, field, building_descr)
   if not field:has_caps(building_descr.size) then return false end
   for i,f in pairs(array_combine(field.brn:region(1), field:region(building_descr.size == "big" and 2 or 1))) do
      if f.owner ~= pl then return false end
   end
   return true
end

-- Selects the best building of the given array of BuildingDescriptions
-- and builds it on the best field in the specified region
function build_best_building(pl, buildings, region, sleeptime)
   local stock = stock(pl)
   -- First, the best building is determined by comparing construction costs with our stock.
   -- We consider the most expensive building which we can afford to be the best one.
   -- If we can't afford any of them, we choose the cheapest
   local best = nil
   local cheapest = nil
   local best_score
   local cheapest_score

   -- If we are offered mainly small fields, we preferably build a small building...
   local size_bias = 0
   for i,f in pairs(region) do
      if f:has_caps("big") then
         size_bias = size_bias - 1
      elseif f:has_caps("small") and not f:has_caps("medium") then
         size_bias = size_bias + 1
      end
   end
   size_bias = size_bias / #region
   -- Now we have values between +1 (only small fields) and -1 (only big fields)

   for i,b in pairs(buildings) do
      local score = 0
      local affordable = true
      for ware,amount in pairs(b.buildcost) do
         score = score + ai_ware_preciousness[ware]
         if stock[ware] < amount then
            affordable = false
         end
      end
      score = score * (12 * math.exp((b.size == "small" and 1 or b.size == "big" and -1 or 0) - size_bias) * math.log(12))

      if (not cheapest) or score < cheapest_score then
         cheapest = b
         cheapest_score = score
      end
      if affordable and ((not best) or score > best_score) then
         best = b
         best_score = score
      end
   end
   if not best then best = cheapest end
   if not best then return nil end

   -- We will build the building 'best'. Now we must decide where.
   -- We map each field to a score. Only fields where we can actually build the building are considered.
   -- All scoring values are arbitrary. They are designed to produce fairly good results.
   local field = nil
   local field_score = nil
   for i,f in pairs(region) do
      if suitability(pl, f, best) then
         local score

         local border_distance = nil
         local d = 1
         while not border_distance do
            for j,fld in pairs(f:region(d, d - 1)) do
               if fld.owner and fld.owner.team ~= pl.team then
                  border_distance = d
                  break
               end
            end
            d = d + 1
         end

         if best.type_name == "militarysite" then
            score = (ai_milsite_border_score_factor - border_distance)
            score = 72 * score * score * score
         elseif best.type_name == "warehouse" then
            score = 72 * border_distance * border_distance
         elseif best.type_name == "trainingsite" then
            score = 72 * (ai_trainsite_border_score_offset - (border_distance - ai_trainsite_border_score_factor) *
                  (border_distance - ai_trainsite_border_score_factor) * ai_trainsite_border_score_offset /
                  (ai_trainsite_border_score_factor * ai_trainsite_border_score_factor))
         elseif best.name:find("fisher") or best.name:find("well") then
            score = 0
            local what
            if best.name:find("fisher") then what = "fish" else what = "water" end
            for i,fld in pairs(f:region(6)) do
               if fld.resource == what then
                  score = score + fld.resource_amount
               end
            end
         elseif best.workarea_radius then
            score = border_distance
            for d = 1, best.workarea_radius do
               local sc = 84 - 12 * d
               for j,fld in pairs(f:region(d, d-1)) do
                  if fld.immovable then
                     score = score - sc
                  else
                     score = score + sc
                  end
               end
            end
         else
            score = 12 * border_distance
         end

         -- Placing a small building on a big plot is BAD. Placing a small building on a small plot is GOOD.
         if f:has_caps("big") then
            if best.size == "small" then
               score = score / 6
            elseif best.size == "medium" then
               score = score / 3
            end
         elseif f:has_caps("medium") and best.size == "small" then
            score = score / 2
         end

         for j,fld in pairs(f:region(best.workarea_radius and best.workarea_radius + 4 or 4, 1)) do
            if fld.immovable and fld.immovable.descr.type_name == "productionsite" and
                  fld.immovable.descr.workarea_radius then
               score = score / 2
            end
         end

         if score > 0 and ((not field) or field_score < score) then
            field = f
            field_score = score
         end
      end
   end
   if not field then return nil end

   local is_flag = field.brn
   is_flag = is_flag.immovable and is_flag.immovable.descr.type_name == "flag"
   local building = pl:place_building(best.name, field, true)
   if not constructionsites[best.name] then
      constructionsites[best.name] = {}
   end
   table.insert(constructionsites[best.name], building.fields[1])
   if not is_flag then
      sleep(ai_speed_1)
      if connect(pl, building.flag, 1) then
         table.insert(ai_flags[pl.number], building.flag.fields[1])
      else
         building:destroy()
      end
   end
   return building
end

-- Connects the given flag to a nearby road network
function connect(pl, flag, precision)
   local d = precision
   while d < map.width / 2 do
      local fields = flag.fields[1]:region(d, d - precision)
      while #fields > 0 do
         local fi = math.random(#fields)
         local f = fields[fi]
         table.remove(fields, fi)
         if f.immovable and (f.immovable.descr.type_name == "flag" or
               (f.immovable.descr.type_name == "road" and f:has_caps("flag"))) then
            if pl:connect_with_road(flag, f.immovable, 10) then
               return true
            end
         end
      end
      d = d + precision
   end
   return false
end

-- Helper function: Determine the distance between two fields
function distance(f1, f2, max, sleeptime)
   if f1.x == f2.x and f1.y == f2.y then return 0 end
   local d = 1
   while true do
      for i,f in pairs(f1:region(d, d - 1)) do
         if f.x == f2.x and f.y == f2.y then return d end
      end
      d = d + 1
      if max and d > max then
         return nil
      elseif sleeptime then
         sleep(sleeptime)
      end
   end
end
