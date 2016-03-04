dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 7, 38 },
   },
   fishing = {
      pictures = path.list_files(dirname .. "fishing_??.png"),
      hotspot = { 9, 39 },
      fps = 10,
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {10, 38}, 20)
add_worker_animations(animations, "walkload", dirname, "walk", {10, 38}, 20)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_fisher",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Fisher"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      fishing_rod = 1
   },

   programs = {
      fish = {
         "findspace size:any radius:7 resource:fish",
         "walk coords",
         "play_sound sound/fisher fisher_throw_net 192",
         "mine fish 1",
         "animation fishing 3000", -- Play a fishing animation
         "play_sound sound/fisher fisher_pull_net 192",
         "createware fish",
         "return"
      }
   },

   animations = animations,
}
