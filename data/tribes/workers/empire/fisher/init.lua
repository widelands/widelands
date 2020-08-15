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
add_directional_animation(animations, "walk", dirname, "walk", {10, 38}, 20)
add_directional_animation(animations, "walkload", dirname, "walk", {10, 38}, 20)


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
         "findspace=size:any radius:7 resource:fish",
         "walk=coords",
         "playsound=sound/fisher/fisher_throw_net priority:50% allow_multiple",
         "mine=fish radius:1",
         "animate=fishing duration:10s", -- Play a fishing animation
         "playsound=sound/fisher/fisher_pull_net priority:50% allow_multiple",
         "createware=fish",
         "return"
      }
   },

   animations = animations,
}
