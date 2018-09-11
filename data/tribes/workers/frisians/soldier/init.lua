dirname = path.dirname (__file__)

-- Quick overview: We use 4 sets of animations here.
-- The default set is for soldiers with no helmet and one sword.
-- The set prefixed "h" is for soldiers with a helmet and one sword.
-- The set prefixed "s" is for soldiers with no helmet and two swords.
-- The set prefixed "sh" is for soldiers with a Helmet and two swords.
animations = {
   idle = {
      pictures = path.list_files (dirname .. "h_idle_??.png"),
      hotspot = {9, 25},
      fps = 10
   },
   atk_ok_e = {
      pictures = path.list_files (dirname .. "h_atk_ok_w_??.png"),
      hotspot = { 5, 26 },
      fps = 10
   },
   atk_fail_e = {
      pictures = path.list_files (dirname .. "h_atk_fail_w_??.png"),
      hotspot = { 5, 26 },
      fps = 10
   },
   atk_ok_w = {
      pictures = path.list_files (dirname .. "h_atk_ok_e_??.png"),
      hotspot = { 23, 26 },
      fps = 10
   },
   atk_fail_w = {
      pictures = path.list_files (dirname .. "h_atk_fail_e_??.png"),
      hotspot = { 23, 26 },
      fps = 10
   },
   eva_ok_e = {
      pictures = path.list_files (dirname .. "h_eva_ok_w_??.png"),
      hotspot = { 5, 26 },
      fps = 10
   },
   eva_fail_e = {
      pictures = path.list_files (dirname .. "h_eva_fail_w_??.png"),
      hotspot = { 5, 26 },
      fps = 10
   },
   eva_ok_w = {
      pictures = path.list_files (dirname .. "h_eva_ok_e_??.png"),
      hotspot = { 11, 26 },
      fps = 10
   },
   eva_fail_w = {
      pictures = path.list_files (dirname .. "h_eva_fail_e_??.png"),
      hotspot = { 12, 26 },
      fps = 10
   },
   die_w = {
      pictures = path.list_files (dirname .. "h_die_w_??.png"),
      hotspot = { 22, 27 },
      fps = 10
   },
   die_e = {
      pictures = path.list_files (dirname .. "h_die_e_??.png"),
      hotspot = { 13, 26 },
      fps = 10
   },
   h_idle = {
      pictures = path.list_files (dirname .. "h_idle_??.png"),
      hotspot = {9, 25},
      fps = 10
   },
   h_atk_ok_e = {
      pictures = path.list_files (dirname .. "h_atk_ok_w_??.png"),
      hotspot = { 5, 26 },
      fps = 10
   },
   h_atk_fail_e = {
      pictures = path.list_files (dirname .. "h_atk_fail_w_??.png"),
      hotspot = { 5, 26 },
      fps = 10
   },
   h_atk_ok_w = {
      pictures = path.list_files (dirname .. "h_atk_ok_e_??.png"),
      hotspot = { 23, 26 },
      fps = 10
   },
   h_atk_fail_w = {
      pictures = path.list_files (dirname .. "h_atk_fail_e_??.png"),
      hotspot = { 23, 26 },
      fps = 10
   },
   h_eva_ok_e = {
      pictures = path.list_files (dirname .. "h_eva_ok_w_??.png"),
      hotspot = { 5, 26 },
      fps = 10
   },
   h_eva_fail_e = {
      pictures = path.list_files (dirname .. "h_eva_fail_w_??.png"),
      hotspot = { 5, 26 },
      fps = 10
   },
   h_eva_ok_w = {
      pictures = path.list_files (dirname .. "h_eva_ok_e_??.png"),
      hotspot = { 11, 26 },
      fps = 10
   },
   h_eva_fail_w = {
      pictures = path.list_files (dirname .. "h_eva_fail_e_??.png"),
      hotspot = { 12, 26 },
      fps = 10
   },
   h_die_w = {
      pictures = path.list_files (dirname .. "h_die_w_??.png"),
      hotspot = { 22, 27 },
      fps = 10
   },
   h_die_e = {
      pictures = path.list_files (dirname .. "h_die_e_??.png"),
      hotspot = { 13, 26 },
      fps = 10
   },
   sh_idle = {
      pictures = path.list_files (dirname .. "h_idle_??.png"),
      hotspot = {9, 25},
      fps = 10
   },
   sh_atk_ok_e = {
      pictures = path.list_files (dirname .. "h_atk_ok_w_??.png"),
      hotspot = { 5, 26 },
      fps = 10
   },
   sh_atk_fail_e = {
      pictures = path.list_files (dirname .. "h_atk_fail_w_??.png"),
      hotspot = { 5, 26 },
      fps = 10
   },
   sh_atk_ok_w = {
      pictures = path.list_files (dirname .. "h_atk_ok_e_??.png"),
      hotspot = { 23, 26 },
      fps = 10
   },
   sh_atk_fail_w = {
      pictures = path.list_files (dirname .. "h_atk_fail_e_??.png"),
      hotspot = { 23, 26 },
      fps = 10
   },
   sh_eva_ok_e = {
      pictures = path.list_files (dirname .. "h_eva_ok_w_??.png"),
      hotspot = { 5, 26 },
      fps = 10
   },
   sh_eva_fail_e = {
      pictures = path.list_files (dirname .. "h_eva_fail_w_??.png"),
      hotspot = { 5, 26 },
      fps = 10
   },
   sh_eva_ok_w = {
      pictures = path.list_files (dirname .. "h_eva_ok_e_??.png"),
      hotspot = { 11, 26 },
      fps = 10
   },
   sh_eva_fail_w = {
      pictures = path.list_files (dirname .. "h_eva_fail_e_??.png"),
      hotspot = { 12, 26 },
      fps = 10
   },
   sh_die_w = {
      pictures = path.list_files (dirname .. "h_die_w_??.png"),
      hotspot = { 22, 27 },
      fps = 10
   },
   sh_die_e = {
      pictures = path.list_files (dirname .. "h_die_e_??.png"),
      hotspot = { 13, 26 },
      fps = 10
   },
   s_idle = {
      pictures = path.list_files (dirname .. "h_idle_??.png"),
      hotspot = {9, 25},
      fps = 10
   },
   s_atk_ok_e = {
      pictures = path.list_files (dirname .. "h_atk_ok_w_??.png"),
      hotspot = { 5, 26 },
      fps = 10
   },
   s_atk_fail_e = {
      pictures = path.list_files (dirname .. "h_atk_fail_w_??.png"),
      hotspot = { 5, 26 },
      fps = 10
   },
   s_atk_ok_w = {
      pictures = path.list_files (dirname .. "h_atk_ok_e_??.png"),
      hotspot = { 23, 26 },
      fps = 10
   },
   s_atk_fail_w = {
      pictures = path.list_files (dirname .. "h_atk_fail_e_??.png"),
      hotspot = { 23, 26 },
      fps = 10
   },
   s_eva_ok_e = {
      pictures = path.list_files (dirname .. "h_eva_ok_w_??.png"),
      hotspot = { 5, 26 },
      fps = 10
   },
   s_eva_fail_e = {
      pictures = path.list_files (dirname .. "h_eva_fail_w_??.png"),
      hotspot = { 5, 26 },
      fps = 10
   },
   s_eva_ok_w = {
      pictures = path.list_files (dirname .. "h_eva_ok_e_??.png"),
      hotspot = { 11, 26 },
      fps = 10
   },
   s_eva_fail_w = {
      pictures = path.list_files (dirname .. "h_eva_fail_e_??.png"),
      hotspot = { 12, 26 },
      fps = 10
   },
   s_die_w = {
      pictures = path.list_files (dirname .. "h_die_w_??.png"),
      hotspot = { 22, 27 },
      fps = 10
   },
   s_die_e = {
      pictures = path.list_files (dirname .. "h_die_e_??.png"),
      hotspot = { 13, 26 },
      fps = 10
   },
}
add_walking_animations (animations, "walk", dirname, "h_walk", {10, 25}, 15)
add_walking_animations (animations, "s_walk", dirname, "h_walk", {10, 25}, 15)
add_walking_animations (animations, "h_walk", dirname, "h_walk", {10, 25}, 15)
add_walking_animations (animations, "sh_walk", dirname, "h_walk", {10, 25}, 15)

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
      increase_per_level = 3250,
      pictures = path.list_files (dirname .. "health_level?.png"),
   },
   attack = {
      max_level = 6,
      base = 1350,
      maximum = 1550,
      increase_per_level = 975,
      pictures = path.list_files (dirname .. "attack_level?.png"),
   },
   defense = {
      max_level = 2,
      base = 4,
      increase_per_level = 15,
      pictures = path.list_files (dirname .. "defense_level?.png"),
   },
   evade = {
      max_level = 0,
      base = 35,
      increase_per_level = 0,
      pictures = path.list_files (dirname .. "evade_level?.png"),
   },

   -- Random animations for battle
   attack_success_w = {
      h_atk_ok_w = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_atk_ok_w = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      atk_ok_w = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_atk_ok_w = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
   attack_success_e = {
      h_atk_ok_e = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_atk_ok_e = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      atk_ok_e = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_atk_ok_e = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
   attack_failure_w = {
      h_atk_fail_w = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_atk_fail_w = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      atk_fail_w = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_atk_fail_w = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
   attack_failure_e = {
      h_atk_fail_e = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_atk_fail_e = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      atk_fail_e = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_atk_fail_e = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
   evade_failure_w = {
      h_eva_fail_w = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_eva_fail_w = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      eva_fail_w = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_eva_fail_w = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
   evade_failure_e = {
      h_eva_fail_e = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_eva_fail_e = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      eva_fail_e = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_eva_fail_e = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
   evade_success_w = {
      h_eva_ok_w = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_eva_ok_w = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      eva_ok_w = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_eva_ok_w = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
   evade_success_e = {
      h_eva_ok_e = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_eva_ok_e = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      eva_ok_e = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_eva_ok_e = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
   die_w = {
      h_die_w = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_die_w = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      die_w = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_die_w = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
   die_e = {
      h_die_e = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_die_e = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      die_e = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_die_e = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
   idle = {
      h_idle = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_idle = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      idle = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_idle = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
   walk_sw = {
      h_walk_sw = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_walk_sw = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      walk_sw = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_walk_sw = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
   walk_nw = {
      walk_nw = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_walk_nw = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      walk_nw = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_walk_nw = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
   walk_w = {
      h_walk_w = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_walk_w = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      walk_w = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_walk_w = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
   walk_se = {
      h_walk_se = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_walk_se = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      walk_se = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_walk_se = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
   walk_ne = {
      h_walk_ne = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_walk_ne = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      walk_ne = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_walk_ne = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
   walk_e = {
      h_walk_e = {
         min_health = 1,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 3,
         max_defense = 2,
         max_evade = 0,
      },
      sh_walk_e = {
         min_health = 1,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 2,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      walk_e = {
         min_health = 0,
         min_attack = 0,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
      s_walk_e = {
         min_health = 0,
         min_attack = 4,
         min_defense = 0,
         min_evade = 0,
         max_health = 0,
         max_attack = 6,
         max_defense = 2,
         max_evade = 0,
      },
   },
}
