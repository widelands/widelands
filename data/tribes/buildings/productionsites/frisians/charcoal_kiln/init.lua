dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_charcoal_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Charcoal Kiln"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 5,
      granite = 1,
      log = 1,
      thatch_reed = 2
   },
   return_on_dismantle = {
      brick = 3,
      granite = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 43, 63 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 43, 63 },
      },
   },

   aihints = {
      forced_after = 900
   },

   working_positions = {
      frisians_charcoal_burner = 1
   },

   inputs = {
      { name = "log", amount = 8 },
   },
   outputs = {
      "coal"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"burning charcoal",
         actions = {
            "sleep=37000",
            "return=skipped unless economy needs coal",
            "consume=log:5",
            "animate=working 20000",
            "produce=coal"
         },
      },
   },
}
