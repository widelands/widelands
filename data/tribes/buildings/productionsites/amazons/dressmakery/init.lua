push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_dressmakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Dressmakery"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      balsa = 3,
      log = 3,
      rubber = 3,
      rope = 1,
   },
   return_on_dismantle = {
      balsa = 1,
      log = 2,
      rubber = 1,
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {43, 44}},
      unoccupied = {hotspot = {43, 44}},
   },
   spritesheets = {
      working = {
         hotspot = {43, 44},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      }
   },

   aihints = {
      prohibited_till = 750,
   },

   working_positions = {
      amazons_dressmaker= 1
   },

   inputs = {
      { name = "ironwood", amount = 2 },
      { name = "balsa", amount = 6 },
      { name = "rubber", amount = 9 },
      { name = "rope", amount = 6 },
      { name = "gold", amount = 3 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=produce_tunic",
            "call=produce_helmet_wooden",
            "call=produce_armor_wooden",
            "call=produce_warriors_coat",
            "call=produce_boots_sturdy",
            "call=produce_vest_padded",
            "call=produce_boots_swift",
            "call=produce_boots_hero",
            "call=produce_protector_padded",
         },
      },
      produce_tunic = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a felling ax because ...
         descname = _("sewing a tunic"),
         actions = {
            "return=skipped unless economy needs tunic",
            "consume=rubber rope",
            "sleep=duration:32s",
            "animate=working duration:35s",
            "produce=tunic"
         },
      },
      produce_armor_wooden = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a pick because ...
         descname = _("making a light wooden armor"),
         actions = {
            "return=skipped unless economy needs armor_wooden",
            "consume=balsa:2 rope",
            "sleep=duration:32s",
            "animate=working duration:35s",
            "produce=armor_wooden"
         },
      },
      produce_helmet_wooden = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a scythe because ...
         descname = _("making a wooden helmet"),
         actions = {
            "return=skipped unless economy needs helmet_wooden",
            "consume=ironwood rubber",
            "sleep=duration:32s",
            "animate=working duration:35s",
            "produce=helmet_wooden"
         },
      },
      produce_warriors_coat = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a shovel because ...
         descname = _("sewing a warrior’s coat"),
         actions = {
            "return=skipped unless economy needs warriors_coat",
            "consume=ironwood balsa:2 rubber gold",
            "sleep=duration:32s",
            "animate=working duration:35s",
            "produce=warriors_coat"
         },
      },
      produce_boots_sturdy = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a basket because ...
         descname = _("making sturdy boots"),
         actions = {
            "return=skipped unless economy needs boots_sturdy",
            "consume=rubber balsa",
            "sleep=duration:32s",
            "animate=working duration:35s",
            "produce=boots_sturdy"
         },
      },
      produce_boots_swift = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hunting spear because ...
         descname = _("making swift boots"),
         actions = {
            "return=skipped unless economy needs boots_swift",
            "consume=rubber:3",
            "sleep=duration:32s",
            "animate=working duration:35s",
            "produce=boots_swift"
         },
      },
      produce_boots_hero = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a fishing net because ...
         descname = _("making hero boots"),
         actions = {
            "return=skipped unless economy needs boots_hero",
            "consume=rubber:3 gold",
            "sleep=duration:32s",
            "animate=working duration:35s",
            "produce=boots_hero"
         },
      },
      produce_vest_padded = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a bread paddle because ...
         descname = _("making a padded vest"),
         actions = {
            "return=skipped unless economy needs vest_padded",
            "consume=rubber:2 rope:2",
            "sleep=duration:32s",
            "animate=working duration:35s",
            "produce=vest_padded"
         },
      },
      produce_protector_padded = {
         -- TRANSLATORS: Completed/Skipped/Did not start making kitchen tools because ...
         descname = _("making a padded protector"),
         actions = {
            "return=skipped unless economy needs protector_padded",
            "consume=rubber:2 rope:2 balsa gold",
            "sleep=duration:32s",
            "animate=working duration:35s",
            "produce=protector_padded"
         },
      },
   },
}

pop_textdomain()
