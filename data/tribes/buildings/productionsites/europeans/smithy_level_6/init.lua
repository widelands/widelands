push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_smithy_level_6",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Smithy Level 6"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "europeans_smithy_level_7",
      enhancement_cost = {
         marble_column = 1,
         quartz = 1,
         diamond = 1
      },
      enhancement_return_on_dismantle = {
         marble = 1,
         quartz = 1,
         diamond = 1
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 58 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 50, 58 },
      }
   },

   aihints = {
   },

   working_positions = {
      europeans_smith_normal = 2
   },

   inputs = {
      { name = "planks", amount = 6 },
      { name = "spidercloth", amount = 6 },
      { name = "armor", amount = 6 },
      { name = "coal", amount = 6 },
      { name = "iron", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_hammer",
            "call=produce_saw",
            "call=produce_felling_ax",
            "call=produce_pick",
            "call=produce_shovel",
            "call=produce_buckets",
            "call=produce_fire_tongs",
            "call=produce_scythe",
            "call=produce_basket",
            "call=produce_milking_tongs",
            "call=produce_needles",
            "call=produce_bread_paddle",
            "call=produce_fishing_rod",
            "call=produce_fishing_net",
            "call=produce_hunting_bow",
            "call=produce_hunting_spear",
            "call=produce_hook_pole",
            "call=produce_spear_wooden",
            "call=produce_spear_advanced",
            "call=produce_ax_broad",
            "call=produce_armor_chain",
            "call=produce_armor_helmet",
            "call=produce_helmet_mask",
            "call=produce_shield_steel",
            "call=produce_sword_broad",
            "call=produce_trident_steel",
         }
      },
      produce_hammer = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hammer because ...
         descname = _"making a hammer",
         actions = {
            "return=skipped unless economy needs hammer",
            "consume=planks iron",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=hammer"
         }
      },
      produce_saw = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a saw because ...
         descname = _"making a saw",
         actions = {
            "return=skipped unless economy needs saw",
            "consume=planks iron",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=saw"
         }
      },
      produce_felling_ax = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a felling ax because ...
         descname = _"making a felling ax",
         actions = {
            "return=skipped unless economy needs felling_ax",
            "consume=planks iron",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=felling_ax"
         }
      },
      produce_pick = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a pick because ...
         descname = _"making a pick",
         actions = {
            "return=skipped unless economy needs pick",
            "consume=planks iron",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=pick"
         }
      },
      produce_shovel = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a shovel because ...
         descname = _"making a shovel",
         actions = {
            "return=skipped unless economy needs shovel",
            "consume=planks iron",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=shovel"
         }
      },
      produce_buckets = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a pair of buckets because ...
         descname = _"making a pair of buckets",
         actions = {
            "return=skipped unless economy needs buckets",
            "consume=planks iron",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=buckets"
         }
      },
      produce_fire_tongs = {
         -- TRANSLATORS: Completed/Skipped/Did not start making fire tongs because ...
         descname = _"making fire tongs",
         actions = {
            "return=skipped unless economy needs fire_tongs",
            "consume=planks iron",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=fire_tongs"
         }
      },
      produce_scythe = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a scythe because ...
         descname = _"making a scythe",
         actions = {
            "return=skipped unless economy needs scythe",
            "consume=planks iron",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=scythe"
         }
      },
      produce_basket = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a basket because ...
         descname = _"making a basket",
         actions = {
            "return=skipped unless economy needs basket",
            "consume=planks iron",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=basket"
         }
      },
      produce_milking_tongs = {
         -- TRANSLATORS: Completed/Skipped/Did not start making milking tongs because ...
         descname = _"making milking tongs",
         actions = {
            "return=skipped unless economy needs milking_tongs",
            "consume=planks iron",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=milking_tongs"
         }
      },
      produce_needles = {
         -- TRANSLATORS: Completed/Skipped/Did not start making milking tongs because ...
         descname = _"making needles",
         actions = {
            "return=skipped unless economy needs milking_tongs",
            "consume=iron",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=needles"
         }
      },
      produce_bread_paddle = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a bread paddle because ...
         descname = _"making a bread paddle",
         actions = {
            "return=skipped unless economy needs bread_paddle",
            "consume=planks iron",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=bread_paddle"
         }
      },
      produce_fishing_rod = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a fishing rod because ...
         descname = _"making a fishing rod",
         actions = {
            "return=skipped unless economy needs fishing_rod",
            "consume=planks spidercloth",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=fishing_rod"
         }
      },
      produce_fishing_net = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a fishing net because ...
         descname = _"making a fishing net",
         actions = {
            "return=skipped unless economy needs fishing_net",
            "consume=spidercloth:2",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=fishing_net"
         }
      },
      produce_hunting_bow = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hunting bow because ...
         descname = _"making a hunting bow",
         actions = {
            "return=skipped unless economy needs hunting_bow",
            "consume=planks spidercloth",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=hunting_bow"
         }
      },
      produce_hunting_spear = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hunting spear because ...
         descname = _"making a hunting spear",
         actions = {
            "return=skipped unless economy needs hunting_spear",
            "consume=planks iron",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=hunting_spear"
         }
      },
      produce_hook_pole = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hook pole because ...
         descname = _"making a hook pole",
         actions = {
            "return=skipped unless economy needs hook_pole",
            "consume=planks iron",
            "sleep=duration:30s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=hook_pole"
         }
      },
      produce_spear_wooden = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a wooden spear because ...
         descname = _"forging a wooden spear",
         actions = {
            "return=skipped unless economy needs spear_wooden",
            "consume=planks",
            "sleep=duration:30s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:20s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:10s",
            "produce=spear_wooden"
         }
      },
      produce_spear_advanced = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging an advanced spear because ...
         descname = _"forging an advanced spear",
         actions = {
            "return=skipped unless economy needs spear_advanced",
            "consume=planks coal:2 iron:2",
            "sleep=duration:30s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:20s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:10s",
            "produce=spear_advanced"
         }
      },
      produce_ax_broad = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a broad ax because ...
         descname = _"forging a broad ax",
         actions = {
            "return=skipped unless economy needs ax_broad",
            "consume=planks coal:2 iron:2",
            "sleep=duration:30s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:20s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:10s",
            "produce=ax_broad"
         }
      },
      produce_armor_chain = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a suit of chain armor because ...
         descname = _"forging a suit of chain armor",
         actions = {
            "return=skipped unless economy needs armor_chain",
            "consume=armor coal:2 iron:2",
            "sleep=duration:30s",
            "animate=working duration:30s",
            "produce=armor_chain"
         }
      },
      produce_armor_helmet = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a helmet because ...
         descname = _"forging a helmet",
         actions = {
            "return=skipped unless economy needs armor_helmet",
            "consume=coal iron",
            "sleep=duration:30s",
            "animate=working duration:30s",
            "produce=armor_helmet"
         }
      },
      produce_helmet_mask = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a mask because ...
         descname = _"forging a mask",
         actions = {
            "return=skipped unless economy needs helmet_mask",
            "consume=coal:2 iron:2",
            "sleep=duration:30s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=helmet_mask"
         }
      },
      produce_shield_steel = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a steel shield because ...
         descname = _"forging a steel shield",
         actions = {
            "return=skipped unless economy needs shield_steel",
            "consume=coal:2 iron:2",
            "sleep=duration:30s",
            "animate=working duration:30s",
            "produce=shield_steel"
         }
      },
      produce_sword_broad = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a broadsword because ...
         descname = _"forging a broadsword",
         actions = {
            "return=skipped unless economy needs sword_broad",
            "consume=planks coal:2 iron:2",
            "sleep=duration:20s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:30s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:10s",
            "produce=sword_broad"
         },
      },
      produce_trident_steel = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a steel trident because ...
         descname = _"forging a steel trident",
         actions = {
            "return=skipped unless economy needs trident_steel",
            "consume=planks coal:2 iron:2",
            "sleep=duration:20s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:30s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:10s",
            "produce=trident_steel"
         }
      },
   },
}

pop_textdomain()
