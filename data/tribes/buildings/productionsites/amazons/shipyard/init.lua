push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_shipyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Shipyard"),
   icon = dirname .. "menu.png",
   size = "medium",
   map_check = {"seafaring"},

   buildcost = {
      balsa = 2,
      log = 2,
      rubber = 3,
      rope = 2
   },
   return_on_dismantle = {
      balsa = 1,
      log = 1,
      rubber = 1,
      rope = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {43, 44}},
      unoccupied = {hotspot = {43, 44}}
   },

   aihints = {
      needs_water = true,
      shipyard = true,
      prohibited_till = 1050
   },

   working_positions = {
      amazons_shipwright = 1
   },

   inputs = {
      { name = "log", amount = 5 },
      { name = "balsa", amount = 5 },
      { name = "rubber", amount = 3 },
      { name = "rope", amount = 3 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- steps from building to ship: 2-9
            -- worker time: 5.5 sec
            -- number of wares to carry: 5 + 5 + 3 + 3 = 16
            -- (see data/tribes/immovables/shipconstruction_amazons/init.lua)
            -- min. time total: 16 * (2 * 2 * 1.8 + 5.5 + 20 + 35) = 1083.2 sec
            -- max. time total: 16 * (2 * 9 * 1.8 + 5.5 + 20 + 35) = 1486.4 sec
            "call=ship on failure fail",
            "call=ship_preparation"
         }
      },
      ship = {
         -- TRANSLATORS: Completed/Skipped/Did not start constructing a ship because ...
         descname = _("constructing a ship"),
         actions = {
            "construct=amazons_shipconstruction worker:buildship radius:6",
            "sleep=duration:20s"
         }
      },
      ship_preparation = {
         descname = _("working"),
         actions = {
            "sleep=duration:35s"
         }
      },
   },
}

pop_textdomain()
