-- ===============
-- Initialization
-- ===============

function init_player()

   plr:allow_buildings("all")

   prefilled_buildings(plr,
      {"fortress", 32, 62, soldiers = {[{3,5,0,2}] = 8 }},
      {"warehouse", 33, 57,
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
            builder = 3,
            ox = 15,
            trainer = 2
         },
         wares = {
            log = 40,
            blackwood = 40,
            cloth = 10,
            gold = 10,
            grout = 30,
            raw_stone = 30,
            thatchreed = 40,

            -- wares for training
            sharpax = 20,
            broadax = 20,
            bronzeax = 20,
            battleax = 20,
            warriorsax = 20,
            helm = 20,
            mask = 20,
            warhelm = 20,
            strongbeer = 50,
            pittabread = 200,
            meat = 200
         }
      },
      {"sentry", 28, 57, soldiers = {[{3,5,0,2}] = 2 }},
      {"sentry", 37, 61, soldiers = {[{3,5,0,2}] = 2 }},
      {"barrier", 30, 58, soldiers = {[{0,0,0,0}] = 1 }}, -- to make sure some soldiers walk out
      {"lumberjacks_hut", 24, 1}
   )

   -- Build the roads
   local map = wl.Game().map
   connected_road(plr,map:get_field(29,58).immovable,"br,r|r,r|r,tr")
   connected_road(plr,map:get_field(38,62).immovable,"l,l|l,bl|tl,tl|tl,tl")
   connected_road(plr,map:get_field(32,63).immovable,"tr,tr|")
   connected_road(plr,map:get_field(25,2).immovable,"tr,tr|tr,tr|tr,tr|tr,tr")
end

run(init_player)

