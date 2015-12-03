dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_coalmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Coal Mine"),
   directory = dirname,
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
      prohibited_till = 1200
   },

   working_positions = {
      atlanteans_miner = 3
   },

   inputs = {
      atlanteans_bread = 10,
      smoked_fish = 10,
      smoked_meat = 6
   },
   outputs = {
      "coal"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining coal because ...
         descname = _"mining coal",
         actions = {
            "sleep=45000",
            "return=skipped unless economy needs coal",
            "consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
            "animate=working 20000",
            "mine=coal 4 100 5 2",
            "produce=coal:2",
            "animate=working 20000",
            "mine=coal 4 100 5 2",
            "produce=coal:2",
            "animate=working 20000",
            "mine=coal 4 100 5 2",
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
