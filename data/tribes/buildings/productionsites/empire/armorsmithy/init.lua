push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_armorsmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Armor Smithy"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      granite = 2,
      marble = 2,
      marble_column = 3
   },
   return_on_dismantle = {
      granite = 1,
      marble = 1,
      marble_column = 2
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 50, 66 },
      },
      unoccupied = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 50, 66 },
      },
      build = {
         frames = 4,
         columns = 4,
         rows = 1,
         hotspot = { 50, 66 }
      },
      working = {
         fps = 5,
         frames = 20,
         columns = 10,
         rows = 2,
         hotspot = { 50, 66 }
      },
   },

   aihints = {
      prohibited_till = 1400,
   },

   working_positions = {
      empire_armorsmith = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "iron", amount = 8 },
      { name = "gold", amount = 8 },
      { name = "cloth", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=produce_armor_helmet",
            "call=produce_armor",
            "call=produce_armor_chain",
            "call=produce_armor_helmet",
            "call=produce_armor_gilded",
         }
      },
      produce_armor_helmet = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a helmet because ...
         descname = _("forging a helmet"),
         actions = {
            -- time total: 67 + 3.6
            "return=skipped unless economy needs armor_helmet",
            "consume=iron coal",
            "sleep=duration:47s",
            "animate=working duration:20s",
            "produce=armor_helmet"
         }
      },
      produce_armor = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a suit of armor because ...
         descname = _("forging a suit of armor"),
         actions = {
            -- time total: 77 + 3.6
            "return=skipped unless economy needs armor",
            "consume=iron coal cloth",
            "sleep=duration:32s",
            "animate=working duration:45s",
            "produce=armor"
         }
      },
      produce_armor_chain = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a suit of chain armor because ...
         descname = _("forging a suit of chain armor"),
         actions = {
            -- time total: 77 + 3.6
            "return=skipped unless economy needs armor_chain",
            "consume=iron:2 coal cloth",
            "sleep=duration:32s",
            "animate=working duration:45s",
            "produce=armor_chain"
         }
      },
      produce_armor_gilded = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a suit of gilded armor because ...
         descname = _("forging a suit of gilded armor"),
         actions = {
            -- time total: 77 + 3.6
            "return=skipped unless economy needs armor_gilded",
            "consume=iron:2 coal:2 cloth gold",
            "sleep=duration:32s",
            "animate=working duration:45s",
            "produce=armor_gilded"
         }
      },
   },
}

pop_textdomain()
