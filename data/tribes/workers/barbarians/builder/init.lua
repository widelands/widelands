dirname = path.dirname(__file__)

animations = {}
-- NOCOM remove this and the files add_animation(animations, "idle", dirname, "waiting", {11, 22}, 10)
add_animation(animations, "work", dirname, "work", {10, 22}, 10)
animations["work"]["sound_effect"] = {
   path = "sound/hammering/hammering",
   priority = 64
}
add_directional_animation(animations, "walk", dirname, "walk", {7, 22}, 10)
add_directional_animation(animations, "walkload", dirname, "walk", {7, 22}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_builder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Builder"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      hammer = 1
   },

   animations = animations,
   spritesheets = {
      idle = {
         directory = path.dirname(__file__),
         basename = "idle",
         fps = 10,
         frames = 150,
         columns = 12,
         rows = 13,
         hotspot = { 6, 19 }
      }
   }
}
