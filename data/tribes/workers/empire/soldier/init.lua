dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 10, 36 },
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
      hotspot = { 30, 36 },
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
      hotspot = { 30, 36 },
      fps = 20
   },
   eva_fail_w = {
      pictures = path.list_files(dirname .. "eva_fail_w_??.png"),
      hotspot = { 30, 36 },
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
add_walking_animations(animations, "walk", dirname, "walk", {10, 36}, 10)

l_all = {
   min_health = 0,
   min_attack = 0,
   min_defense = 0,
   min_evade = 0,
   max_health = 4,
   max_attack = 4,
   max_defense = 0,
   max_evade = 2,
}

tribes:new_soldier_type {
   msgctxt = "empire_worker",
   name = "empire_soldier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Soldier"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = animations,

   default_target_quantity = 10,

   -- Battle attributes - initial values and per level increase
   health = {
      max_level = 4,
      base = 13000,
      increase_per_level = 2100,
      pictures = path.list_files(dirname .. "health_level?.png"),
   },
   attack = {
      max_level = 4,
      base = 1300,
      maximum = 1500,
      increase_per_level = 920,
      pictures = path.list_files(dirname .. "attack_level?.png"),
   },
   defense = {
      max_level = 0,
      base = 5,
      increase_per_level = 5,
      pictures = path.list_files(dirname .. "defense_level?.png"),
   },
   evade = {
      max_level = 2,
      base = 30,
      increase_per_level = 16,
      pictures = path.list_files(dirname .. "evade_level?.png"),
   },

   -- Random animations for battle
   attack_success_w = {
      atk_ok_w = l_all,
   },
   attack_success_e = {
      atk_ok_e = l_all,
   },
   attack_failure_w = {
      atk_fail_w = l_all,
   },
   attack_failure_e = {
      atk_fail_e = l_all,
   },
   evade_success_w = {
      eva_ok_w = l_all,
   },
   evade_success_e = {
      eva_ok_e = l_all,
   },
   evade_failure_w = {
      eva_fail_w = l_all,
   },
   evade_failure_e = {
      eva_fail_e = l_all,
   },
   die_w = {
      die_w = l_all,
   },
   die_e = {
      die_e = l_all,
   },
   idle = {
      idle = l_all,
   },
   walk_e = {
      walk_e = l_all,
   },
   walk_w = {
      walk_w = l_all,
   },
   walk_sw = {
      walk_sw = l_all,
   },
   walk_nw = {
      walk_nw = l_all,
   },
   walk_se = {
      walk_se = l_all,
   },
   walk_ne = {
      walk_ne = l_all,
   },
}
