push_textdomain("tribes")

dirname = path.dirname (__file__)

-- Quick overview: We use 4 sets of animations here.
-- The default set (fri_rookie, "rookie") is for soldiers with no helmet and one sword.
-- The set prefixed "h" (fri_health, "helm") is for soldiers with a helmet and one sword.
-- The set prefixed "s" (fri_attack, "sword") is for soldiers with no helmet and two swords.
-- The set prefixed "sh" (fri_hero, "hero") is for soldiers with a helmet and two swords.
--
-- Side Note for the w/e naming scheme:
-- The attack and evade animation consider the soldier on the LEFT to be called E
-- and the soldier on the RIGHT to be called W.
-- In my animations, the soldier on the LEFT is considered the WESTERN soldier
-- and the soldier on the RIGHT the EASTERN one.
-- The die animations consider the soldier on the LEFT to called W
-- and the soldier on the RIGHT to be called E.
-- That's the same as in the animations.
-- Confused?
-- Oh, and by the way, the soldiers' swords are not long enough to touch the opponent if the
-- hotspot is at the soldier's feet. That's why all battle hotspots are shifted by 3 pixels.
--
-- The above explanation refers only to frisian soldiers. Other tribes's soldiers
-- may follow other conventions described in their respective init.lua's.

spritesheets = {
   -- Rookie
   idle = {
      directory = dirname .. "rookie",
      fps = 10,
      frames = 50,
      rows = 8,
      columns = 7,
      hotspot = { 7, 23 }
   },
   walk = {
      directory = dirname .. "rookie",
      fps = 15,
      frames = 10,
      rows = 4,
      columns = 3,
      directional = true,
      hotspot = { 7, 24 }
   },
   atk_ok_1_e = {
      directory = dirname .. "rookie",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 23, 24 }
   },
   atk_ok_1_w = {
      directory = dirname .. "rookie",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { -1, 22 }
   },
   atk_ok_2_e = {
      directory = dirname .. "rookie",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 23, 23 }
   },
   atk_ok_2_w = {
      directory = dirname .. "rookie",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { -1, 23 }
   },
   atk_fail_1_e = {
      directory = dirname .. "rookie",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 23, 23 }
   },
   atk_fail_1_w = {
      directory = dirname .. "rookie",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { -1, 22 }
   },
   atk_fail_2_e = {
      directory = dirname .. "rookie",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 23, 23 }
   },
   atk_fail_2_w = {
      directory = dirname .. "rookie",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { -1, 23 }
   },
   eva_fail_e = {
      directory = dirname .. "rookie",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 12, 23 }
   },
   eva_fail_w = {
      directory = dirname .. "rookie",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { -1, 22 }
   },
   eva_ok_e = {
      directory = dirname .. "rookie",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 11, 23 }
   },
   eva_ok_w = {
      directory = dirname .. "rookie",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { -1, 22 }
   },
   die_e = {
      directory = dirname .. "rookie",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 12, 23 }
   },
   die_w = {
      directory = dirname .. "rookie",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 15, 25 }
   },
   -- Health upgrade only
   health_idle = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 50,
      rows = 8,
      columns = 7,
      hotspot = { 7, 25 }
   },
   health_walk = {
      directory = dirname .. "helm",
      fps = 15,
      frames = 10,
      rows = 4,
      columns = 3,
      directional = true,
      hotspot = { 7, 25 }
   },
   health_atk_ok_1_e = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 23, 24 }
   },
   health_atk_ok_1_w = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { -1, 24 }
   },
   health_atk_ok_2_e = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 23, 24 }
   },
   health_atk_ok_2_w = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { -1, 24 }
   },
   health_atk_fail_1_e = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 23, 24 }
   },
   health_atk_fail_1_w = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { -1, 24 }
   },
   health_atk_fail_2_e = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 23, 24 }
   },
   health_atk_fail_2_w = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { -1, 24 }
   },
   health_eva_fail_e = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 12, 24 }
   },
   health_eva_fail_w = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { -1, 24 }
   },
   health_eva_ok_e = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 11, 24 }
   },
   health_eva_ok_w = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { -1, 24 }
   },
   health_die_f_e = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 19, 24 }
   },
   health_die_f_w = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { -1, 24 }
   },
   health_die_b_e = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 12, 24 }
   },
   health_die_b_w = {
      directory = dirname .. "helm",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 15, 24 }
   },

   -- Health and attack upgrade
   hero_idle = {
      directory = dirname .. "hero",
      fps = 10,
      frames = 50,
      rows = 8,
      columns = 7,
      hotspot = { 8, 25 }
   },
   hero_walk = {
      directory = dirname .. "hero",
      fps = 15,
      frames = 10,
      rows = 4,
      columns = 3,
      directional = true,
      hotspot = { 8, 25 }
   },
   hero_atk_fail_1_e = {
      directory = dirname .. "hero",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 22, 25 }
   },
   hero_atk_fail_1_w = {
      directory = dirname .. "hero",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 0, 25 }
   },
   hero_atk_fail_2_e = {
      directory = dirname .. "hero",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 23, 25 }
   },
   hero_atk_fail_2_w = {
      directory = dirname .. "hero",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 0, 25 }
   },
   hero_atk_ok_1_e = {
      directory = dirname .. "hero",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 23, 25 }
   },
   hero_atk_ok_1_w = {
      directory = dirname .. "hero",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 0, 25 }
   },
   hero_atk_ok_2_e = {
      directory = dirname .. "hero",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 23, 25 }
   },
   hero_atk_ok_2_w = {
      directory = dirname .. "hero",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 0, 25 }
   },
   hero_eva_fail_e = {
      directory = dirname .. "hero",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 13, 25 }
   },
   hero_eva_fail_w = {
      directory = dirname .. "hero",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 0, 25 }
   },
   hero_eva_ok_e = {
      directory = dirname .. "hero",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 13, 25 }
   },
   hero_eva_ok_w = {
      directory = dirname .. "hero",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 0, 25 }
   },
   hero_die_e = {
      directory = dirname .. "hero",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 19, 24 }
   },
   hero_die_w = {
      directory = dirname .. "hero",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { -1, 24 }
   },

   -- Sword upgrade only
   attack_idle = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 50,
      rows = 8,
      columns = 7,
      hotspot = { 8, 23 }
   },
   attack_walk = {
      directory = dirname .. "sword",
      fps = 15,
      frames = 10,
      rows = 4,
      columns = 3,
      directional = true,
      hotspot = { 8, 24 }
   },
   attack_atk_fail_1_e = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 23, 24 }
   },
   attack_atk_fail_1_w = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 0, 23 }
   },
   attack_atk_fail_2_e = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 23, 23 }
   },
   attack_atk_fail_2_w = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 0, 23 }
   },
   attack_atk_ok_1_e = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 23, 24 }
   },
   attack_atk_ok_1_w = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 0, 23 }
   },
   attack_atk_ok_2_e = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 23, 23 }
   },
   attack_atk_ok_2_w = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 0, 23 }
   },
   attack_eva_fail_e = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 13, 23 }
   },
   attack_eva_fail_w = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 0, 23 }
   },
   attack_eva_ok_e = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 13, 23 }
   },
   attack_eva_ok_w = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 0, 24 }
   },
   attack_die_b_e = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 13, 23 }
   },
   attack_die_b_w = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 17, 25 }
   },
   attack_die_f_e = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 19, 22 }
   },
   attack_die_f_w = {
      directory = dirname .. "sword",
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { -1, 21 }
   },
}

