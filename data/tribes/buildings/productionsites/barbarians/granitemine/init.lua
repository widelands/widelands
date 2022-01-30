push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "barbarians_granitemine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Granite Mine"),
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

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 42, 35 },
      },
      empty = {
         hotspot = { 42, 35 },
      },
   },

   spritesheets = {
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 42, 35 }
      },
      working = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 42, 35 }
      },
   },

   aihints = {
      prohibited_till = 600
   },

   working_positions = {
      barbarians_miner = 1
   },

   inputs = {
      { name = "ration", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining granite because ...
         descname = _("mining granite"),
         actions = {
            "return=skipped unless economy needs granite",
            "consume=ration",
            "sleep=duration:20s",
            "call=mine_produce",
            "call=mine_produce",
            "return=skipped"
         }
      },
      mine_produce = {
         descname = _("mining granite"),
         actions = {
            "animate=working duration:10s",
            "mine=resource_stones radius:2 yield:100% when_empty:5% experience_on_fail:2%",
            "produce=granite",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=ration",
            "produce=granite:2",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Granite"),
      heading = _("Main Granite Vein Exhausted"),
      message =
         pgettext("barbarians_building", "This granite mine’s main vein is exhausted. Expect strongly diminished returns on investment. This mine can’t be enhanced any further, so you should consider dismantling or destroying it."),
   },
}

pop_textdomain()
