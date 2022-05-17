dirname = "campaigns/atl02.wmf/scripting/tribes/atlanteans_temple_of_satul/"

push_textdomain("scenario_atl02.wmf")

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_temple_of_satul",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Temple of Satul"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      planks = 4,
      granite = 3,
      log = 5,
      spidercloth = 3,
      diamond = 2,
      quartz = 2,
      gold = 2,
   },
   return_on_dismantle = {
      planks = 2,
      granite = 1,
      log = 2
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 86, 91 },
      }
   },

   aihints = {},

   working_positions = {
      atlanteans_initiate = 2
   },

   inputs = {
      { name = "gold", amount = 4 },
      { name = "diamond", amount = 4 },
      { name = "quartz", amount = 4 },
      { name = "smoked_fish", amount = 2 },
      { name = "smoked_meat", amount = 2 },
      { name = "atlanteans_bread", amount = 2 },
      { name = "atlanteans_initiate", amount = 1 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start consecrating priestess because ...
         descname = pgettext("atlanteans_building", "consecrating priestess"),
         actions = {
            "consume=gold:4 diamond:4 quartz:4 smoked_fish,smoked_meat:2 atlanteans_bread:2 atlanteans_initiate",
            "sleep=duration:120s",
            "recruit=atlanteans_priestess"
         }
      },
   }
}

pop_textdomain()
