push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_coalmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Coal Mine"),
   icon = dirname .. "menu.png",
   size = "mine",

   buildcost = {
      log = 7,
      planks = 4,
      spidercloth = 1
   },
   return_on_dismantle = {
      log = 3,
      planks = 2
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 53, 61 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         hotspot = { 53, 61 },
      },
      empty = {
         hotspot = { 53, 61 },
      },
   },

   aihints = {
      prohibited_till = 910
   },

   working_positions = {
      atlanteans_miner = 3
   },

   inputs = {
      { name = "smoked_fish", amount = 10 },
      { name = "smoked_meat", amount = 6 },
      { name = "atlanteans_bread", amount = 10 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining coal because ...
         descname = _("mining coal"),
         actions = {
            -- time total: 105 + 7 x 3.6
            "return=skipped unless economy needs coal",
            "consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
            "sleep=duration:35s",
            -- after having the food the miners are working 7 times
            -- each cycle lasts 10 seconds for mining and producing coal
            -- and 3.6 seconds to deliver the coal to the flag
            -- calling the subroutine "mine_produce" has the effect
            -- that even when depleted the mine has 7 working cycles
            -- as no call cycle is skipped due to a failed mine command
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
            "call=mine_produce",
         },
      },
      mine_produce = {
         descname = _("mining coal"),
         actions = {
            "animate=working duration:10s",
            "mine=resource_coal radius:4 yield:100% when_empty:5%",
            "produce=coal",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
            "produce=coal:7",
         }
      },
   },

   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Coal"),
      heading = _("Main Coal Vein Exhausted"),
      message =
         pgettext("atlanteans_building", "This coal mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider dismantling or destroying it."),
   },
}

pop_textdomain()
