-- =======================================================================
--                      Atlanteans Tutorial Mission 02
-- =======================================================================

-- ===============
-- Initialization
-- ===============

-- Initializing p1 (the scenario player)
   p1:allow_buildings("all")
   p1:forbid_buildings{"atlanteans_scouts_house", "atlanteans_temple_of_satul", "atlanteans_market"}
   p1:allow_workers("all")

   if #campaign_data.shipnames > 2 then
      local lost_ship = math.random(3)
      if lost_ship == 1 then
         campaign_data.shipnames[1] = campaign_data.shipnames[3]
      else
         if lost_ship == 2 then
            campaign_data.shipnames[2] = campaign_data.shipnames[3]
         end
      end
   end

   local sf = map.player_slots[1].starting_field
   local ship = p1:place_ship(map:get_field(41,0))
   if campaign_data.shipnames[1] then
      ship.shipname = campaign_data.shipnames[1]
   end
   ship.capacity = 110
   ship:make_expedition({
      atlanteans_fishbreeder = 1,
      atlanteans_geologist = 1,
      atlanteans_horse = 1,
      atlanteans_miner = 3,
      atlanteans_sawyer = 2,
      atlanteans_smelter = 1,
      atlanteans_stonecutter = 1,
      atlanteans_toolsmith = 1,
      atlanteans_woodcutter = 5,
      atlanteans_soldier = 8,
      atlanteans_forester =1,
      coal = 9,
      spider_silk = 4,
      smoked_fish = 10,
      smoked_meat = 10,
      atlanteans_bread = 20,
      iron_ore = 11,
      atlanteans_builder = 3,
      })

   ship = p1:place_ship(map:get_field(44,0))
   if campaign_data.shipnames[2] then
      ship.shipname = campaign_data.shipnames[2]
   end
   ship.capacity = 110
   ship:make_expedition({
      iron_ore = 29,
      spidercloth = 2,
      granite = 4,
      log = 33,
      quartz = 2,
      diamond = 1,
      planks = 20,
      atlanteans_builder = 1,
      })

-- Initialize Maletus (Player 2)
   Maletus:allow_buildings("all")
   Maletus:forbid_buildings("empire_market")

   sf = map.player_slots[2].starting_field
   Maletus:place_flag(sf.brn,true)
   create_economy_for_plr_2()

-- Initialize Kalitath (Player 3)
   Kalitath:allow_buildings("all")
   Kalitath:forbid_buildings("barbarians_market")

   sf = map.player_slots[3].starting_field
   Kalitath:place_flag(sf.brn,true)
   create_economy_for_plr_3()
   map:place_immovable("barbarians_trade_pole", map:get_field(112,150))
