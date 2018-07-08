dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_clay_pit",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Clay Pit"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 1,
      log = 1,
      thatch_reed = 1
   },
   return_on_dismantle = {
      brick = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {40, 64},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {40, 64},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {40, 64},
      },
   },

   aihints = {
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
      basic_amount = 1,
      supports_production_of = { "fish" }
   },

   working_positions = {
      frisians_claydigger = 1
   },

   inputs = {
      { name = "water", amount = 4 },
   },
   outputs = {
      "clay"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start making clay because ...
         descname = _"making clay",
         actions = {
            "sleep=20000",
            "return=skipped unless economy needs clay or site has water",
            "return=failed unless site has water",
            "worker=dig",
            "consume=water",
            "sleep=2000",
            "animate=working 17000",
            "sleep=1000",
            "produce=clay"
         },
      },
   },
   out_of_resource_notification = {
      -- TRANSLATORS: Short for "No Level Ground" for clay pits
      title = _"No Ground",
      heading = _"No Level Ground",
      message = pgettext ("frisians_building", "The clay digger working at this clay pit can’t find any level ground in his work area. Consider dismantling this clay pit and rebuilding it somewhere else."),
      productivity_threshold = 33
   },
}
