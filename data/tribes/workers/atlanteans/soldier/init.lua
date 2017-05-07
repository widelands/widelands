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
-- The soldier will also need its help texts, which are defined in
-- ``data/tribes/wares/<tribe name>/soldier/helptexts.lua``

dirname = path.dirname(__file__)

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
add_walking_animations(animations, "walk", dirname, "walk", {20, 34}, 10)

-- RST
-- .. function:: new_soldier_type(table)
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

tribes:new_soldier_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_soldier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Soldier"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = animations,

   default_target_quantity = 10,

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
      increase_per_level = 800,
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
      "atk_ok_w",
   },
   attack_success_e = {
      "atk_ok_e",
   },
   attack_failure_w = {
      "atk_fail_w",
   },
   attack_failure_e = {
      "atk_fail_e",
   },
   evade_success_w = {
      "eva_ok_w",
   },
   evade_success_e = {
      "eva_ok_e",
   },
   evade_failure_w = {
      "eva_fail_w",
   },
   evade_failure_e = {
      "eva_fail_e",
   },
   die_w = {
      "die_w",
   },
   die_e = {
      "die_e",
   }
}
