push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_stonemine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("amazons_building", "Stone Mine"),
   icon = dirname .. "menu.png",
   size = "mine",

   buildcost = {
      log = 2,
   },
   return_on_dismantle = {
      log = 1
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
         frames = 15,
         columns = 5,
         rows = 3
      }
   },

   aihints = {
      prohibited_till = 630
   },

   working_positions = {
      amazons_stonecutter = 1
   },

   inputs = {
      { name = "ration", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining granite and quartz because ...
         descname = _("mining granite and quartz"),
         actions = {
            -- Total time: 28.8 + 17 + 21 + 2 * 3.6 = 74
            "return=skipped unless economy needs granite or economy needs quartz",
            "consume=ration",
            "sleep=duration:28.8s",
            "animate=working duration:17s",
            "mine=resource_stones radius:1 yield:100% when_empty:20%",
            "produce=granite",
            "animate=working duration:21s",
            "mine=resource_stones radius:1 yield:100% when_empty:15%",
            "produce=quartz",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Stones"),
      heading = _("Main Stone Vein Exhausted"),
      message =
         pgettext("amazons_building", "This stone mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider dismantling or destroying it."),
   },
}

pop_textdomain()
