push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_tailors_shop",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Tailor’s Shop"),
   icon = dirname .. "menu.png",
   size = "medium",

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {50, 82},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         directory = dirname,
         basename = "working",
         hotspot = {50, 82},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {50, 72}
      }
   },

   aihints = {
      prohibited_till = 890
   },

   working_positions = {
      frisians_seamstress = 1,
      frisians_seamstress_master = 1,
   },

   inputs = {
      { name = "fur_garment", amount = 8 },
      { name = "iron", amount = 8 },
      { name = "gold", amount = 4 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=weave_studded",
            "call=weave_gold",
            "return=skipped"
         },
      },
      weave_studded = {
         -- TRANSLATORS: Completed/Skipped/Did not start sewing studded fur garment because ...
         descname = _("sewing studded fur garment"),
         actions = {
            -- time total: 50 + 3.6
            "return=skipped unless economy needs fur_garment_studded",
            "consume=fur_garment iron",
            "sleep=duration:25s",
            "animate=working duration:25s",
            "produce=fur_garment_studded"
         },
      },
      weave_gold = {
         -- TRANSLATORS: Completed/Skipped/Did not start sewing golden fur garment because ...
         descname = _("sewing golden fur garment"),
         actions = {
            -- time total: 50 + 3.6
            "return=skipped unless economy needs fur_garment_golden",
            "consume=fur_garment iron gold",
            "sleep=duration:25s",
            "animate=working duration:25s",
            "produce=fur_garment_golden"
         },
      },
   },
}

pop_textdomain()
