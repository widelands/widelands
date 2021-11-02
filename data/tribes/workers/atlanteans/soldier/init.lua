-- RST
-- .. _lua_tribes_soldiers:
--
-- Soldiers
-- --------
--
-- Soldiers are specialized workers that can occupy and conquer military sites.
-- Each tribe has exactly one soldier type.
--
-- Soldiers, like workers, are defined in
-- ``data/tribes/workers/<tribe name>/soldier/init.lua``.
-- The soldier will also need its :ref:`help texts <lua_tribes_tribes_helptexts>`,
-- which are defined in ``data/tribes/initialization/<tribe_name>/units.lua``
--
-- .. function:: new_soldier_type{table}
--
--    This function adds the definition of a soldier to the engine.
--
--    :arg table: This table contains all the data that the game engine will add
--                to this soldier. It contains the :ref:`lua_tribes_workers_common`,
--                plus the following additional properties:
--
--    **health**
--        This table defines how many health points the soldiers will have. It
--        has the following entries:
--
--           * ``max_level``: The maximum health level that this soldier can be
--             trained to.
--           * ``base``: The health a level 0 soldier will have.
--           * ``increase_per_level``: The health that a soldier will gain with
--             each level.
--           * ``pictures``: A list of health icons, one for each level.
--
--        Example::
--
--              health = {
--                 max_level = 1,
--                 base = 13500,
--                 increase_per_level = 4000,
--                 pictures = path.list_files(dirname .. "health_level?.png"),
--              },
--
--    **attack**
--        This table defines how good the soldiers are at attacking. It
--        has the following entries:
--
--           * ``max_level``: The maximum attack level that this soldier can be
--             trained to.
--           * ``base``: The minimum attack a level 0 soldier will have. During
--             a battle, the engine will pick a value between ``base`` and
--             ``maximum`` at random.
--           * ``maximum``: The maximum attack a level 0 soldier will have. During
--             a battle, the engine will pick a value between ``base`` and
--             ``maximum`` at random.
--           * ``increase_per_level``: The attack that a soldier will gain with
--             each level.
--           * ``pictures``: A list of attack icons, one for each level.
--
--        Example::
--
--           attack = {
--              max_level = 4,
--              base = 1200,
--              maximum = 1600,
--              increase_per_level = 800,
--              pictures = path.list_files(dirname .. "attack_level?.png"),
--           },
--
--    **defense**
--        This table defines how good the soldiers are at reducing the damage
--        received by a successful attack. It has the following entries:
--
--           * ``max_level``: The maximum defense level that this soldier can be
--             trained to.
--           * ``base``: The defense a level 0 soldier will have.
--           * ``increase_per_level``: The defense that a soldier will gain with
--             each level.
--           * ``pictures``: A list of defense icons, one for each level.
--
--        Example::
--
--           defense = {
--              max_level = 2,
--              base = 6,
--              increase_per_level = 8,
--              pictures = path.list_files(dirname .. "defense_level?.png"),
--           },
--
--    **evade**
--
--        This table defines how good the soldiers are at not being hit by an attack.
--        It has the following entries:
--
--           * ``max_level``: The maximum evade level that this soldier can be
--             trained to.
--           * ``base``: The evade a level 0 soldier will have.
--           * ``increase_per_level``: The evade that a soldier will gain with
--             each level.
--           * ``pictures``: A list of evade icons, one for each level.
--
--        Example::
--
--           evade = {
--              max_level = 2,
--              base = 30,
--              increase_per_level = 17,
--              pictures = path.list_files(dirname .. "evade_level?.png"),
--           },
--
--    **Lists of battle animations**: Each soldier needs the following battle animations:
--       * attack_success_w
--       * attack_success_e
--       * attack_failure_w
--       * attack_failure_e
--       * evade_success_w
--       * evade_success_e
--       * evade_failure_w
--       * evade_failure_e
--       * die_w
--       * die_e
--
--    As well as custom walking and idle animations:
--       * idle
--       * walk_ne
--       * walk_e
--       * walk_se
--       * walk_sw
--       * walk_w
--       * walk_nw
--
--    The engine then picks within the listed animations at random.
--    The lists look like this::
--
--       die_w = {
--          die_w_0 = {...},
--          die_w_1 = {...},
--       },
--
--    With "die_w_0" and "die_w_1" being members of the "animations" table.
--    Each animation name is mapped to a table with the following entries:
--
--       * min_health
--       * min_attack
--       * min_defense
--       * min_evade
--       * max_health
--       * max_attack
--       * max_defense
--       * max_evade
--
--    Each animation will be used only for soldiers whose current training matches these limitations.
--
--    Walking animations have a special syntax::
--
--       walk = {
--          {
--             range = {...},
--             sw = "walk_sw",
--             se = "walk_se",
--             nw = "walk_nw",
--             ne = "walk_ne",
--             w = "walk_w",
--             e = "walk_e",
--          },
--          ...
--       },
--
--    ..note :: You must make sure that each animation type has at least one member for every possible
--    combination of training levels. Furthermore, there must be one and only one walk animation of
--    each type and idle animation for each level combination.
--    ANOTHER NOTE: The animations table has to contain animations named "idle" and "walk_*", but you
--    decide whether they will actually be used (by assigning them (or not) to a table below).
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
--    spritesheets = {
--       atk_fail_e = {
--          fps = 10,
--          frames = 10,
--          rows = 4,
--          columns = 3,
--          hotspot = { 13, 35 }
--       },
--       atk_fail_w = {
--          fps = 10,
--          frames = 10,
--          rows = 4,
--          columns = 3,
--          hotspot = { 36, 39 }
--       },
--       atk_ok_e = {
--          fps = 10,
--          frames = 10,
--          rows = 4,
--          columns = 3,
--          hotspot = { 13, 36 }
--       },
--       atk_ok_w = {
--          fps = 10,
--          frames = 10,
--          rows = 4,
--          columns = 3,
--          hotspot = { 36, 39 }
--       },
--       die_e = {
--          fps = 10,
--          frames = 20,
--          rows = 5,
--          columns = 4,
--          hotspot = { 10, 36 }
--       },
--       die_w = {
--          fps = 10,
--          frames = 20,
--          rows = 5,
--          columns = 4,
--          hotspot = { 10, 36 }
--       },
--       eva_fail_e = {
--          fps = 10,
--          frames = 10,
--          rows = 4,
--          columns = 3,
--          hotspot = { 14, 27 }
--       },
--       eva_fail_w = {
--          fps = 10,
--          frames = 10,
--          rows = 4,
--          columns = 3,
--          hotspot = { 34, 26 }
--       },
--       eva_ok_e = {
--          fps = 20,
--          frames = 20,
--          rows = 5,
--          columns = 4,
--          hotspot = { 21, 28 }
--       },
--       eva_ok_w = {
--          fps = 20,
--          frames = 20,
--          rows = 5,
--          columns = 4,
--          hotspot = { 31, 31 }
--       },
--       idle = {
--          fps = 5,
--          frames = 40,
--          rows = 7,
--          columns = 6,
--          hotspot = { 10, 32 }
--       },
--       walk = {
--          fps = 10,
--          frames = 10,
--          rows = 4,
--          columns = 3,
--          directional = true,
--          hotspot = { 19, 27 }
--       },
--    }
--
--    all_levels_atl = {
--       min_health = 0,
--       min_attack = 0,
--       min_defense = 0,
--       min_evade = 0,
--       max_health = 1,
--       max_attack = 4,
--       max_defense = 2,
--       max_evade = 2,
--    }
--
--    descriptions:new_soldier_type {
--       name = "atlanteans_soldier",
--       descname = pgettext("atlanteans_worker", "Soldier"),
--       animation_directory = dirname,
--       icon = dirname .. "menu.png",
--       vision_range = 2,
--
--       spritesheets = spritesheets,
--
--       -- Battle attributes - initial values and per level increase
--       health = {
--          max_level = 1,
--          base = 13500,
--          increase_per_level = 4000,
--          pictures = path.list_files(dirname .. "health_level?.png"),
--       },
--       attack = {
--          max_level = 4,
--          base = 1200,
--          maximum = 1600,
--          increase_per_level = 920,
--          pictures = path.list_files(dirname .. "attack_level?.png"),
--       },
--       defense = {
--          max_level = 2,
--          base = 6,
--          increase_per_level = 8,
--          pictures = path.list_files(dirname .. "defense_level?.png"),
--       },
--       evade = {
--          max_level = 2,
--          base = 30,
--          increase_per_level = 17,
--          pictures = path.list_files(dirname .. "evade_level?.png"),
--       },
--
--       -- Random animations for battle
--       attack_success_w = {
--          atk_ok_w = all_levels_atl,
--       },
--       attack_success_e = {
--          atk_ok_e = all_levels_atl,
--       },
--       attack_failure_w = {
--          atk_fail_w = all_levels_atl,
--       },
--       attack_failure_e = {
--          atk_fail_e = all_levels_atl,
--       },
--       evade_success_w = {
--          eva_ok_w = all_levels_atl,
--       },
--       evade_success_e = {
--          eva_ok_e = all_levels_atl,
--       },
--       evade_failure_w = {
--          eva_fail_w = all_levels_atl,
--       },
--       evade_failure_e = {
--          eva_fail_e = all_levels_atl,
--       },
--       die_w = {
--          die_w = all_levels_atl,
--       },
--       die_e = {
--          die_e = all_levels_atl,
--       },
--       idle = {
--          idle = all_levels_atl,
--       },
--       walk = {
--          {
--             range = all_levels_atl,
--             sw = "walk_sw",
--             se = "walk_se",
--             nw = "walk_nw",
--             ne = "walk_ne",
--             w = "walk_w",
--             e = "walk_e",
--          },
--       },
--    }
--
--    pop_textdomain()

