-- ===============
-- Initialization
-- ===============

function init_player()

   plr:allow_buildings("all")

   prefilled_buildings(plr,
      {"barbarians_fortress", 32, 62, soldiers = {[{3,5,0,2}] = 8 }},
      {"barbarians_headquarters", 29, 55},
      {"barbarians_warehouse", 33, 57,
         soldiers = {
            [{0,0,0,0}] = 20, -- needed for training
            [{1,0,0,0}] = 1,
            [{2,0,0,0}] = 1,
            [{3,0,0,0}] = 1,
            [{0,1,0,0}] = 1,
            [{0,2,0,0}] = 1,
            [{0,3,0,0}] = 1,
            [{0,4,0,0}] = 1,
            [{0,5,0,0}] = 1,
            [{0,0,0,1}] = 1,
            [{0,0,0,2}] = 1,
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
            ax_sharp = 20,
            ax_broad = 20,
            ax_bronze = 20,
            ax_battle = 20,
            ax_warriors = 20,
            helmet = 20,
            helmet_mask = 20,
            helmet_warhelm = 20,
            beer_strong = 50,
            barbarians_bread = 200,
            meat = 200
         }
      },
      {"barbarians_sentry", 28, 57, soldiers = {[{3,5,0,2}] = 2 }},
      {"barbarians_sentry", 37, 61, soldiers = {[{3,5,0,2}] = 2 }},
      {"barbarians_barrier", 30, 58, soldiers = {[{0,0,0,0}] = 1 }}, -- to make sure some soldiers walk out
      {"barbarians_barracks", 32, 58, inputs = {ax = 8}},
      {"barbarians_lumberjacks_hut", 24, 1}
   )

   -- Build the roads
   local map = wl.Game().map
   connected_road("normal", plr,map:get_field(29,58).immovable,"br,r|r,r|r,tr")
   connected_road("normal", plr,map:get_field(38,62).immovable,"l,l|l,bl|tl,tl|tl,tl")
   connected_road("normal", plr,map:get_field(32,63).immovable,"tr,tr|")
   connected_road("normal", plr,map:get_field(25,2).immovable,"tr,tr|tr,tr|tr,tr|tr,tr")
   connected_road("normal", plr,map:get_field(30,56).immovable,"bl,bl")
end

run(init_player)
