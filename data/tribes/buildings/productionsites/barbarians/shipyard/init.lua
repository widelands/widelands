push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "barbarians_shipyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Shipyard"),
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

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 62, 48 },
      },
      unoccupied = {
         hotspot = { 62, 48 },
      },
      working = {
         hotspot = { 61, 47 },
      },
   },

   spritesheets = {
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 61, 47 }
      },
   },

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

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- steps from building to ship: 2-9
            -- worker time: 5.5 sec
            -- number of wares to carry: 10 + 2 + 4 = 16
            -- (see data/tribes/immovables/shipconstruction_barbarians/init.lua)
            -- min. time total: 5 + 16 * (2 * 2 * 1.8 + 5.5 + 20 + 35) = 1088.2 sec
            -- max. time total: 5 + 16 * (2 * 9 * 1.8 + 5.5 + 20 + 35) = 1491.4 sec
            "call=ship on failure fail",
            "call=ship_preparation",
         }
      },
      ship = {
         -- TRANSLATORS: Completed/Skipped/Did not start constructing a ship because ...
         descname = _("constructing a ship"),
         actions = {
            "construct=barbarians_shipconstruction worker:buildship radius:6",
            "sleep=duration:20s",
         }
      },
      ship_preparation = {
         descname = _("working"),
         actions = {
            "animate=working duration:35s",
         }
      },
   },
}

pop_textdomain()
