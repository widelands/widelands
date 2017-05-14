-- The basic worker documentation is located in /doc/sphinx/lua_tribes_workers_rst.org

dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 10, 21 },
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {8, 23}, 10)
add_walking_animations(animations, "walkload", dirname, "walkload", {8, 23}, 10)


tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_armorsmith",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Armorsmith"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      hammer = 1
   },

   animations = animations,
}