push_textdomain("tribes")

dirname = path.dirname(__file__)

spritesheets = {
   atk_fail_e = {
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 13, 35 }
   },
   atk_fail_w = {
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 36, 39 }
   },
   atk_ok_e = {
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 13, 36 }
   },
   atk_ok_w = {
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 36, 39 }
   },
   die_e = {
      fps = 10,
      frames = 20,
      rows = 5,
      columns = 4,
      hotspot = { 10, 36 }
   },
   die_w = {
      fps = 10,
      frames = 20,
      rows = 5,
      columns = 4,
      hotspot = { 10, 36 }
   },
   eva_fail_e = {
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 14, 27 }
   },
   eva_fail_w = {
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 34, 26 }
   },
   eva_ok_e = {
      fps = 20,
      frames = 20,
      rows = 5,
      columns = 4,
      hotspot = { 21, 28 }
   },
   eva_ok_w = {
      fps = 20,
      frames = 20,
      rows = 5,
      columns = 4,
      hotspot = { 31, 31 }
   },
   idle = {
      fps = 5,
      frames = 40,
      rows = 7,
      columns = 6,
      hotspot = { 10, 32 }
   },
   walk = {
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      directional = true,
      hotspot = { 19, 27 }
   },
}

all_levels_atl = {
   min_health = 0,
   min_attack = 0,
   min_defense = 0,
   min_evade = 0,
   max_health = 1,
   max_attack = 4,
   max_defense = 2,
   max_evade = 2,
}

