-- test battle between players
-- especially if fighting is allowed when expected and ends when expected

local bld_defender
local bld_attacker

local INITIAL_SOLDIERS = 6

local function prepare_buildings()

   local p_atlanteans = game.players[3]
   local p_frisians = game.players[4]

   assert_equal("atlanteans", p_atlanteans.tribe.name)
   local field = map:get_field(62, 59)
   p_atlanteans:conquer(field, 3)
   -- do not force placing building, works with conquer and an appropriate place
   local bld_a = p_atlanteans:place_building("atlanteans_guardhall", field)
   bld_a:set_soldiers({0, 0, 0, 0}, INITIAL_SOLDIERS)
   bld_defender = bld_a

   assert_equal("frisians", p_frisians.tribe.name)
   field = map:get_field(1, 8)
   p_frisians:conquer(field, 3)
   local bld_f = p_frisians:place_building("frisians_outpost", field)
   bld_f:set_soldiers({0, 0, 0, 0}, INITIAL_SOLDIERS)
   bld_attacker = bld_f

   game.players[game.interactive_player]:switchplayer(p_frisians.number)
   -- set interactive player to attacker, to be able to open windows
end

local function click_building_for_attack(building)
   mapview:click(building.fields[1])
   return "attack_" .. building.serial
end

local function attack_building(building, additional_soldiers)
   local window_name = click_building_for_attack(building)

   local t = 0
   repeat
      sleep(game.real_speed // 10)  -- 1/10 in real world seconds
      t = t + 1
   until mapview.windows[window_name] or t > 20
   assert_not_nil(mapview.windows[window_name], "attack window did not open")
   if additional_soldiers then
      for _i = 1, additional_soldiers do
         mapview.windows[window_name].buttons.more:click()
         sleep(game.real_speed // 10)
      end
   end
   mapview.windows[window_name].buttons.attack:click()

   return window_name
end

local function start_attack(building, additional_soldiers)
   local window_name = attack_building(building, additional_soldiers)
   game.desired_speed = 4 * 1000
   local attacking_soldiers = (additional_soldiers or 0) + 1
   sleep(attacking_soldiers * 1000)
   assert_equal(INITIAL_SOLDIERS - attacking_soldiers, #bld_attacker.fields[1].bobs,
                "soldier(s) should have left for attacking")
   assert_equal(INITIAL_SOLDIERS, #building.fields[1].bobs,
                "all defending soldiers should be inside")
   assert_nil(mapview.windows[window_name], "attack window should be closed")
end

local function count_healty_soldiers(field)
   local max_health = -1
   local with_max_health = 0
   for _i, bob in pairs(field.bobs) do
      if bob.current_health > max_health then
         max_health = bob.current_health
         with_max_health = 1
      elseif bob.current_health == max_health then
         with_max_health = with_max_health + 1
      end
   end
   return with_max_health, max_health
end

local function check_soldiers_inside_and_healthy(building, total, healthy)
   assert_equal(total, #building.fields[1].bobs,
                building.owner.tribe.name .. " soldiers should be inside")
   local with_max_health, max_health = count_healty_soldiers(building.fields[1])
   if healthy < 0 then
      healthy = total + healthy  -- smaller than total, is negative
   end
   assert_equal(healthy, with_max_health,
                "healthy " .. building.owner.tribe.name .. " soldiers (h=" .. max_health)
end

local function test_attack_when_forbidden()
   game.desired_speed = 1000

   assert_equal(INITIAL_SOLDIERS, #bld_attacker.fields[1].bobs, "all soldiers at home")

   local window_name = click_building_for_attack(bld_defender)
   sleep(500)
   assert_nil(mapview.windows[window_name], "attack window should still be closed")
   assert_not_nil(mapview.windows.field_action, "other window opened instead")
   mapview.windows.field_action.buttons.b_close:click()  -- or ...s.field_action:close()
   sleep(500)
   assert_nil(mapview.windows.field_action, "other window closed")
   assert_equal(INITIAL_SOLDIERS, #bld_attacker.fields[1].bobs, "no soldier should have left")
end

--[[ nonworking:
local function reset_soldiers(bld)
   bld:set_soldiers({0, 0, 0, 0}, INITIAL_SOLDIERS)

   -- the soldiers fighting seem not to be influenced nor counted,
   --   some more soldiers are added to those in the house
end
]]


local test_case_battle = lunit.TestCase("Battle")

function test_case_battle.test_attack_start()
   game.desired_speed = 1000
   assert_equal(INITIAL_SOLDIERS, bld_defender:get_soldiers({0, 0, 0, 0}), "still all soldiers")
   assert_equal(INITIAL_SOLDIERS, #bld_defender.fields[1].bobs, "all soldiers should be inside")
   assert_equal(INITIAL_SOLDIERS, bld_attacker:get_soldiers({0, 0, 0, 0}), "still all soldiers")
   assert_equal(INITIAL_SOLDIERS, #bld_attacker.fields[1].bobs, "all soldiers should be inside")

   start_attack(bld_defender)

   game.desired_speed = 16 * 1000
   sleep(30 * 1000)
   assert_true(#bld_defender.fields[1].bobs < INITIAL_SOLDIERS,
               "some defending soldiers should be outside")
   game.desired_speed = 1 * 1000
   sleep(700)
end

function test_case_battle.test_battle_cancel_by_forbidden()
   game.desired_speed = 8 * 1000

   assert_equal(INITIAL_SOLDIERS, bld_defender:get_soldiers({0, 0, 0, 0}), "still all soldiers")
   assert_equal(INITIAL_SOLDIERS, bld_attacker:get_soldiers({0, 0, 0, 0}), "still all soldiers")

   -- only one forbidden
   local DEFENDER_RETURN_TIME = 5 * 1000
   bld_defender.owner:set_attack_forbidden(bld_attacker.owner.number, true)
   sleep(DEFENDER_RETURN_TIME)
   assert_true(#bld_defender.fields[1].bobs < INITIAL_SOLDIERS,
               "defenders should still be fighting")

   -- only the other forbidden
   bld_defender.owner:set_attack_forbidden(bld_attacker.owner.number, false)
   bld_attacker.owner:set_attack_forbidden(bld_defender.owner.number, true)
   sleep(DEFENDER_RETURN_TIME)
   assert_true(#bld_defender.fields[1].bobs < INITIAL_SOLDIERS,
               "defenders should still be fighting")

   -- both forbidden ends the fight (GH #5665)
   game.desired_speed = 1000
   bld_defender.owner:set_attack_forbidden(bld_attacker.owner.number, true)
   bld_attacker.owner:set_attack_forbidden(bld_defender.owner.number, true)
   sleep(500)

   -- check that battle ended
   -- and that fighting happened (there are wounded solders on both sides)
   game.desired_speed = 16 * 1000
   sleep(DEFENDER_RETURN_TIME)  -- soldiers go back, defenders arrive
   check_soldiers_inside_and_healthy(bld_defender, INITIAL_SOLDIERS, -1)  -- check defenders

   sleep(17 * 1000)  -- all soldiers arrive
   check_soldiers_inside_and_healthy(bld_attacker, INITIAL_SOLDIERS, -1) -- check attackers
end

function test_case_battle.test_attack_peace()
   assert_true(bld_defender.owner:is_attack_forbidden(bld_attacker.owner.number))
   assert_true(bld_attacker.owner:is_attack_forbidden(bld_defender.owner.number))

   test_attack_when_forbidden()
end

function test_case_battle.test_attack_onesided()
   -- defender could attack, but not the attacker
   bld_defender.owner:set_attack_forbidden(bld_attacker.owner.number, false)
   assert_true(bld_attacker.owner:is_attack_forbidden(bld_defender.owner.number))

   test_attack_when_forbidden()
end

function test_case_battle.test_attack_plain_defenders()
   -- defenders are allwed to defend even if not allowed to attack
   -- and fighting does not get blocked (fixed in pr CB #4574 / GH #6233)
   game.desired_speed = 1000

   bld_defender.owner:set_attack_forbidden(bld_attacker.owner.number, true)
   bld_attacker.owner:set_attack_forbidden(bld_defender.owner.number, false)

   start_attack(bld_defender, 1)

   game.desired_speed = 16 * 1000
   sleep(30 * 1000)
   assert_true(#bld_defender.fields[1].bobs < INITIAL_SOLDIERS,
               "some defending soldiers should be outside")
   game.desired_speed = 1000
   sleep(700)
end

function test_case_battle.test_battle_cancel_by_team()
   game.desired_speed = 1000

   assert_equal(INITIAL_SOLDIERS, bld_defender:get_soldiers({0, 0, 0, 0}), "still all soldiers")
   assert_equal(INITIAL_SOLDIERS, bld_attacker:get_soldiers({0, 0, 0, 0}), "still all soldiers")

   bld_defender.owner.team = 9
   bld_attacker.owner.team = 9
   sleep(500)
   game.desired_speed = 16 * 1000

   -- check that battle ended,
   -- and that it happened (there are wounded solders on both sides)
   local DEFENDER_RETURN_TIME = 5 * 1000
   sleep(DEFENDER_RETURN_TIME)  -- soldiers go back, defenders arrive
   local unhealthy = count_healty_soldiers(bld_defender.fields[1]) - INITIAL_SOLDIERS
   -- cases to handle:
   -- * only same soldier fought (against one)      --> one is hurt
   -- * same and other soldier fought (against two) --> two are hurt
   -- * one other soldiers fought (agains one)      --> two are hurt
   -- * two other soldiers fought (against two)     --> three are hurt
   if unhealthy ~= -1 and  -- often the soldier who fought before fights again
      unhealty ~= -2 and  -- one other (or one other plus same) did some fighting
      unhealty ~= -3  -- two other did fight
   then
      unhealthy = -1  -- take one same fighter as default
   end
   check_soldiers_inside_and_healthy(bld_defender, INITIAL_SOLDIERS, unhealthy)  -- check defenders

   sleep(21 * 1000)  -- all soldiers arrive, takes longer here
   local unh_attack = count_healty_soldiers(bld_attacker.fields[1]) - INITIAL_SOLDIERS
   -- cases to handle:
   -- * as above, and matching to defenders
   -- => max 1 difference to hurt defenders, but in same range
   if unh_attack ~= unhealthy and  -- matches defenders
      unh_attack - unhealthy ~= -1 and unh_attack - unhealthy ~= 1 -- one difference,
      -- ... (probably) the unhealthy soldier acted differently
      or unh_attack < -3 or unh_attack > -1  -- within range
   then
      unh_attack = unhealthy  -- unmatching, default to same as defenders
   end
   check_soldiers_inside_and_healthy(bld_attacker, INITIAL_SOLDIERS, unh_attack)  -- check attackers
end

function test_case_battle.test_attack_team()
   assert_not_equal(0, bld_defender.owner.team, "in a team")
   assert_equal(bld_defender.owner.team, bld_attacker.owner.team, "in one team")

   test_attack_when_forbidden()
end

function test_case_battle.test_healing()
   for _i, building in pairs({bld_defender, bld_attacker}) do
      if count_healty_soldiers(building.fields[1]) == INITIAL_SOLDIERS then
         -- the number of healthy soldiers was lower in test_battle_cancel_by_*()
         assert_true(true)
      else
         -- check if healing happens now
         local function get_min_health()
            local min_health = math.maxinteger
            for _j, bob in pairs(building.fields[1].bobs) do
               if bob.current_health < min_health then
                  min_health = bob.current_health
               end
            end
            return min_health
         end

         local min_health_t1 = get_min_health()
         sleep(1500) -- rate is per second, give some tolerance
         local min_health_t2 = get_min_health()
         assert_true(min_health_t2 > min_health_t1,
                     min_health_t2 .. " > " .. min_health_t1 .. " for " .. tostring(building.owner))
      end
   end
end

function test_case_battle.test_attack_till_deads()
   game.desired_speed = 1000

   local function get_solders_of_player(plr)
      return plr:get_workers(plr.tribe.soldier)
   end

   -- check reference for final check is fine
   local surviving_soldiers = get_solders_of_player(bld_defender.owner) +
                              get_solders_of_player(bld_attacker.owner)
   assert_equal(2 * INITIAL_SOLDIERS, surviving_soldiers, "no soldiers have died yet")

   bld_defender.owner.team = 0

   start_attack(bld_defender, 2)

   game.desired_speed = 16 * 1000
   sleep(30 * 1000)
   -- attackers have arrived
   assert_true(#bld_defender.fields[1].bobs < INITIAL_SOLDIERS,
               "some defending soldiers should be outside")
   sleep(55 * 1000)  -- let them fight longer

   -- do not care about location of soldiers, neither about which player's
   surviving_soldiers = get_solders_of_player(bld_defender.owner) +
                        get_solders_of_player(bld_attacker.owner)
   assert_not_equal(2 * INITIAL_SOLDIERS, surviving_soldiers, "some soldiers should have died")
end

run(function()
   game.desired_speed = 1000 -- slow for dialogs
   sleep(800) -- to allow pause by user
   prepare_buildings()
   sleep(500)
   game.allow_saving = false  -- to be sure save does not happen while lunit.run() runs
   lunit.run()
   print("(quit)")
   wl.ui.MapView():close()
end)
