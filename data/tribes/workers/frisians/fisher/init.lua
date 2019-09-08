dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {8, 25},
   },
   fishing = {
      pictures = path.list_files (dirname .. "fishing_??.png"),
      hotspot = { 9, 23 },
      fps = 10
   },
   release = {
      pictures = path.list_files (dirname .. "release_??.png"),
      hotspot = { 15, 20 },
      fps = 10
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {13, 25}, 15)
add_directional_animation(animations, "walkload", dirname, "walkload", {13, 26}, 15)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_fisher",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Fisher"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      fishing_net = 1
   },

   programs = {
      fish = {
         "findspace=size:any radius:8 resource:fish",
         "walk=coords",
         "mine=fish 1",
         "animate=fishing 3000",
         "createware=fish",
         "return"
      },
      fish_in_pond = {
         "findobject=attrib:pond_mature radius:8",
         "walk=object",
         "animate=fishing 3000",
         "callobject=fall_dry",
         "createware=fish",
         "return"
      },
      breed_in_pond = {
         "findobject=attrib:pond_dry radius:8",
         "walk=object",
         "animate=release 2000",
         "callobject=with_fish",
         "return"
      }
   },

   ware_hotspot = {0, 20},
   animations = animations,
}
