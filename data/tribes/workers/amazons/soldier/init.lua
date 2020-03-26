dirname = path.dirname (__file__)

animations = {
   -- normal
   idle = {
      pictures = path.list_files (dirname .. "rookie/idle_??.png"),
      hotspot = {20, 24},
      fps = 10
   },
   atk_ok_e = {
      pictures = path.list_files (dirname .. "rookie/atk_ok_e_??.png"),
      hotspot = {23, 24},
      fps = 10
   },
   atk_fail_e = {
      pictures = path.list_files (dirname .. "rookie/atk_fail_e_??.png"),
      hotspot = {23, 23},
      fps = 10
   },
   atk_ok_w = {
      pictures = path.list_files (dirname .. "rookie/atk_ok_w_??.png"),
      hotspot = {-1, 22},
      fps = 10
   },
   atk_fail_w = {
      pictures = path.list_files (dirname .. "rookie/atk_fail_w_??.png"),
      hotspot = {-1, 22},
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

}
add_directional_animation(animations, "walk", dirname .. "rookie", "walk", {7, 24}, 15)


-- convenience definitions so we don´t have to repeat this table for every single entry below
all_levels_ama = {
   min_health = 0,
   min_attack = 0,
   min_defense = 0,
   min_evade = 0,
   max_health = 3,
   max_attack = 2,
   max_defense = 2,
   max_evade = 3,
}

tribes:new_soldier_type {
   msgctxt = "amazons_worker",
   name = "amazons_soldier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Soldier"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   animation_directory = dirname .. "rookie",
   animations = animations,

   default_target_quantity = 10,

   -- Battle attributes - initial values and per level increase
   health = {
      max_level = 3,
      base = 13000,
      increase_per_level = 2025,
      pictures = path.list_files (dirname .. "health_level?.png"),
   },
   attack = {
      max_level = 2,
      base = 1200,
      maximum = 1600,
      increase_per_level = 800,
      pictures = path.list_files (dirname .. "attack_level?.png"),
   },
   defense = {
      max_level = 2,
      base = 5,
      increase_per_level = 10,
      pictures = path.list_files (dirname .. "defense_level?.png"),
   },
   evade = {
      max_level = 3,
      base = 30,
      increase_per_level = 15,
      pictures = path.list_files (dirname .. "evade_level?.png"),
   },

   aihints = {
      preciousness = {
         amazons = 5
      },
   },

   -- Random animations for battle
   attack_success_w = {
      atk_ok_w = all_levels_ama,
   },
   attack_success_e = {
      atk_ok_e = all_levels_ama,
   },
   attack_failure_w = {
      atk_fail_w = all_levels_ama,
   },
   attack_failure_e = {
      atk_fail_e = all_levels_ama,
   },
   evade_success_w = {
      eva_ok_w = all_levels_ama,
   },
   evade_success_e = {
      eva_ok_e = all_levels_ama,
   },
   evade_failure_w = {
      eva_fail_w = all_levels_ama,
   },
   evade_failure_e = {
      eva_fail_e = all_levels_ama,
   },
   die_w = {
      die_w = all_levels_ama,
   },
   die_e = {
      die_e = all_levels_ama,
   },
   idle = {
      idle = all_levels_ama,
   },
   walk = {
      {
         range = all_levels_ama,
         sw = "walk_sw",
         se = "walk_se",
         nw = "walk_nw",
         ne = "walk_ne",
         w = "walk_w",
         e = "walk_e",
      },
   },
}
