-- RST
-- .. _lua_tribes_buildings_trainingsites:
--
-- Training Sites
-- --------------
--
-- Trainingsites are a special type of productionsite building where soldiers are
-- housed and wares are being consumed to train the soldiers to better levels.
-- They also have workers working at them.
--
-- Trainingsites are defined in
-- ``data/tribes/buildings/trainingsites/<tribe_name>/<building_name>/init.lua``.
-- The building will also need its help texts, which are defined in
-- ``data/tribes/buildings/trainingsites/<tribe_name>/<building_name>/helptexts.lua``
--
-- .. function:: new_trainingsite_type{table}
--
--    This function adds the definition of a training site building to the engine.
--
--    :arg table: This table contains all the data that the game engine will add to this building.
--                It is a special type of production site, so it has all the entries that :ref:`lua_tribes_buildings_productionsites` can have, plus the following entries:
--
--    **soldier_capacity**
--        *Mandatory*. An int describing how many soldiers this building can house.
--
--    **trainer_patience**
--        *Mandatory*. An int describing how patient the trainer is.
--        If trainer patience runs out, a soldier will be kicked out.
--
--    **soldier attack**
--        *Optional*. A table describing what is needed to train a soldier in attack.
--        It contains the following entries:
--
--        **min_level**
--            *Deprecated*. The minimum attack level that a soldier needs before it
--            can be trained in attack at this training site.
--
--        **max_level**
--            *Deprecated*. The maximum level of attack that a soldier can be trained in.
--
--        **food**
--            *Optional*. A table with the types of food needed to train a
--            soldier in attack. It contains subtables with alternatives, e.g.
--            ``{"fish", "meat"}`` means that fish OR meat is needed,
--            ``{"fish"}, {"meat"}`` means that fish AND meat are needed.
--
--        **weapons**
--            *Optional*. A table with the list of weapons that are used for
--            attack training at the various levels.
--
--        Example::
--
--            ["soldier attack"] = {
--                food = {
--                    {"smoked_fish", "smoked_meat"},
--                    {"atlanteans_bread"}
--                },
--                weapons = {
--                    "trident_long",
--                    "trident_steel",
--                    "trident_double",
--                    "trident_heavy_double"
--                }
--            },
--
--    **soldier defense**
--            *Optional*. Just like ``soldier attack``, but for defense training.
--
--    **soldier health**
--            *Optional*. Just like ``soldier attack``, but for health training.
--
--    **soldier evade**
--            *Optional*. Just like ``soldier attack``, but for evade training.
--
-- For making the UI texts translateable, we also need to push/pop the correct textdomain.
--
-- Example:
--
-- .. code-block:: lua
--
--    push_textdomain("tribes")
--
--    dirname = path.dirname(__file__)
--
--    tribes:new_trainingsite_type {
--       name = "empire_arena",
--       -- TRANSLATORS: This is a building name used in lists of buildings
--       descname = pgettext("empire_building", "Arena"),
--       icon = dirname .. "menu.png",
--       animation_directory = dirname,
--       size = "big",
--       enhancement = "empire_colosseum",
--
--       buildcost = {
--          log = 2,
--          granite = 4,
--          marble = 5,
--          planks = 5,
--          marble_column = 2
--       },
--       return_on_dismantle = {
--          log = 1,
--          granite = 3,
--          marble = 3,
--          planks = 2,
--          marble_column = 1
--       },
--
--       animations = {
--          idle = {
--             hotspot = { 81, 82 }
--          },
--          build = {
--             hotspot = { 82, 83 },
--          }
--       },
--
--       aihints = {
--          trainingsites_max_percent = 10,
--          prohibited_till = 900,
--          very_weak_ai_limit = 1,
--          weak_ai_limit = 2
--       },
--
--       working_positions = {
--          empire_trainer = 1
--       },
--
--       inputs = {
--          { name = "fish", amount = 6 },
--          { name = "meat", amount = 6 },
--          { name = "empire_bread", amount = 10 }
--       },
--
--       ["soldier evade"] = {
--          food = {
--             {"fish", "meat"},
--             {"empire_bread"}
--          }
--       },
--
--       programs = {
--          sleep = {
--             -- TRANSLATORS: Completed/Skipped/Did not start sleeping because ...
--             descname = _"sleeping",
--             actions = {
--                "sleep=duration:5s",
--                "return=skipped",
--             }
--          },
--          upgrade_soldier_evade_0 = {
--             -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
--             descname = pgettext("empire_building", "upgrading soldier evade from level 0 to level 1"),
--             actions = {
--                "checksoldier=soldier:evade level:0", -- Fails when aren't any soldier of level 0 evade
--                "return=failed unless site has empire_bread",
--                "return=failed unless site has fish,meat",
--                "sleep=duration:30s",
--                "checksoldier=soldier:evade level:0", -- Because the soldier can be expelled by the player
--                "consume=empire_bread fish,meat",
--                "train=soldier:evade level:1"
--             }
--          },
--       },
--
--       soldier_capacity = 8,
--       trainer_patience = 8
--    }
--
--    pop_textdomain()

