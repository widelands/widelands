dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 9, 39 },
   },
   fishing = {
      pictures = path.list_files(dirname .. "fishing_??.png"),
      hotspot = { 9, 39 },
      fps = 10
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {10, 38}, 10)
add_directional_animation(animations, "walkload", dirname, "walk", {10, 38}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_fisher",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Fisher"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      fishing_rod = 1
   },

   programs = {
      fish = {
         "findspace=size:any radius:7 resource:fish",
         "walk=coords",
         "playsound=sound/fisher/fisher_throw_net 192",
         "mine=fish 1",
         "animate=fishing 10500",
         "playsound=sound/fisher/fisher_pull_net 192",
         "createware=fish",
         "return"
      }
   },

   animations = animations,
}
