push_textdomain("tribes")

dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 16, 31 },
      fps = 5
   },
   atk_ok_e = {
      pictures = path.list_files(dirname .. "atk_ok_e_??.png"),
      hotspot = { 30, 36 },
      fps = 10
   },
   atk_fail_e = {
      pictures = path.list_files(dirname .. "atk_fail_e_??.png"),
      hotspot = { 30, 36 },
      fps = 10
   },
   eva_ok_e = {
      pictures = path.list_files(dirname .. "eva_ok_e_??.png"),
      hotspot = { 18, 34 },
      fps = 20
   },
   eva_fail_e = {
      pictures = path.list_files(dirname .. "eva_fail_e_??.png"),
      hotspot = { 30, 36 },
      fps = 10
   },
   atk_ok_w = {
      pictures = path.list_files(dirname .. "atk_ok_w_??.png"),
      hotspot = { 30, 36 },
      fps = 10
   },
   atk_fail_w = {
      pictures = path.list_files(dirname .. "atk_fail_w_??.png"),
      hotspot = { 30, 36 },
      fps = 10
   },
   eva_ok_w = {
      pictures = path.list_files(dirname .. "eva_ok_w_??.png"),
      hotspot = { 18, 34 },
      fps = 20
   },
   eva_fail_w = {
      pictures = path.list_files(dirname .. "eva_fail_w_??.png"),
      hotspot = { 30, 36 },
      fps = 10
   },
   die_w = {
      pictures = path.list_files(dirname .. "die_??.png"),
      hotspot = { 16, 31 },
      fps = 20
   },
   die_e = {
      pictures = path.list_files(dirname .. "die_??.png"),
      hotspot = { 16, 31 },
      fps = 20
   },
   walk = {
      hotspot = { 16, 31 },
      fps = 10,
      directional = true
   },
}

all_levels_bar = {
   min_health = 0,
   min_attack = 0,
   min_defense = 0,
   min_evade = 0,
   max_health = 3,
   max_attack = 5,
   max_defense = 0,
   max_evade = 2,
}

tribes:new_soldier_type {
   name = "barbarians_soldier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Soldier"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   animation_directory = dirname,
   animations = animations,

   -- Battle attributes - initial values and per level increase
   health = {
      max_level = 3,
      base = 13000,
      increase_per_level = 3000,
      pictures = path.list_files(dirname .. "health_level?.png"),
   },
   attack = {
      max_level = 5,
      base = 1200,
      maximum = 1600,
      increase_per_level = 850,
      pictures = path.list_files(dirname .. "attack_level?.png"),
   },
   defense = {
      max_level = 0,
      base = 3,
      increase_per_level = 4,
      pictures = path.list_files(dirname .. "defense_level?.png"),
   },
   evade = {
      max_level = 2,
      base = 25,
      increase_per_level = 16,
      pictures = path.list_files(dirname .. "evade_level?.png"),
   },

   -- Random animations for battle
   attack_success_w = {
      atk_ok_w = all_levels_bar,
   },
   attack_success_e = {
      atk_ok_e = all_levels_bar,
   },
   attack_failure_w = {
      atk_fail_w = all_levels_bar,
   },
   attack_failure_e = {
      atk_fail_e = all_levels_bar,
   },
   evade_success_w = {
      eva_ok_w = all_levels_bar,
   },
   evade_success_e = {
      eva_ok_e = all_levels_bar,
   },
   evade_failure_w = {
      eva_fail_w = all_levels_bar,
   },
   evade_failure_e = {
      eva_fail_e = all_levels_bar,
   },
   die_w = {
      die_w = all_levels_bar,
   },
   die_e = {
      die_e = all_levels_bar,
   },
   idle = {
      idle = all_levels_bar,
   },
   walk = {
      {
         range = all_levels_bar,
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
