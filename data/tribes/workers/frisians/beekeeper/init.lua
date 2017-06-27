dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 9, 21 },
      fps = 10
   },
   beeswarm = {
      pictures = path.list_files(dirname .. "beeswarm_??.png"),
      hotspot = { 11, 18 },
      fps = 10
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {9, 21}, 10)
add_walking_animations(animations, "walkload", dirname, "walk", {9, 21}, 10)


tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_beekeeper",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("frisians_worker", "Bee-keeper"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1
   },

   programs = {
      bees = {
         "findobject attrib:flowering radius:7",
         "walk object",
         "animation beeswarm 4000",
         "createware honey",
         "return"
      }
   },

   animations = animations,
}
