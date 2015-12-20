dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_ironmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Iron Mine"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "mine",

   buildcost = {
      log = 7,
      planks = 4,
      spidercloth = 1
   },
   return_on_dismantle = {
      log = 2,
      planks = 2
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 50, 56 },
      },
      working = {
         template = "idle_??", -- TODO(GunChleoc): No animation yet.
         directory = dirname,
         hotspot = { 50, 56 },
      },
      empty = {
         template = "empty_??",
         directory = dirname,
         hotspot = { 50, 56 },
      },
   },

   aihints = {
      mines = "iron",
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
      "iron_ore"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining iron because ...
         descname = _"mining iron",
         actions = {
            "sleep=45000",
            "return=skipped unless economy needs iron_ore",
            "consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
            "animate=working 20000",
            "mine=iron 4 100 5 2",
            "produce=iron_ore",
            "animate=working 20000",
            "mine=iron 4 100 5 2",
            "produce=iron_ore:2",
            "animate=working 20000",
            "mine=iron 4 100 5 2",
            "produce=iron_ore:2"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Iron",
      heading = _"Main Iron Vein Exhausted",
      message =
         pgettext("atlanteans_building", "This iron mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider dismantling or destroying it."),
   },
}
