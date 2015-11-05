dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 9, 23 },
      fps = 10
   },
   hacking = {
      template = "hacking_??",
      directory = dirname,
      hotspot = { 8, 23 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {9, 22}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {8, 25}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_stonemason",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Stonemason"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      pick = 1
   },

   programs = {
      cut_granite = {
         "findobject attrib:rocks radius:6",
         "walk object",
         "playFX sound/stonecutting stonecutter 220",
         "animation hacking 10000",
         "object shrink",
         "createware granite",
         "return"
      },
      cut_marble = {
         "findobject attrib:rocks radius:6",
         "walk object",
         "playFX sound/stonecutting stonecutter 220",
         "animation hacking 10000",
         "object shrink",
         "createware marble",
         "return"
      }
   },

   animations = animations,
}