-- Convenience definitions so we don´t have to repeat this table for every single entry below
fri_health = {
   min_health = 1,
   min_attack = 0,
   min_defense = 0,
   min_evade = 0,
   max_health = 2,
   max_attack = 3,
   max_defense = 2,
   max_evade = 0,
}
fri_attack = {
   min_health = 0,
   min_attack = 4,
   min_defense = 0,
   min_evade = 0,
   max_health = 0,
   max_attack = 6,
   max_defense = 2,
   max_evade = 0,
}
fri_hero = {
   min_health = 1,
   min_attack = 4,
   min_defense = 0,
   min_evade = 0,
   max_health = 2,
   max_attack = 6,
   max_defense = 2,
   max_evade = 0,
}
fri_rookie = {
   min_health = 0,
   min_attack = 0,
   min_defense = 0,
   min_evade = 0,
   max_health = 0,
   max_attack = 3,
   max_defense = 2,
   max_evade = 0,
}

descriptions:new_soldier_type {
   name = "frisians_soldier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Soldier"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   spritesheets = spritesheets,

   -- Battle attributes - initial values and per level increase
   health = {
      max_level = 2,
      base = 12250,
      increase_per_level = 2955,
      pictures = path.list_files (dirname .. "health_level?.png"),
   },
   attack = {
      max_level = 6,
      base = 1300,
      maximum = 1600,
      increase_per_level = 1006,
      pictures = path.list_files (dirname .. "attack_level?.png"),
   },
   defense = {
      max_level = 2,
      base = 4,
      increase_per_level = 16,
      pictures = path.list_files (dirname .. "defense_level?.png"),
   },
   evade = {
      max_level = 0,
      base = 35,
      increase_per_level = 0,
      pictures = path.list_files (dirname .. "evade_level?.png"),
   },

   -- Random animations for battle
   attack_success_e = {
      atk_ok_1_w = fri_rookie,
      atk_ok_2_w = fri_rookie,
      health_atk_ok_1_w = fri_health,
      health_atk_ok_2_w = fri_health,
      attack_atk_ok_1_w = fri_attack,
      attack_atk_ok_2_w = fri_attack,
      hero_atk_ok_1_w = fri_hero,
      hero_atk_ok_2_w = fri_hero,
   },
   attack_success_w = {
      atk_ok_1_e = fri_rookie,
      atk_ok_2_e = fri_rookie,
      health_atk_ok_1_e = fri_health,
      health_atk_ok_2_e = fri_health,
      attack_atk_ok_1_e = fri_attack,
      attack_atk_ok_2_e = fri_attack,
      hero_atk_ok_1_e = fri_hero,
      hero_atk_ok_2_e = fri_hero,
   },
   attack_failure_e = {
      atk_fail_1_w = fri_rookie,
      atk_fail_2_w = fri_rookie,
      health_atk_fail_1_w = fri_health,
      health_atk_fail_2_w = fri_health,
      attack_atk_fail_1_w = fri_attack,
      attack_atk_fail_2_w = fri_attack,
      hero_atk_fail_1_w = fri_hero,
      hero_atk_fail_2_w = fri_hero,
   },
   attack_failure_w = {
      atk_fail_1_e = fri_rookie,
      atk_fail_2_e = fri_rookie,
      health_atk_fail_1_e = fri_health,
      health_atk_fail_2_e = fri_health,
      attack_atk_fail_1_e = fri_attack,
      attack_atk_fail_2_e = fri_attack,
      hero_atk_fail_1_e = fri_hero,
      hero_atk_fail_2_e = fri_hero,
   },
   evade_failure_e = {
      health_eva_fail_w = fri_health,
      hero_eva_fail_w = fri_hero,
      eva_fail_w = fri_rookie,
      attack_eva_fail_w = fri_attack,
   },
   evade_failure_w = {
      health_eva_fail_e = fri_health,
      hero_eva_fail_e = fri_hero,
      eva_fail_e = fri_rookie,
      attack_eva_fail_e = fri_attack,
   },
   evade_success_e = {
      health_eva_ok_w = fri_health,
      hero_eva_ok_w = fri_hero,
      eva_ok_w = fri_rookie,
      attack_eva_ok_w = fri_attack,
   },
   evade_success_w = {
      health_eva_ok_e = fri_health,
      hero_eva_ok_e = fri_hero,
      eva_ok_e = fri_rookie,
      attack_eva_ok_e = fri_attack,
   },
   die_w = {
      health_die_f_w = fri_health,
      health_die_b_w = fri_health,
      hero_die_w = fri_hero,
      die_w = fri_rookie,
      attack_die_f_w = fri_attack,
      attack_die_b_w = fri_attack,
   },
   die_e = {
      health_die_f_e = fri_health,
      health_die_b_e = fri_health,
      hero_die_e = fri_hero,
      die_e = fri_rookie,
      attack_die_f_e = fri_attack,
      attack_die_b_e = fri_attack,
   },
   -- level-dependent walking and idle animations
   idle = {
      health_idle = fri_health,
      hero_idle = fri_hero,
      idle = fri_rookie,
      attack_idle = fri_attack,
   },
   walk = {
      {
         range = fri_rookie,
         sw = "walk_sw",
         se = "walk_se",
         nw = "walk_nw",
         ne = "walk_ne",
         w = "walk_w",
         e = "walk_e",
      },
      {
         range = fri_attack,
         sw = "attack_walk_sw",
         se = "attack_walk_se",
         nw = "attack_walk_nw",
         ne = "attack_walk_ne",
         w = "attack_walk_w",
         e = "attack_walk_e",
      },
      {
         range = fri_health,
         sw = "health_walk_sw",
         se = "health_walk_se",
         nw = "health_walk_nw",
         ne = "health_walk_ne",
         w = "health_walk_w",
         e = "health_walk_e",
      },
      {
         range = fri_hero,
         sw = "hero_walk_sw",
         se = "hero_walk_se",
         nw = "hero_walk_nw",
         ne = "hero_walk_ne",
         w = "hero_walk_w",
         e = "hero_walk_e",
      },
   },
}

pop_textdomain()
