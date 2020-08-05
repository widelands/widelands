dirname = "test/maps/plain.wmf/" .. path.dirname(__file__)

animations = {}

add_animation(animations, "idle", dirname, "idle", {14, 21}, 5)
-- These won't be played in the test suite, just testing the table structure.
animations["idle"]["sound_effect"] = {
   path = "sound/hammering/hammering",
   priority = 64
}

add_directional_animation(animations, "walk", dirname, "walk", {9, 19}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {7, 22}, 10)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_file_animation",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "File Animation"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   animations = animations,

   programs = {
      show_idle = {
         "findspace=size:any radius:5",
         "walk=coords",
         "animate=idle duration:10s",
         "return"
      }
   },
}
