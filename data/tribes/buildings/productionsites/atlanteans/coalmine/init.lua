dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_coalmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Coal Mine"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "mine",

   buildcost = {
      log = 7,
      planks = 4,
      spidercloth = 1
   },
   return_on_dismantle = {
      log = 3,
      planks = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 56 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 50, 56 },
      },
      empty = {
         pictures = path.list_files(dirname .. "empty_??.png"),
         hotspot = { 50, 56 },
      },
   },

   aihints = {
      mines = "coal",
      prohibited_till = 910
   },

   working_positions = {
      atlanteans_miner = 3
   },

   inputs = {
      { name = "smoked_fish", amount = 10 },
      { name = "smoked_meat", amount = 6 },
      { name = "atlanteans_bread", amount = 10 }
   },
   outputs = {
      "coal"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining coal because ...
         descname = _"mining coal",
         actions = {
            "sleep=40000",
            "return=skipped unless economy needs coal",
            "consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
            "animate=working 60000",
            "mine=coal 4 100 35 2",
            -- this step has a 35 percent chance to mine 1 coal. 1 coal is 1/7th of the normal yield.
            -- this results in 5% chance with regard to normal yield (35 divided by 7)
            -- following cycles of mine will fail (chance of only 1 percent
            "produce=coal",
            "sleep=2500",
            "mine=coal 4 100 1 2", -- 3 mine commands to have one resource mined for each ware delivered
            "mine=coal 4 100 1 2",
            "mine=coal 4 100 1 2",
            -- if the last mine command fails due to the second command mined the last ressource max. 2 ressources are lost
            "produce=coal:3",
            "sleep=2500",    
            "mine=coal 4 100 1 2",
            "mine=coal 4 100 1 2",
            "mine=coal 4 100 1 2",
            "produce=coal:3"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Coal",
      heading = _"Main Coal Vein Exhausted",
      message =
         pgettext("atlanteans_building", "This coal mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider dismantling or destroying it."),
   },
}
