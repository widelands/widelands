dirname = path.dirname (__file__)

-- Quick overview: We use 4 sets of animations here.
-- The default set (l__, "rookie") is for soldiers with no helmet and one sword.
-- The set prefixed "h" (l_h, "helm") is for soldiers with a helmet and one sword.
-- The set prefixed "s" (l_s, "sword") is for soldiers with no helmet and two swords.
-- The set prefixed "sh" (l_sh, "hero") is for soldiers with a helmet and two swords.

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
   h_idle = {
      pictures = path.list_files (dirname .. "helm/idle_??.png"),
      hotspot = {20, 25},
      fps = 10
   },
   h_atk_ok_1_e = {
      pictures = path.list_files (dirname .. "helm/atk_ok_1_e_??.png"),
      hotspot = {23, 24},
      fps = 10
   },
   h_atk_fail_1_e = {
      pictures = path.list_files (dirname .. "helm/atk_fail_1_e_??.png"),
      hotspot = {23, 24},
      fps = 10
   },
   h_atk_ok_1_w = {
      pictures = path.list_files (dirname .. "helm/atk_ok_1_w_??.png"),
      hotspot = {-1, 24},
      fps = 10
   },
   h_atk_fail_1_w = {
      pictures = path.list_files (dirname .. "helm/atk_fail_1_w_??.png"),
      hotspot = {-1, 24},
      fps = 10
   },
   h_atk_ok_2_e = {
      pictures = path.list_files (dirname .. "helm/atk_ok_2_e_??.png"),
      hotspot = {23, 24},
      fps = 10
   },
   h_atk_fail_2_e = {
      pictures = path.list_files (dirname .. "helm/atk_fail_2_e_??.png"),
      hotspot = {23, 24},
      fps = 10
   },
   h_atk_ok_2_w = {
      pictures = path.list_files (dirname .. "helm/atk_ok_2_w_??.png"),
      hotspot = {-1, 24},
      fps = 10
   },
   h_atk_fail_2_w = {
      pictures = path.list_files (dirname .. "helm/atk_fail_2_w_??.png"),
      hotspot = {-1, 24},
      fps = 10
   },
   h_eva_ok_e = {
      pictures = path.list_files (dirname .. "helm/eva_ok_e_??.png"),
      hotspot = {11, 24},
      fps = 10
   },
   h_eva_fail_e = {
      pictures = path.list_files (dirname .. "helm/eva_fail_e_??.png"),
      hotspot = {12, 24},
      fps = 10
   },
   h_eva_ok_w = {
      pictures = path.list_files (dirname .. "helm/eva_ok_w_??.png"),
      hotspot = {-1, 24},
      fps = 10
   },
   h_eva_fail_w = {
      pictures = path.list_files (dirname .. "helm/eva_fail_w_??.png"),
      hotspot = {-1, 24},
      fps = 10
   },
   h_die_f_w = {
      pictures = path.list_files (dirname .. "helm/die_f_w_??.png"),
      hotspot = {-1, 24},
      fps = 10
   },
   h_die_f_e = {
      pictures = path.list_files (dirname .. "helm/die_f_e_??.png"),
      hotspot = {19, 24},
      fps = 10
   },
   h_die_b_w = {
      pictures = path.list_files (dirname .. "helm/die_b_w_??.png"),
      hotspot = {15, 24},
      fps = 10
   },
   h_die_b_e = {
      pictures = path.list_files (dirname .. "helm/die_b_e_??.png"),
      hotspot = {12, 24},
      fps = 10
   },
   -- health and attack upgrade
   sh_idle = {
      pictures = path.list_files (dirname .. "hero/idle_??.png"),
      hotspot = {20, 25},
      fps = 10
   },
   sh_atk_ok_1_e = {
      pictures = path.list_files (dirname .. "hero/atk_ok_1_e_??.png"),
      hotspot = {23, 25},
      fps = 10
   },
   sh_atk_fail_1_e = {
      pictures = path.list_files (dirname .. "hero/atk_fail_1_e_??.png"),
      hotspot = {22, 25},
      fps = 10
   },
   sh_atk_ok_1_w = {
      pictures = path.list_files (dirname .. "hero/atk_ok_1_w_??.png"),
      hotspot = {0, 25},
      fps = 10
   },
   sh_atk_fail_1_w = {
      pictures = path.list_files (dirname .. "hero/atk_fail_1_w_??.png"),
      hotspot = {0, 25},
      fps = 10
   },
   sh_atk_ok_2_e = {
      pictures = path.list_files (dirname .. "hero/atk_ok_2_e_??.png"),
      hotspot = {23, 25},
      fps = 10
   },
   sh_atk_fail_2_e = {
      pictures = path.list_files (dirname .. "hero/atk_fail_2_e_??.png"),
      hotspot = {23, 25},
      fps = 10
   },
   sh_atk_ok_2_w = {
      pictures = path.list_files (dirname .. "hero/atk_ok_2_w_??.png"),
      hotspot = {0, 25},
      fps = 10
   },
   sh_atk_fail_2_w = {
      pictures = path.list_files (dirname .. "hero/atk_fail_2_w_??.png"),
      hotspot = {0, 25},
      fps = 10
   },
   sh_eva_ok_e = {
      pictures = path.list_files (dirname .. "hero/eva_ok_e_??.png"),
      hotspot = {13, 25},
      fps = 10
   },
   sh_eva_fail_e = {
      pictures = path.list_files (dirname .. "hero/eva_fail_e_??.png"),
      hotspot = {13, 25},
      fps = 10
   },
   sh_eva_ok_w = {
      pictures = path.list_files (dirname .. "hero/eva_ok_w_??.png"),
      hotspot = {0, 25},
      fps = 10
   },
   sh_eva_fail_w = {
      pictures = path.list_files (dirname .. "hero/eva_fail_w_??.png"),
      hotspot = {0, 25},
      fps = 10
   },
   sh_die_w = {
      pictures = path.list_files (dirname .. "hero/die_w_??.png"),
      hotspot = {-1, 24},
      fps = 10
   },
   sh_die_e = {
      pictures = path.list_files (dirname .. "hero/die_e_??.png"),
      hotspot = {19, 24},
      fps = 10
   },
   -- sword upgrade only
   s_idle = {
      pictures = path.list_files (dirname .. "sword/idle_??.png"),
      hotspot = {20, 25},
      fps = 10
   },
   s_atk_ok_1_e = {
      pictures = path.list_files (dirname .. "sword/atk_ok_1_e_??.png"),
      hotspot = {23, 24},
      fps = 10
   },
   s_atk_fail_1_e = {
      pictures = path.list_files (dirname .. "sword/atk_fail_1_e_??.png"),
      hotspot = {23, 24},
      fps = 10
   },
   s_atk_ok_1_w = {
      pictures = path.list_files (dirname .. "sword/atk_ok_1_w_??.png"),
      hotspot = {0, 23},
      fps = 10
   },
   s_atk_fail_1_w = {
      pictures = path.list_files (dirname .. "sword/atk_fail_1_w_??.png"),
      hotspot = {0, 23},
      fps = 10
   },
   s_atk_ok_2_e = {
      pictures = path.list_files (dirname .. "sword/atk_ok_2_e_??.png"),
      hotspot = {23, 23},
      fps = 10
   },
   s_atk_fail_2_e = {
      pictures = path.list_files (dirname .. "sword/atk_fail_2_e_??.png"),
      hotspot = {23, 23},
      fps = 10
   },
   s_atk_ok_2_w = {
      pictures = path.list_files (dirname .. "sword/atk_ok_2_w_??.png"),
      hotspot = {0, 23},
      fps = 10
   },
   s_atk_fail_2_w = {
      pictures = path.list_files (dirname .. "sword/atk_fail_2_w_??.png"),
      hotspot = {0, 23},
      fps = 10
   },
   s_eva_ok_e = {
      pictures = path.list_files (dirname .. "sword/eva_ok_e_??.png"),
      hotspot = {13, 23},
      fps = 10
   },
   s_eva_fail_e = {
      pictures = path.list_files (dirname .. "sword/eva_fail_e_??.png"),
      hotspot = {13, 23},
      fps = 10
   },
   s_eva_ok_w = {
      pictures = path.list_files (dirname .. "sword/eva_ok_w_??.png"),
      hotspot = {0, 24},
      fps = 10
   },
   s_eva_fail_w = {
      pictures = path.list_files (dirname .. "sword/eva_fail_w_??.png"),
      hotspot = {0, 23},
      fps = 10
   },
   s_die_f_w = {
      pictures = path.list_files (dirname .. "sword/die_f_w_??.png"),
      hotspot = {-1, 21},
      fps = 10
   },
   s_die_f_e = {
      pictures = path.list_files (dirname .. "sword/die_f_e_??.png"),
      hotspot = {19, 22},
      fps = 10
   },
   s_die_b_w = {
      pictures = path.list_files (dirname .. "sword/die_b_w_??.png"),
      hotspot = {17, 25},
      fps = 10
   },
   s_die_b_e = {
      pictures = path.list_files (dirname .. "sword/die_b_e_??.png"),
      hotspot = {13, 23},
      fps = 10
   },
}
add_walking_animations (animations, "walk", dirname, "rookie/walk", {7, 24}, 15)
add_walking_animations (animations, "h_walk", dirname, "helm/walk", {7, 25}, 15)
add_walking_animations (animations, "s_walk", dirname, "sword/walk", {8, 24}, 15)
add_walking_animations (animations, "sh_walk", dirname, "hero/walk", {8, 25}, 15)

