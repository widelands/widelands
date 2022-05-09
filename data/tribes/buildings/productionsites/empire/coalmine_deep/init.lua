push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_coalmine_deep",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Deep Coal Mine"),
   icon = dirname .. "menu.png",
   size = "mine",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 49, 61 },
      },
      empty = {
         hotspot = { 49, 61 },
      },
   },

   spritesheets = {
      working = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 49, 61 }
      },
   },

   aihints = {},

   working_positions = {
      empire_miner = 1,
      empire_miner_master = 1
   },

   inputs = {
      { name = "meal", amount = 6 },
      { name = "beer", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining coal because ...
         descname = _("mining coal"),
         actions = {
            "return=skipped unless economy needs coal",
            "consume=beer meal",
            "sleep=duration:38s",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "return=skipped"
         }
      },
      mine_produce = {
         descname = _("mining coal"),
         actions = {
            "animate=working duration:8s",
            "mine=resource_coal radius:2 yield:100% when_empty:5% experience_on_fail:2%",
            "produce=coal",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=beer meal",
            "produce=coal:5",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Coal"),
      heading = _("Main Coal Vein Exhausted"),
      message =
         pgettext("empire_building", "This coal mine’s main vein is exhausted. Expect strongly diminished returns on investment. This mine can’t be enhanced any further, so you should consider dismantling or destroying it."),
   },
}

pop_textdomain()
