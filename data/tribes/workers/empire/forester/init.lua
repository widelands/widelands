dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 3, 23 }
   },
   dig = {
      pictures = path.list_files(dirname .. "dig_??.png"),
      hotspot = { 5, 22 },
      fps = 5
   },
   planting = {
      pictures = path.list_files(dirname .. "plant_??.png"),
      hotspot = { 18, 23 },
      fps = 10
   },
   water = {
      pictures = path.list_files(dirname .. "water_??.png"),
      hotspot = { 18, 26 },
      fps = 5
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {10, 23}, 10)
add_directional_animation(animations, "walkload", dirname, "walk", {10, 23}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_forester",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Forester"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:5 avoid:field saplingsearches:8",
         "walk=coords",
         "animate=dig 2500",
         "animate=planting 1500",
         "plant=attrib:tree_sapling",
         "animate=water 2000",
         "return"
      }
   },

   animations = animations,
}
