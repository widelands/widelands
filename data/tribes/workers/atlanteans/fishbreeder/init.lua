dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 8, 22 }
   },
   freeing = {
      pictures = path.list_files(dirname .. "freeing_??.png"),
      hotspot = { 10, 19 },
      fps = 10
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {11, 23}, 20)

tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_fishbreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Fish Breeder"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      buckets = 1
   },

   programs = {
      breed = {
         "findspace size:any radius:7 breed resource:fish",
         "walk coords",
         "animation freeing 3000", -- Play a freeing animation
         "breed fish 1",
         "return"
      }
   },

   animations = animations,
}
