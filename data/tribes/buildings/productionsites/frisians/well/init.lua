dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_well",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Well"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 3,
      granite = 1
   },
   return_on_dismantle = {
      log = 1,
      granite = 1
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
      frisians_carrier = 1
   },

   outputs = {
      "water"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining water because ...
         descname = _"mining water",
         actions = {
            "sleep=20000",
            "animate=working 20000",
            "mine=water 1 100 65 2",
            "produce=water",
         }
      },
   },

   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Water",
      heading = _"Out of Water",
      message = pgettext("frisians_building", "The carrier working at this well can’t find any water in his well."),
      productivity_threshold = 33
   },
}
