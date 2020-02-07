dirname = path.dirname(__file__)

animations = {}
add_animation(animations, "idle", dirname, "idle", { 62, 48 })
add_animation(animations, "build", dirname, "build", { 62, 48 })
add_animation(animations, "unoccupied", dirname, "unoccupied", { 62, 48 })
add_animation(animations, "working", dirname, "working", { 62, 48 })

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_shipyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Shipyard"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   map_check = {"seafaring"},

   buildcost = {
      log = 3,
      blackwood = 2,
      granite = 3,
      cloth = 2
   },
   return_on_dismantle = {
      log = 1,
      blackwood = 1,
      granite = 2
   },

   animations = animations,

   aihints = {
      needs_water = true,
      shipyard = true,
      prohibited_till = 1050
   },

   working_positions = {
      barbarians_shipwright = 1
   },

   inputs = {
      { name = "log", amount = 2 },
      { name = "blackwood", amount = 10 },
      { name = "cloth", amount = 4 }
   },

   indicate_workarea_overlaps = {
      barbarians_shipyard = false,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=ship on failure fail",
            "call=ship_preparation",
            "return=no_stats"
         }
      },
      ship = {
         -- TRANSLATORS: Completed/Skipped/Did not start constructing a ship because ...
         descname = _"constructing a ship",
         actions = {
            "checkmap=seafaring",
            "construct=barbarians_shipconstruction buildship 6",
            "sleep=20000",
         }
      },
      ship_preparation = {
         descname = _"working",
         actions = {
            "animate=working 35000",
         }
      },
   },
}
