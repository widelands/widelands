dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_sweetbread_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Honey Bread Bakery"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement_cost = {
      brick = 5,
      log = 1,
      thatch_reed = 1
   },
   return_on_dismantle_on_enhanced = {
      brick = 5,
      granite = 2,
      log = 2,
      thatch_reed = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 51, 84 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 51, 84 },
      },
   },

   aihints = {
      prohibited_till = 1200
   },

   working_positions = {
      frisians_baker = 1,
      frisians_baker_master = 1
   },

   inputs = {
      { name = "honey", amount = 8 },
      { name = "barley", amount = 6 },
      { name = "water", amount = 6 },
   },
   outputs = {
      "sweetbread"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking honey bread because ...
         descname = _"baking honey bread",
         actions = {
            "sleep=10000",
            "return=skipped unless economy needs sweetbread or workers need experience",
            "consume=barley:2 water:2 honey:3",
            "animate=working 2500",
            "produce=sweetbread",
            "animate=working 25000",
            "produce=sweetbread"
         }
      },
   },
}
