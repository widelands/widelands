push_textdomain("tribes")

local dirname = path.dirname (__file__)

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

wl.Descriptions():new_soldier_type {
   name = "amazons_soldier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Soldier"),
   icon = dirname .. "menu.png",
   vision_range = 2,

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
      die_e = all_levels_ama,
   },
   die_e = {
      die_w = all_levels_ama,
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
   animation_directory = dirname,
   spritesheets = {
      walk = {
         directional = true,
         hotspot = {17, 28},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      idle = {
         hotspot = {17, 28},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      die_w = {
         hotspot = {16, 30},
         fps = 28,
         frames = 30,
         columns = 6,
         rows = 5
      },
      die_e = {
         hotspot = {11, 38},
         fps = 28,
         frames = 30,
         columns = 6,
         rows = 5
      },
      atk_fail_w = {
         hotspot = {33, 30},
         fps = 30,
         frames = 30,
         columns = 6,
         rows = 5
      },
      atk_fail_e = {
         hotspot = {11, 35},
         fps = 30,
         frames = 30,
         columns = 6,
         rows = 5
      },
      atk_ok_w = {
         hotspot = {33, 30},
         fps = 30,
         frames = 30,
         columns = 6,
         rows = 5
      },
      atk_ok_e = {
         hotspot = {11, 35},
         fps = 30,
         frames = 30,
         columns = 6,
         rows = 5
      },
      eva_fail_w = {
         hotspot = {8, 30},
         fps = 30,
         frames = 30,
         columns = 6,
         rows = 5
      },
      eva_fail_e = {
         hotspot = {12, 32},
         fps = 30,
         frames = 30,
         columns = 6,
         rows = 5
      },
      eva_ok_w = {
         hotspot = {8, 30},
         fps = 30,
         frames = 30,
         columns = 6,
         rows = 5
      },
      eva_ok_e = {
         hotspot = {23, 32},
         fps = 30,
         frames = 30,
         columns = 6,
         rows = 5
      },
   },
}

pop_textdomain()