push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   name = "atlanteans_dungeon",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Dungeon"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      planks = 2,
      granite = 4,
      diamond = 2,
      quartz = 2,
      spidercloth = 2,
      gold = 2,
      log = 4
   },
   return_on_dismantle = {
      planks = 1,
      granite = 3,
      diamond = 1,
      quartz = 1,
      gold = 1,
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 47, 47 },
      }
   },

   aihints = {
      trainingsites_max_percent = 40,
      prohibited_till = 1500,
      very_weak_ai_limit = 0,
      weak_ai_limit = 1
   },

   working_positions = {
      atlanteans_trainer = 1
   },

   inputs = {
      { name = "smoked_fish", amount = 6 },
      { name = "smoked_meat", amount = 6 },
      { name = "atlanteans_bread", amount = 10 },
      { name = "trident_long", amount = 4 },
      { name = "trident_steel", amount = 4 },
      { name = "trident_double", amount = 4 },
      { name = "trident_heavy_double", amount = 4 }
   },

   ["soldier attack"] = {
      food = {
         {"smoked_fish", "smoked_meat"},
         {"atlanteans_bread"}
      },
      weapons = {
         "trident_long",
         "trident_steel",
         "trident_double",
         "trident_heavy_double"
      }
   },

   programs = {
      sleep = {
         -- TRANSLATORS: Completed/Skipped/Did not start sleeping because ...
         descname = _"sleeping",
         actions = {
            "sleep=duration:5s",
            "return=skipped",
         }
      },
      upgrade_soldier_attack_0 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("atlanteans_building", "upgrading soldier attack from level 0 to level 1"),
         actions = {
            "checksoldier=soldier:attack level:0",
            "return=failed unless site has trident_long",
            "return=failed unless site has atlanteans_bread",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=duration:30s",
            "checksoldier=soldier:attack level:0",
            "consume=atlanteans_bread smoked_fish,smoked_meat trident_long",
            "train=soldier:attack level:1"
         }
      },
      upgrade_soldier_attack_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("atlanteans_building", "upgrading soldier attack from level 1 to level 2"),
         actions = {
            "checksoldier=soldier:attack level:1",
            "return=failed unless site has trident_steel",
            "return=failed unless site has atlanteans_bread",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=duration:30s",
            "checksoldier=soldier:attack level:1",
            "consume=atlanteans_bread smoked_fish,smoked_meat trident_steel",
            "train=soldier:attack level:2"
         }
      },
      upgrade_soldier_attack_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("atlanteans_building", "upgrading soldier attack from level 2 to level 3"),
         actions = {
            "checksoldier=soldier:attack level:2",
            "return=failed unless site has trident_double",
            "return=failed unless site has atlanteans_bread",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=duration:30s",
            "checksoldier=soldier:attack level:2",
            "consume=atlanteans_bread smoked_fish,smoked_meat trident_double",
            "train=soldier:attack level:3"
         }
      },
      upgrade_soldier_attack_3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("atlanteans_building", "upgrading soldier attack from level 3 to level 4"),
         actions = {
            "checksoldier=soldier:attack level:3",
            "return=failed unless site has trident_heavy_double",
            "return=failed unless site has atlanteans_bread",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=duration:30s",
            "checksoldier=soldier:attack level:3",
            "consume=atlanteans_bread smoked_fish,smoked_meat trident_heavy_double",
            "train=soldier:attack level:4"
         }
      },
   },

   soldier_capacity = 8,
   trainer_patience = 16
}

pop_textdomain()
