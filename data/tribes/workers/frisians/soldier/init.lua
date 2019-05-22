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

animations = {
   -- normal
   idle = {
      pictures = path.list_files (dirname .. "rookie/idle_??.png"),
      hotspot = {20, 24},
      fps = 10
   },
   atk_ok_1_e = {
      pictures = path.list_files (dirname .. "rookie/atk_ok_1_e_??.png"),
      hotspot = {23, 24},
      fps = 10
   },
   atk_fail_1_e = {
      pictures = path.list_files (dirname .. "rookie/atk_fail_1_e_??.png"),
      hotspot = {23, 23},
      fps = 10
   },
   atk_ok_1_w = {
      pictures = path.list_files (dirname .. "rookie/atk_ok_1_w_??.png"),
      hotspot = {-1, 22},
      fps = 10
   },
   atk_fail_1_w = {
      pictures = path.list_files (dirname .. "rookie/atk_fail_1_w_??.png"),
      hotspot = {-1, 22},
      fps = 10
   },
   atk_ok_2_e = {
      pictures = path.list_files (dirname .. "rookie/atk_ok_2_e_??.png"),
      hotspot = {23, 23},
      fps = 10
   },
   atk_fail_2_e = {
      pictures = path.list_files (dirname .. "rookie/atk_fail_2_e_??.png"),
      hotspot = {23, 23},
      fps = 10
   },
   atk_ok_2_w = {
      pictures = path.list_files (dirname .. "rookie/atk_ok_2_w_??.png"),
      hotspot = {-1, 23},
      fps = 10
   },
   atk_fail_2_w = {
      pictures = path.list_files (dirname .. "rookie/atk_fail_2_w_??.png"),
      hotspot = {-1, 23},
      fps = 10
   },
   eva_ok_e = {
      pictures = path.list_files (dirname .. "rookie/eva_ok_e_??.png"),
      hotspot = {11, 23},
      fps = 10
   },
   eva_fail_e = {
      pictures = path.list_files (dirname .. "rookie/eva_fail_e_??.png"),
      hotspot = {12, 23},
      fps = 10
   },
   eva_ok_w = {
      pictures = path.list_files (dirname .. "rookie/eva_ok_w_??.png"),
      hotspot = {-1, 22},
      fps = 10
   },
   eva_fail_w = {
      pictures = path.list_files (dirname .. "rookie/eva_fail_w_??.png"),
      hotspot = {-1, 22},
      fps = 10
   },
   die_w = {
      pictures = path.list_files (dirname .. "rookie/die_w_??.png"),
      hotspot = {15, 25},
      fps = 10
   },
   die_e = {
      pictures = path.list_files (dirname .. "rookie/die_e_??.png"),
      hotspot = {12, 23},
      fps = 10
   },
   -- health upgrade only
   health_idle = {
      pictures = path.list_files (dirname .. "helm/idle_??.png"),
      hotspot = {20, 25},
      fps = 10
   },
   health_atk_ok_1_e = {
      pictures = path.list_files (dirname .. "helm/atk_ok_1_e_??.png"),
      hotspot = {23, 24},
      fps = 10
   },
   health_atk_fail_1_e = {
      pictures = path.list_files (dirname .. "helm/atk_fail_1_e_??.png"),
      hotspot = {23, 24},
      fps = 10
   },
   health_atk_ok_1_w = {
      pictures = path.list_files (dirname .. "helm/atk_ok_1_w_??.png"),
      hotspot = {-1, 24},
      fps = 10
   },
   health_atk_fail_1_w = {
      pictures = path.list_files (dirname .. "helm/atk_fail_1_w_??.png"),
      hotspot = {-1, 24},
      fps = 10
   },
   health_atk_ok_2_e = {
      pictures = path.list_files (dirname .. "helm/atk_ok_2_e_??.png"),
      hotspot = {23, 24},
      fps = 10
   },
   health_atk_fail_2_e = {
      pictures = path.list_files (dirname .. "helm/atk_fail_2_e_??.png"),
      hotspot = {23, 24},
      fps = 10
   },
   health_atk_ok_2_w = {
      pictures = path.list_files (dirname .. "helm/atk_ok_2_w_??.png"),
      hotspot = {-1, 24},
      fps = 10
   },
   health_atk_fail_2_w = {
      pictures = path.list_files (dirname .. "helm/atk_fail_2_w_??.png"),
      hotspot = {-1, 24},
      fps = 10
   },
   health_eva_ok_e = {
      pictures = path.list_files (dirname .. "helm/eva_ok_e_??.png"),
      hotspot = {11, 24},
      fps = 10
   },
   health_eva_fail_e = {
      pictures = path.list_files (dirname .. "helm/eva_fail_e_??.png"),
      hotspot = {12, 24},
      fps = 10
   },
   health_eva_ok_w = {
      pictures = path.list_files (dirname .. "helm/eva_ok_w_??.png"),
      hotspot = {-1, 24},
      fps = 10
   },
   health_eva_fail_w = {
      pictures = path.list_files (dirname .. "helm/eva_fail_w_??.png"),
      hotspot = {-1, 24},
      fps = 10
   },
   health_die_f_w = {
      pictures = path.list_files (dirname .. "helm/die_f_w_??.png"),
      hotspot = {-1, 24},
      fps = 10
   },
   health_die_f_e = {
      pictures = path.list_files (dirname .. "helm/die_f_e_??.png"),
      hotspot = {19, 24},
      fps = 10
   },
   health_die_b_w = {
      pictures = path.list_files (dirname .. "helm/die_b_w_??.png"),
      hotspot = {15, 24},
      fps = 10
   },
   health_die_b_e = {
      pictures = path.list_files (dirname .. "helm/die_b_e_??.png"),
      hotspot = {12, 24},
      fps = 10
   },
   -- health and attack upgrade
   hero_idle = {
      pictures = path.list_files (dirname .. "hero/idle_??.png"),
      hotspot = {20, 25},
      fps = 10
   },
   hero_atk_ok_1_e = {
      pictures = path.list_files (dirname .. "hero/atk_ok_1_e_??.png"),
      hotspot = {23, 25},
      fps = 10
   },
   hero_atk_fail_1_e = {
      pictures = path.list_files (dirname .. "hero/atk_fail_1_e_??.png"),
      hotspot = {22, 25},
      fps = 10
   },
   hero_atk_ok_1_w = {
      pictures = path.list_files (dirname .. "hero/atk_ok_1_w_??.png"),
      hotspot = {0, 25},
      fps = 10
   },
   hero_atk_fail_1_w = {
      pictures = path.list_files (dirname .. "hero/atk_fail_1_w_??.png"),
      hotspot = {0, 25},
      fps = 10
   },
   hero_atk_ok_2_e = {
      pictures = path.list_files (dirname .. "hero/atk_ok_2_e_??.png"),
      hotspot = {23, 25},
      fps = 10
   },
   hero_atk_fail_2_e = {
      pictures = path.list_files (dirname .. "hero/atk_fail_2_e_??.png"),
      hotspot = {23, 25},
      fps = 10
   },
   hero_atk_ok_2_w = {
      pictures = path.list_files (dirname .. "hero/atk_ok_2_w_??.png"),
      hotspot = {0, 25},
      fps = 10
   },
   hero_atk_fail_2_w = {
      pictures = path.list_files (dirname .. "hero/atk_fail_2_w_??.png"),
      hotspot = {0, 25},
      fps = 10
   },
   hero_eva_ok_e = {
      pictures = path.list_files (dirname .. "hero/eva_ok_e_??.png"),
      hotspot = {13, 25},
      fps = 10
   },
   hero_eva_fail_e = {
      pictures = path.list_files (dirname .. "hero/eva_fail_e_??.png"),
      hotspot = {13, 25},
      fps = 10
   },
   hero_eva_ok_w = {
      pictures = path.list_files (dirname .. "hero/eva_ok_w_??.png"),
      hotspot = {0, 25},
      fps = 10
   },
   hero_eva_fail_w = {
      pictures = path.list_files (dirname .. "hero/eva_fail_w_??.png"),
      hotspot = {0, 25},
      fps = 10
   },
   hero_die_w = {
      pictures = path.list_files (dirname .. "hero/die_w_??.png"),
      hotspot = {-1, 24},
      fps = 10
   },
   hero_die_e = {
      pictures = path.list_files (dirname .. "hero/die_e_??.png"),
      hotspot = {19, 24},
      fps = 10
   },
   -- sword upgrade only
   attack_idle = {
      pictures = path.list_files (dirname .. "sword/idle_??.png"),
      hotspot = {20, 25},
      fps = 10
   },
   attack_atk_ok_1_e = {
      pictures = path.list_files (dirname .. "sword/atk_ok_1_e_??.png"),
      hotspot = {23, 24},
      fps = 10
   },
   attack_atk_fail_1_e = {
      pictures = path.list_files (dirname .. "sword/atk_fail_1_e_??.png"),
      hotspot = {23, 24},
      fps = 10
   },
   attack_atk_ok_1_w = {
      pictures = path.list_files (dirname .. "sword/atk_ok_1_w_??.png"),
      hotspot = {0, 23},
      fps = 10
   },
   attack_atk_fail_1_w = {
      pictures = path.list_files (dirname .. "sword/atk_fail_1_w_??.png"),
      hotspot = {0, 23},
      fps = 10
   },
   attack_atk_ok_2_e = {
      pictures = path.list_files (dirname .. "sword/atk_ok_2_e_??.png"),
      hotspot = {23, 23},
      fps = 10
   },
   attack_atk_fail_2_e = {
      pictures = path.list_files (dirname .. "sword/atk_fail_2_e_??.png"),
      hotspot = {23, 23},
      fps = 10
   },
   attack_atk_ok_2_w = {
      pictures = path.list_files (dirname .. "sword/atk_ok_2_w_??.png"),
      hotspot = {0, 23},
      fps = 10
   },
   attack_atk_fail_2_w = {
      pictures = path.list_files (dirname .. "sword/atk_fail_2_w_??.png"),
      hotspot = {0, 23},
      fps = 10
   },
   attack_eva_ok_e = {
      pictures = path.list_files (dirname .. "sword/eva_ok_e_??.png"),
      hotspot = {13, 23},
      fps = 10
   },
   attack_eva_fail_e = {
      pictures = path.list_files (dirname .. "sword/eva_fail_e_??.png"),
      hotspot = {13, 23},
      fps = 10
   },
   attack_eva_ok_w = {
      pictures = path.list_files (dirname .. "sword/eva_ok_w_??.png"),
      hotspot = {0, 24},
      fps = 10
   },
   attack_eva_fail_w = {
      pictures = path.list_files (dirname .. "sword/eva_fail_w_??.png"),
      hotspot = {0, 23},
      fps = 10
   },
   attack_die_f_w = {
      pictures = path.list_files (dirname .. "sword/die_f_w_??.png"),
      hotspot = {-1, 21},
      fps = 10
   },
   attack_die_f_e = {
      pictures = path.list_files (dirname .. "sword/die_f_e_??.png"),
      hotspot = {19, 22},
      fps = 10
   },
   attack_die_b_w = {
      pictures = path.list_files (dirname .. "sword/die_b_w_??.png"),
      hotspot = {17, 25},
      fps = 10
   },
   attack_die_b_e = {
      pictures = path.list_files (dirname .. "sword/die_b_e_??.png"),
      hotspot = {13, 23},
      fps = 10
   },
}
add_walking_animations (animations, "walk", dirname, "rookie/walk", {7, 24}, 15)
add_walking_animations (animations, "health_walk", dirname, "helm/walk", {7, 25}, 15)
add_walking_animations (animations, "attack_walk", dirname, "sword/walk", {8, 24}, 15)
add_walking_animations (animations, "hero_walk", dirname, "hero/walk", {8, 25}, 15)

-- convenience definitions so we don´t have to repeat this table for every single entry below
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

tribes:new_soldier_type {
   msgctxt = "frisians_worker",
   name = "frisians_soldier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Soldier"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = animations,

   default_target_quantity = 10,

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

   aihints = {
      preciousness = {
         frisians = 5
      },
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