descriptions:new_soldier_type {
   name = "atlanteans_soldier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Soldier"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   spritesheets = spritesheets,

   -- Battle attributes - initial values and per level increase
   health = {
      max_level = 1,
      base = 13500,
      increase_per_level = 4000,
      pictures = path.list_files(dirname .. "health_level?.png"),
   },
   attack = {
      max_level = 4,
      base = 1200,
      maximum = 1600,
      increase_per_level = 920,
      pictures = path.list_files(dirname .. "attack_level?.png"),
   },
   defense = {
      max_level = 2,
      base = 6,
      increase_per_level = 8,
      pictures = path.list_files(dirname .. "defense_level?.png"),
   },
   evade = {
      max_level = 2,
      base = 30,
      increase_per_level = 17,
      pictures = path.list_files(dirname .. "evade_level?.png"),
   },

   -- Random animations for battle
   -- TODO(GunChleoc): Make more animations to use the random function
   attack_success_w = {
      atk_ok_w = all_levels_atl,
   },
   attack_success_e = {
      atk_ok_e = all_levels_atl,
   },
   attack_failure_w = {
      atk_fail_w = all_levels_atl,
   },
   attack_failure_e = {
      atk_fail_e = all_levels_atl,
   },
   evade_success_w = {
      eva_ok_w = all_levels_atl,
   },
   evade_success_e = {
      eva_ok_e = all_levels_atl,
   },
   evade_failure_w = {
      eva_fail_w = all_levels_atl,
   },
   evade_failure_e = {
      eva_fail_e = all_levels_atl,
   },
   die_w = {
      die_w = all_levels_atl,
   },
   die_e = {
      die_e = all_levels_atl,
   },
   idle = {
      idle = all_levels_atl,
   },
   walk = {
      {
         range = all_levels_atl,
         sw = "walk_sw",
         se = "walk_se",
         nw = "walk_nw",
         ne = "walk_ne",
         w = "walk_w",
         e = "walk_e",
      },
   },
}

pop_textdomain()
