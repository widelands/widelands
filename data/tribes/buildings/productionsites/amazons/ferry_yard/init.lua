push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_ferry_yard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("amazons_building", "Ferry Yard"),
   icon = dirname .. "menu.png",
   size = "medium",
   map_check = {"waterways"},

   buildcost = {
      balsa = 2,
      log = 2,
      rubber = 3,
      rope = 2,
   },
   return_on_dismantle = {
      balsa = 1,
      log = 1,
      rubber = 1,
      rope = 1,
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {43, 44}},
      unoccupied = {hotspot = {43, 44}}
   },

   aihints = {},

   working_positions = {
      amazons_shipwright = 1
   },

   inputs = {
      { name = "log", amount = 8 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- time of worker: 17.2-38.8 sec
            -- min. time total: 20 + 17.2 = 37.2 sec
            -- max. time total: 20 + 38.8 = 58.8 sec
            "sleep=duration:20s",
            "return=skipped unless fleet needs ferry",
            "consume=log:3",
            "callworker=buildferry"
         }
      },
   },
}

pop_textdomain()
