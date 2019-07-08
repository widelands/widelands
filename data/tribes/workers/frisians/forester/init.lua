dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {8, 23},
   },
   dig = {
      pictures = path.list_files (dirname .. "dig_??.png"),
      hotspot = { 15, 20 },
      fps = 20
   },
   planting = {
      pictures = path.list_files (dirname .. "plant_??.png"),
      hotspot = { 15, 18 },
      fps = 10
   },
   water = {
      pictures = path.list_files (dirname .. "water_??.png"),
      hotspot = { 14, 20 },
      fps = 10
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {11, 24}, 15)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_forester",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Forester"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:5 avoid:field saplingsearches:7",
         "walk=coords",
         "animate=dig 2000",
         "animate=planting 1000",
         "plant=attrib:tree_sapling",
         "animate=water 2000",
         "return"
      }
   },

   animations = animations,
}
