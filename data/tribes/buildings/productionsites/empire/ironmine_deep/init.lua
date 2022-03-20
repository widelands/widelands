push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_ironmine_deep",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Deep Iron Mine"),
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
         -- TRANSLATORS: Completed/Skipped/Did not start mining iron because ...
         descname = _("mining iron"),
         actions = {
            "return=skipped unless economy needs iron_ore",
            "consume=meal beer",
            "sleep=duration:39s",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "return=skipped"
         }
      },
      mine_produce = {
         descname = _("mining iron"),
         actions = {
            "animate=working duration:10s",
            "mine=resource_iron radius:2 yield:100% when_empty:5% experience_on_fail:2%",
            "produce=iron_ore",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=meal beer",
            "produce=iron_ore:4",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Iron"),
      heading = _("Main Iron Vein Exhausted"),
      message =
         pgettext("empire_building", "This iron mine’s main vein is exhausted. Expect strongly diminished returns on investment. This mine can’t be enhanced any further, so you should consider dismantling or destroying it."),
   },
}

pop_textdomain()
