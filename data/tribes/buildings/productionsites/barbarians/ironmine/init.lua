push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "barbarians_ironmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Iron Mine"),
   icon = dirname .. "menu.png",
   size = "mine",

   buildcost = {
      log = 4,
      granite = 2
   },
   return_on_dismantle = {
      log = 2,
      granite = 1
   },

   enhancement = {
      name = "barbarians_ironmine_deep",
      enhancement_cost = {
         log = 4,
         granite = 2
      },
      enhancement_return_on_dismantle = {
         log = 2,
         granite = 1
      }
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 21, 36 },
      },
      empty = {
         hotspot = { 21, 36 },
      },
   },

   spritesheets = {
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 21, 36 }
      },
      working = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 21, 36 }
      },
   },

   aihints = {
      prohibited_till = 1000
   },

   working_positions = {
      barbarians_miner = 1
   },

   inputs = {
      { name = "ration", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining iron because ...
         descname = _("mining iron"),
         actions = {
            -- time total: 45.4 + 20 + 3.6 = 69 sec
            "return=skipped unless economy needs iron_ore",
            "consume=ration",
            "sleep=duration:45s400ms",
            "animate=working duration:20s",
            "mine=resource_iron radius:2 yield:33.33% when_empty:5% experience_on_fail:17%",
            "produce=iron_ore"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Iron"),
      heading = _("Main Iron Vein Exhausted"),
      message =
         pgettext("barbarians_building", "This iron mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}

pop_textdomain()
