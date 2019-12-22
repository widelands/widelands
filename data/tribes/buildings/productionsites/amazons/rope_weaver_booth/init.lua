dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "amazons_building",
   name = "amazons_rope_weaver_booth",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Rope Weaver Booth"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 5,
      granite = 2,
      rope = 1,
      rubber =1
   },
   return_on_dismantle = {
      log = 3,
      granite = 1,
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {56, 89},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {56, 89},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {56, 66},
      },
   },

   aihints = {
      basic_amount = 1,
   },

   working_positions = {
      amazons_liana_cutter = 1
   },

   inputs = {
      { name = "liana", amount = 8 },
   },
   outputs = {
      "rope",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start weaving cloth because ...
         descname = _"making a rope",
         actions = {
            "consume=liana:2",
            "sleep=32000",
            "animate=working 40000",
            "produce=rope"
         },
      },
   },
}
