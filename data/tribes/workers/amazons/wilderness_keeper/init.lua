dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {8, 23},
   },
   release = {
      pictures = path.list_files (dirname .. "release_??.png"),
      hotspot = { 26, 31 },
      fps = 50
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {11, 24}, 15)
add_directional_animation(animations, "walkload", dirname, "walkload", {10, 26}, 15)

tribes:new_worker_type {
   msgctxt = "amazons_worker",
   name = "amazons_wilderness_keeper",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Wildernesskeeper"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1
   },

   programs = {
      release_game = {
         "findobject=attrib:tree radius:10",
         "walk=object",
         "animate=release 4000",
         "createbob=bunny badger fox lynx marten wildboar",
         "return"
      },
      breed_fish = {
         "findspace=size:swim radius:7",
         "walk=coords",
         "animate=release 13500", -- Play a freeing animation
         "breed=fish 1",
         "return"
      }
   },

   ware_hotspot = {0, 20},
   animations = animations,
}
