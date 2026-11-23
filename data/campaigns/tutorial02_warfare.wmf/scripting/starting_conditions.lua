-- ===============
-- Initialization
-- ===============

function init_player()

   plr:allow_buildings("all")
   plr:forbid_buildings{"barbarians_market"}

   local buildings = prefilled_buildings(plr,
      {"barbarians_headquarters", f_hq.x, f_hq.y},
      {"barbarians_fortress", citadel_field.x, citadel_field.y, soldiers = {[{3,5,0,2}] = 8 }},
      {"barbarians_warehouse", 33, 57,
         soldiers = {
            [{0,0,0,0}] = 5, -- needed for initial replacements, 15 more will be recruited for training
            [{1,0,0,0}] = 2,
            [{2,0,0,0}] = 2,
            [{3,0,0,0}] = 1,
            [{0,1,0,0}] = 2,
            [{0,2,0,0}] = 2,
            [{0,3,0,0}] = 2,
            [{0,4,0,0}] = 1,
            [{0,5,0,0}] = 1,
            [{0,0,0,1}] = 3,
            [{0,0,0,2}] = 3,
            [{1,0,0,2}] = 1,
            [{2,0,0,2}] = 1,
            [{3,0,0,2}] = 1,
            [{0,1,0,2}] = 1,
            [{0,2,0,2}] = 1,
            [{0,3,0,2}] = 1,
            [{0,4,0,2}] = 1,
            [{0,5,0,2}] = 1,
            [{3,5,0,2}] = 30,
         },
         workers = {
            barbarians_builder = 3,
            barbarians_ox = 15,
            barbarians_trainer = 2,
            barbarians_scout = 1
         },
         wares = {
            log = 40,
            blackwood = 40,
            cloth = 10,
            gold = 10,
            grout = 30,
            granite = 30,
            reed = 40,
            ration = 20,

            -- wares for training
            ax = 10,  -- another 5 is added to the barracks directly
            ax_sharp = 30,
            ax_broad = 30,
            ax_bronze = 30,
            ax_battle = 30,
            ax_warriors = 30,
            helmet = 30,
            helmet_mask = 30,
            helmet_warhelm = 30,
            beer_strong = 80,
            barbarians_bread = 500,
            meat = 500
         }
      },
      {"barbarians_sentry", f_sentry_to_dismantle.x, f_sentry_to_dismantle.y, soldiers = {[{3,5,0,2}] = 2 }},
      {"barbarians_sentry", f_sentry_to_reinforce.x, f_sentry_to_reinforce.y, soldiers = {[{3,5,0,2}] = 2 }},
      {"barbarians_barrier", 30, 58, soldiers = {[{0,0,0,0}] = 1 }}, -- to make sure some soldiers walk out
      {"barbarians_barracks", 32, 58, inputs = {ax = 5}},  -- only 3 more axes have to be delivered at the start
      {"barbarians_lumberjacks_hut", 24, 1}
   )

   -- Build the roads
   local map = wl.Game().map
   connected_road("normal", plr,map:get_field(29,58).immovable,"br,r|r,r|r,tr")
   connected_road("normal", plr,map:get_field(38,62).immovable,"l,l|l,bl|tl,tl|tl,tl")
   connected_road("normal", plr,map:get_field(32,63).immovable,"tr,tr|")
   connected_road("normal", plr,map:get_field(25,2).immovable,"tr,tr|tr,tr|tr,tr|tr,tr")
   connected_road("normal", plr,map:get_field(30,56).immovable,"bl,bl")

   buildings[2].destruction_blocked = true
   buildings[3].flag.worker_economy:set_target_quantity("barbarians_soldier", 200)  -- infinite is not implemented
end

run(init_player)
