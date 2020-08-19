dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_weaving_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Weaving Mill"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",
   map_check = {"seafaring", "waterways"},

   buildcost = {
      log = 5,
      granite = 2,
      reed = 2
   },
   return_on_dismantle = {
      log = 2,
      granite = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 36, 74 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 36, 74 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 36, 74 },
      },
   },

   aihints = {
      prohibited_till = 990,
      supports_seafaring = true
   },

   working_positions = {
      barbarians_weaver = 1
   },

   inputs = {
      { name = "reed", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start weaving because ...
         descname = _"weaving",
         actions = {
            "sleep=duration:25s",
            "return=skipped unless economy needs cloth",
            "consume=reed",
            "playsound=sound/barbarians/weaver priority:90%",
            "animate=working duration:20s",
            "produce=cloth"
         }
      },
   },
}
