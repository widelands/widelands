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
add_worker_animations(animations, "walk", dirname, "walk", {20, 34}, 10)


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
