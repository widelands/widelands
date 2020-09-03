push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_charcoal_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Charcoal Kiln"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 3,
      grout = 2,
      reed = 2
   },
   return_on_dismantle = {
      log = 2,
      grout = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 71 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 50, 71 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 50, 71 },
      },
   },

   aihints = {
      basic_amount = 1
   },

   working_positions = {
      barbarians_charcoal_burner = 1
   },

   inputs = {
      { name = "log", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start producing coal because ...
         descname = _"producing coal",
         actions = {
            "return=skipped unless economy needs coal",
            "consume=log:6",
            "sleep=duration:30s",
            "animate=working duration:1m30s", -- Charcoal fires will burn for some days in real life
            "produce=coal"
         }
      },
   },
}

pop_textdomain()
