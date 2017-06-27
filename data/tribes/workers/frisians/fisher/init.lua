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
add_walking_animations(animations, "walk", dirname, "walk", {10, 38}, 10)
add_walking_animations(animations, "walkload", dirname, "walk", {10, 38}, 10)


tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_fisher",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("frisians_worker", "Fisher"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      fishing_net = 1
   },

   programs = {
      fish = {
         "findspace size:any radius:8 resource:fish",
         "walk coords",
         "mine fish 1",
         "animation fishing 3000",
         "createware fish",
         "return"
      }
   },

   animations = animations,
}
