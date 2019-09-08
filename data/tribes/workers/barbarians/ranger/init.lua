dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 11, 23 }
   },
   dig = {
      pictures = path.list_files(dirname .. "dig_??.png"),
      hotspot = { 12, 24 },
      fps = 5
   },
   planting = {
      pictures = path.list_files(dirname .. "plant_??.png"),
      hotspot = { 18, 24 },
      fps = 10
   },
   water = {
      pictures = path.list_files(dirname .. "water_??.png"),
      hotspot = { 19, 25 },
      fps = 5
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {11, 23}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {11, 23})


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_ranger",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Ranger"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:5 avoid:field saplingsearches:5",
         "walk=coords",
         "animate=dig 3500",
         "animate=planting 2500",
         "plant=attrib:tree_sapling",
         "animate=water 3500",
         "return"
      }
   },

   animations = animations,
}