-- convenience definitions so we don´t have to repeat this table for every single entry below
l_h = {
   min_health = 1,
   min_attack = 0,
   min_defense = 0,
   min_evade = 0,
   max_health = 2,
   max_attack = 3,
   max_defense = 2,
   max_evade = 0,
}
l_s = {
   min_health = 0,
   min_attack = 4,
   min_defense = 0,
   min_evade = 0,
   max_health = 0,
   max_attack = 6,
   max_defense = 2,
   max_evade = 0,
}
l_sh = {
   min_health = 1,
   min_attack = 4,
   min_defense = 0,
   min_evade = 0,
   max_health = 2,
   max_attack = 6,
   max_defense = 2,
   max_evade = 0,
}
l__ = {
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

   -- Random animations for battle
   attack_success_e = {
      atk_ok_1_w = l__,
      atk_ok_2_w = l__,
      h_atk_ok_1_w = l_h,
      h_atk_ok_2_w = l_h,
      s_atk_ok_1_w = l_s,
      s_atk_ok_2_w = l_s,
      sh_atk_ok_1_w = l_sh,
      sh_atk_ok_2_w = l_sh,
   },
   attack_success_w = {
      atk_ok_1_e = l__,
      atk_ok_2_e = l__,
      h_atk_ok_1_e = l_h,
      h_atk_ok_2_e = l_h,
      s_atk_ok_1_e = l_s,
      s_atk_ok_2_e = l_s,
      sh_atk_ok_1_e = l_sh,
      sh_atk_ok_2_e = l_sh,
   },
   attack_failure_e = {
      atk_fail_1_w = l__,
      atk_fail_2_w = l__,
      h_atk_fail_1_w = l_h,
      h_atk_fail_2_w = l_h,
      s_atk_fail_1_w = l_s,
      s_atk_fail_2_w = l_s,
      sh_atk_fail_1_w = l_sh,
      sh_atk_fail_2_w = l_sh,
   },
   attack_failure_w = {
      atk_fail_1_e = l__,
      atk_fail_2_e = l__,
      h_atk_fail_1_e = l_h,
      h_atk_fail_2_e = l_h,
      s_atk_fail_1_e = l_s,
      s_atk_fail_2_e = l_s,
      sh_atk_fail_1_e = l_sh,
      sh_atk_fail_2_e = l_sh,
   },
   evade_failure_e = {
      h_eva_fail_w = l_h,
      sh_eva_fail_w = l_sh,
      eva_fail_w = l__,
      s_eva_fail_w = l_s,
   },
   evade_failure_w = {
      h_eva_fail_e = l_h,
      sh_eva_fail_e = l_sh,
      eva_fail_e = l__,
      s_eva_fail_e = l_s,
   },
   evade_success_e = {
      h_eva_ok_w = l_h,
      sh_eva_ok_w = l_sh,
      eva_ok_w = l__,
      s_eva_ok_w = l_s,
   },
   evade_success_w = {
      h_eva_ok_e = l_h,
      sh_eva_ok_e = l_sh,
      eva_ok_e = l__,
      s_eva_ok_e = l_s,
   },
   die_w = {
      h_die_f_w = l_h,
      h_die_b_w = l_h,
      sh_die_w = l_sh,
      die_w = l__,
      s_die_f_w = l_s,
      s_die_b_w = l_s,
   },
   die_e = {
      h_die_f_e = l_h,
      h_die_b_e = l_h,
      sh_die_e = l_sh,
      die_e = l__,
      s_die_f_e = l_s,
      s_die_b_e = l_s,
   },
   -- level-dependent walking and idle animations
   idle = {
      h_idle = l_h,
      sh_idle = l_sh,
      idle = l__,
      s_idle = l_s,
   },
   walk_sw = {
      h_walk_sw = l_h,
      sh_walk_sw = l_sh,
      walk_sw = l__,
      s_walk_sw = l_s,
   },
   walk_nw = {
      h_walk_nw = l_h,
      sh_walk_nw = l_sh,
      walk_nw = l__,
      s_walk_nw = l_s,
   },
   walk_w = {
      h_walk_w = l_h,
      sh_walk_w = l_sh,
      walk_w = l__,
      s_walk_w = l_s,
   },
   walk_se = {
      h_walk_se = l_h,
      sh_walk_se = l_sh,
      walk_se = l__,
      s_walk_se = l_s,
   },
   walk_ne = {
      h_walk_ne = l_h,
      sh_walk_ne = l_sh,
      walk_ne = l__,
      s_walk_ne = l_s,
   },
   walk_e = {
      h_walk_e = l_h,
      sh_walk_e = l_sh,
      walk_e = l__,
      s_walk_e = l_s,
   },
}
