push_textdomain("tribes")

dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 4, 22 },
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {9, 25}, 10)

tribes:new_worker_type {
   msgctxt = "europeans_worker",
   name = "europeans_scout",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Scout"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 3,

   buildcost = {
      europeans_carrier = 1
   },

   programs = {
      scout = {
         "scout=20 90000", -- radius 20, 90 seconds until return
         "return"
      },
   --terraform = {
      --"findspace=size:any radius:6",
      --"walk=coords",
      --"terraform",
      --"return"
   --}
   },

   animations = animations,
}

pop_textdomain()
