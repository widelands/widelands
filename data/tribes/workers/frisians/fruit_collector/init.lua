dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {8, 23},
      fps = 10
   },
   gather = {
      pictures = path.list_files (dirname .. "harvest_??.png"),
      hotspot = { 13, 21 },
      fps = 10
   }
}
add_walking_animations (animations, "walk", dirname, "walk", {11, 24}, 15)
add_walking_animations (animations, "walkload", dirname, "walkload", {10, 26}, 15)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_fruit_collector",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Fruit Collector"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      basket = 1
   },

   programs = {
      harvest = {
         "findobject=attrib:ripe_bush radius:6",
         "walk=object",
         "animate=gather 8000",
         "callobject=harvest",
         "animate=idle 1000",
         "createware=fruit",
         "return"
      }
   },

   ware_hotspot = {0, 20},
   animations = animations,
}
