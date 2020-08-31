push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_ironmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Iron Mine"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "mine",
   enhancement = "empire_ironmine_deep",

   buildcost = {
      log = 4,
      planks = 2
   },
   return_on_dismantle = {
      log = 2,
      planks = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 49, 49 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 49, 49 },
         fps = 10
      },
      empty = {
         pictures = path.list_files(dirname .. "empty_??.png"),
         hotspot = { 49, 49 },
      },
   },

   aihints = {
      mines = "iron",
      mines_percent = 50,
      prohibited_till = 1000
   },

   working_positions = {
      empire_miner = 1
   },

   inputs = {
      { name = "ration", amount = 6 },
      { name = "beer", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining iron because ...
         descname = _"mining iron",
         actions = {
            "return=skipped unless economy needs iron_ore",
            "consume=ration beer",
            "sleep=duration:43s",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
         }
      },
      mine_produce = {
         descname = _"mining iron",
         actions = {
            "animate=working duration:14s",
            "mine=resource_iron radius:2 yield:50% when_empty:5% experience_on_fail:17%",
            "produce=iron_ore",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=ration beer",
            "produce=iron_ore:3",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Iron",
      heading = _"Main Iron Vein Exhausted",
      message =
         pgettext("empire_building", "This iron mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}

pop_textdomain()
