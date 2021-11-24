push_textdomain("tribes")

dirname = path.dirname(__file__)

spritesheets = {
   atk_fail_e = {
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 3, 32 }
   },
   atk_fail_w = {
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 30, 36 }
   },
   atk_ok_e = {
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 6, 32 }
   },
   atk_ok_w = {
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 30, 36 }
   },
   die_e = {
      fps = 20,
      frames = 20,
      rows = 5,
      columns = 4,
      hotspot = { 12, 29 }
   },
   die_w = {
      fps = 20,
      frames = 20,
      rows = 5,
      columns = 4,
      hotspot = { 12, 29 }
   },
   eva_fail_e = {
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 6, 27 }
   },
   eva_fail_w = {
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      hotspot = { 9, 26 }
   },
   eva_ok_e = {
      fps = 20,
      frames = 20,
      rows = 5,
      columns = 4,
      hotspot = { 14, 28 }
   },
   eva_ok_w = {
      fps = 20,
      frames = 20,
      rows = 5,
      columns = 4,
      hotspot = { 13, 26 }
   },
   idle = {
      fps = 5,
      frames = 20,
      rows = 5,
      columns = 4,
      hotspot = { 6, 29 }
   },
   walk = {
      fps = 10,
      frames = 10,
      rows = 4,
      columns = 3,
      directional = true,
      hotspot = { 14, 29 }
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

descriptions:new_soldier_type {
   name = "barbarians_soldier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Soldier"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   animation_directory = dirname,
   spritesheets = spritesheets,

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
