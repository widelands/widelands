dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_ironmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Iron Mine"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "mine",
   enhancement = "empire_ironmine_deep",

   buildcost = {
      log = 4,
      planks = 2
   },
   return_on_dismantle = {
      log = 2,
      planks = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 49, 49 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 49, 49 },
         fps = 10
      },
      empty = {
         pictures = path.list_files(dirname .. "empty_??.png"),
         hotspot = { 49, 49 },
      },
   },

   aihints = {
      mines = "iron",
      mines_percent = 50,
      prohibited_till = 900
   },

   working_positions = {
      empire_miner = 1
   },

   inputs = {
      ration = 6,
      beer = 6
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
            "consume=ration beer",
            "animate=working 20000",
            "mine=iron 2 50 5 17",
            "produce=iron_ore",
            "animate=working 20000",
            "mine=iron 2 50 5 17",
            "produce=iron_ore:2"
         }
      },
   },
   out_of_resource_notification = {
      title = _"Main Iron Vein Exhausted",
      message =
         pgettext("empire_building", "This iron mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}
