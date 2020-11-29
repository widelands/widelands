push_textdomain("tribes")

dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "waiting_???.png"),
      hotspot = { 16, 23 },
      fps=10,
   },
   work = {
      pictures = path.list_files(dirname .. "work_??.png"),
      sound_effect = {
         path = "sound/hammering/hammering",
         priority = 64
      },
      hotspot = { 6, 22 },
      fps=10,
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {8, 24}, 10)
add_directional_animation(animations, "walkload", dirname, "walk", {8, 24}, 10)


tribes:new_worker_type {
   msgctxt = "europeans_worker",
   name = "europeans_builder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Builder"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      europeans_carrier = 1,
      hammer = 1
   },

   animations = animations,
}

pop_textdomain()
