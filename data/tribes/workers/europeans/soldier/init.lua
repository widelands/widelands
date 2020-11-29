push_textdomain("tribes")

dirname = path.dirname(__file__)
level_indicator = dirname .. "/level_indicator/"

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 10, 36 },
      fps = 5
   },
   atk_ok_e = {
      pictures = path.list_files(dirname .. "atk_ok_e_??.png"),
      hotspot = { 36, 40 },
      fps = 10
   },
   atk_fail_e = {
      pictures = path.list_files(dirname .. "atk_fail_e_??.png"),
      hotspot = { 36, 40 },
      fps = 10
   },
   eva_ok_e = {
      pictures = path.list_files(dirname .. "eva_ok_e_??.png"),
      hotspot = { 36, 40 },
      fps = 20
   },
   eva_fail_e = {
      pictures = path.list_files(dirname .. "eva_fail_e_??.png"),
      hotspot = { 36, 40 },
      fps = 10
   },
   atk_ok_w = {
      pictures = path.list_files(dirname .. "atk_ok_w_??.png"),
      hotspot = { 36, 40 },
      fps = 10
   },
   atk_fail_w = {
      pictures = path.list_files(dirname .. "atk_fail_w_??.png"),
      hotspot = { 36, 40 },
      fps = 10
   },
   eva_ok_w = {
      pictures = path.list_files(dirname .. "eva_ok_w_??.png"),
      hotspot = { 36, 40 },
      fps = 20
   },
   eva_fail_w = {
      pictures = path.list_files(dirname .. "eva_fail_w_??.png"),
      hotspot = { 36, 40 },
      fps = 10
   },
   die_w = {
      pictures = path.list_files(dirname .. "die_??.png"),
      hotspot = { 10, 36 },
      fps = 10
   },
   die_e = {
      pictures = path.list_files(dirname .. "die_??.png"),
      hotspot = { 10, 36 },
      fps = 10
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {20, 34}, 10)

all_levels_eur = {
   min_health = 0,
   min_attack = 0,
   min_defense = 0,
   min_evade = 0,
   max_health = 6,
   max_attack = 6,
   max_defense = 6,
   max_evade = 6,
}

-- RST
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
--    The engine then picks within the listed animations at random.
--    The lists look like this::
--
--       die_w = {
--          "die_w_0",
--          "die_w_1",
--       },
--
--    With "die_w_0" and "die_w_1" being members of the "animations" table.

descriptions:new_soldier_type {
   msgctxt = "europeans_worker",
   name = "europeans_soldier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Soldier"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = animations,

   default_target_quantity = 12,
   aihints = {
     preciousness = {
        europeans = 10
      },
   },

   -- Battle attributes - initial values and per level increase
   health = {
      max_level = 6,
      base = 12800,
      increase_per_level = 1585,
      pictures = path.list_files(level_indicator .. "health_level?.png"),
   },
   defense = {
      max_level = 6,
      base = 18,
      increase_per_level = 2,
      pictures = path.list_files(level_indicator .. "defense_level?.png"),
   },
   attack = {
      max_level = 6,
      base = 1150,
      maximum = 1450,
      increase_per_level = 500,
      pictures = path.list_files(level_indicator .. "attack_level?.png"),
   },
   evade = {
      max_level = 6,
      base = 32,
      increase_per_level = 5,
      pictures = path.list_files(level_indicator .. "evade_level?.png"),
   },

   -- Random animations for battle
   -- TODO(GunChleoc): Make more animations to use the random function
   attack_success_w = {
      atk_ok_w = all_levels_eur,
   },
   attack_success_e = {
      atk_ok_e = all_levels_eur,
   },
   attack_failure_w = {
      atk_fail_w = all_levels_eur,
   },
   attack_failure_e = {
      atk_fail_e = all_levels_eur,
   },
   evade_success_w = {
      eva_ok_w = all_levels_eur,
   },
   evade_success_e = {
      eva_ok_e = all_levels_eur,
   },
   evade_failure_w = {
      eva_fail_w = all_levels_eur,
   },
   evade_failure_e = {
      eva_fail_e = all_levels_eur,
   },
   die_w = {
      die_w = all_levels_eur,
   },
   die_e = {
      die_e = all_levels_eur,
   },
   idle = {
      idle = all_levels_eur,
   },
   walk = {
      {
         range = all_levels_eur,
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
