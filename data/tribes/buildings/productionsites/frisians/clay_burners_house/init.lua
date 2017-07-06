dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_clay_burners_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Clay Burner´s House"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 5,
      granite = 3,
      thatch_reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      granite = 2,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 87 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 57, 87 },
      },
   },

   aihints = {
      forced_after = 900
   },

   working_positions = {
      frisians_digger = 1,
      frisians_clay_burner = 1
   },

   inputs = {
      { name = "water", amount = 8 },
   },
   outputs = {
      "clay"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"making clay",
         actions = {
            "sleep=8000",
            "return=skipped unless economy needs clay",
            "consume=water",
            "animate=working 12000",
            "produce=clay"
         },
      },
   },
}
