push_textdomain("tribes")

dirname = path.dirname(__file__)

animations = {}
add_animation(animations, "idle", dirname, "idle", {13, 24}, 10)
add_directional_animation(animations, "walk", dirname, "walk", {8, 25}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {8, 25}, 10)

tribes:new_carrier_type {
   msgctxt = "europeans_worker",
   --msgctxt = msgctxt,
   name = "europeans_carrier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Carrier"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   animations = animations,
}

pop_textdomain()
