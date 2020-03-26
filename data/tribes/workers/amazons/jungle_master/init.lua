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
   msgctxt = "amazons_worker",
   name = "amazons_jungle_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Jungle Master"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   programs = {
      plant_ironwood = {
         "findspace=size:any radius:5 avoid:field saplingsearches:7",
         "walk=coords",
         "animate=dig 2000",
         "animate=planting 1000",
         "plant=attrib:tree_ironwood_sapling",
         "animate=planting 1000",
         "return"
      },
      plant_rubber = {
         "findspace=size:any radius:5 avoid:field saplingsearches:7",
         "walk=coords",
         "animate=dig 2000",
         "animate=planting 1000",
         "plant=attrib:tree_rubber_sapling",
         "animate=planting 1000",
         "return"
      },
      plant_balsa = {
         "findspace=size:any radius:5 avoid:field saplingsearches:7",
         "walk=coords",
         "animate=dig 2000",
         "animate=planting 1000",
         "plant=attrib:tree_balsa_sapling",
         "animate=planting 1000",
         "return"
      },
      check = {
         "findspace=size:any radius:6 terraform",
      },
      terraform = {
         "findspace=size:any radius:6 terraform",
         "walk=coords",
         "terraform",
         "return"
      },
      plant = {
         "findspace=size:any radius:6 avoid:field saplingsearches:7",
         "walk=coords",
         "animate=dig 2000",
         "animate=planting 1000",
         "plant=attrib:tree_pole",
         "animate=planting 1000",
         "return"
      }
   },

   animation_directory = dirname,
   ware_hotspot = {0, 29},
   animations = animations,
}
