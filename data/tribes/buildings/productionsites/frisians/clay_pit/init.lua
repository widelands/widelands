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
      reed = 1
   },
   return_on_dismantle = {
      brick = 1
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {40, 68},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         directory = dirname,
         basename = "working",
         hotspot = {40, 68},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {40, 52}
      }
   },

   indicate_workarea_overlaps = {
      frisians_aqua_farm = true,
      frisians_charcoal_burners_house = true,
      frisians_clay_pit = false,
      frisians_berry_farm = false,
      frisians_reed_farm = false,
      frisians_farm = false,
      frisians_foresters_house = false,
   },

   aihints = {
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
      basic_amount = 1,
      supports_production_of = { "fish", "coal", }
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

            "return=skipped unless economy needs clay or economy needs fish or economy needs coal",
            -- Fish-producing aqua farms and charcoal burners houses can stop working if the clay pits do so
            "return=failed unless site has water",
            "callworker=dig",
            "consume=water",
            "sleep=22000",
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
