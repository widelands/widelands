dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_well",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Well"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 4
   },
   return_on_dismantle = {
      log = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 19, 33 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 19, 33 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 19, 33 },
      },
   },

   aihints = {
      mines_water = true,
      prohibited_till = 800,
      forced_after = 800
   },

   working_positions = {
      barbarians_carrier = 1
   },

   outputs = {
      "water"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "sleep=20000",
            "animate=working 20000",
            "mine=water 1 100 65 2",
            "produce=water",
         }
      },
   },

   out_of_resource_notification = {
      title = _"Out of Water",
      message = pgettext("barbarians_building", "The carrier working at this well can’t find any water in his work area."),
      productivity_threshold = 33
   },
}
