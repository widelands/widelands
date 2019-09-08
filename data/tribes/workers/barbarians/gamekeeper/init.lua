dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 14, 22 }
   },
   releasein = {
      pictures = path.list_files(dirname .. "releasein_??.png"),
      hotspot = { 15, 22 },
      fps = 5
   },
   releaseout = {
      pictures = path.list_files(dirname .. "releaseout_??.png"),
      hotspot = { 15, 22 },
      fps = 5
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {15, 22}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {15, 22})


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_gamekeeper",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Gamekeeper"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1
   },

   programs = {
      release = {
         "findspace=size:any radius:3",
         "walk=coords",
         "animate=releasein 2000",
         "createbob=bunny chamois deer elk reindeer sheep stag wildboar wisent",
         "animate=releaseout 2000",
         "return"
      }
   },

   animations = animations,
}
