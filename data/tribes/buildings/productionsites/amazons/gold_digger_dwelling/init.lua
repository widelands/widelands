push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_gold_digger_dwelling",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("amazons_building", "Gold Digger Dwelling"),
   icon = dirname .. "menu.png",
   size = "mine",

   buildcost = {
      log = 4,
      rope = 1
   },
   return_on_dismantle = {
      log = 2
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {39, 46}},
      empty = {hotspot = {39, 46}},
      unoccupied = {hotspot = {39, 46}},
   },
   spritesheets = {
      working = {
         hotspot = {39, 46},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      }
   },

   aihints = {
      prohibited_till = 1100
   },

   working_positions = {
      amazons_gold_digger = 1
   },

   inputs = {
      { name = "ration", amount = 3 },
      { name = "water", amount = 10 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining gold because ...
         descname = _("mining gold"),
         actions = {
            "return=skipped unless economy needs gold_dust",
            "consume=ration water:5",
            "sleep=duration:45s",
            "animate=working duration:20s",
            "mine=resource_gold radius:1 yield:100% when_empty:5%",
            "produce=gold_dust"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Gold"),
      heading = _("Main Gold Vein Exhausted"),
      message =
         pgettext("amazons_building", "This gold digger dwelling’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider dismantling or destroying it."),
   },
}

pop_textdomain()
