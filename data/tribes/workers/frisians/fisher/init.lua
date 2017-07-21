dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 9, 39 },
   },
   fishing = {
      pictures = path.list_files(dirname .. "fishing_??.png"),
      hotspot = { 9, 39 },
      fps = 10
   },
   release = {
      pictures = path.list_files(dirname .. "fishing_??.png"), --TODO no animation yet
      hotspot = { 9, 39 },
      fps = 10
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {10, 38}, 10)
add_walking_animations(animations, "walkload", dirname, "walk", {10, 38}, 10)


tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_fisher",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("frisians_worker", "Fisher"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      fishing_net = 1
   },

   programs = {
      fish = {
         "findspace size:any radius:8 resource:fish",
         "walk coords",
         "mine fish 1",
         "animation fishing 3000",
         "createware fish",
         "return"
      },
      fish_in_pond = {
         "findobject attrib:pond_mature radius:8",
         "walk object",
         "animation fishing 3000",
         "object fall_dry",
         "createware fish",
         "return"
      },
      breed_in_pond = {
         "findobject attrib:pond_dry radius:8",
         "walk object",
         "animation release 3000",
         "object with_fish",
         "return"
      }
   },

   animations = animations,
}
