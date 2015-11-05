dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 10, 36 },
      fps = 5
   },
   atk_ok_e = {
      template = "atk_ok_e_??",
      directory = dirname,
      hotspot = { 36, 40 },
      fps = 10
   },
   atk_fail_e = {
      template = "atk_fail_e_??",
      directory = dirname,
      hotspot = { 36, 40 },
      fps = 10
   },
   eva_ok_e = {
      template = "eva_ok_e_??",
      directory = dirname,
      hotspot = { 36, 40 },
      fps = 20
   },
   eva_fail_e = {
      template = "eva_fail_e_??",
      directory = dirname,
      hotspot = { 36, 40 },
      fps = 10
   },
   atk_ok_w = {
      template = "atk_ok_w_??",
      directory = dirname,
      hotspot = { 36, 40 },
      fps = 10
   },
   atk_fail_w = {
      template = "atk_fail_w_??",
      directory = dirname,
      hotspot = { 36, 40 },
      fps = 10
   },
   eva_ok_w = {
      template = "eva_ok_w_??",
      directory = dirname,
      hotspot = { 36, 40 },
      fps = 20
   },
   eva_fail_w = {
      template = "eva_fail_w_??",
      directory = dirname,
      hotspot = { 36, 40 },
      fps = 10
   },
   die_w = {
      template = "die_??",
      directory = dirname,
      hotspot = { 10, 36 },
      fps = 10
   },
   die_e = {
      template = "die_??",
      directory = dirname,
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

   buildcost = {
      atlanteans_carrier = 1,
      tabard = 1,
      trident_light = 1
   },

   animations = animations,

   max_hp_level = 1,
   max_attack_level = 4,
   max_defense_level = 2,
   max_evade_level = 2,

   -- Initial values and per level increments.
   hp = 13500,
   hp_incr_per_level = 4000,
   attack = {
      minimum = 1200,
      maximum = 1600
   },
   attack_incr_per_level = 800,
   defense = 6,
   defense_incr_per_level = 8,
   evade = 30,
   evade_incr_per_level = 17,

   hp_level_0_pic = dirname .. "hp_level0.png",
   hp_level_1_pic = dirname .. "hp_level1.png",
   evade_level_0_pic = dirname .. "evade_level0.png",
   evade_level_1_pic = dirname .. "evade_level1.png",
   evade_level_2_pic = dirname .. "evade_level2.png",
   attack_level_0_pic = dirname .. "attack_level0.png",
   attack_level_1_pic = dirname .. "attack_level1.png",
   attack_level_2_pic = dirname .. "attack_level2.png",
   attack_level_3_pic = dirname .. "attack_level3.png",
   attack_level_4_pic = dirname .. "attack_level4.png",
   defense_level_0_pic = dirname .. "defense_level0.png",
   defense_level_1_pic = dirname .. "defense_level1.png",
   defense_level_2_pic = dirname .. "defense_level2.png",

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
