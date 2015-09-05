dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 10, 25 },
      fps = 20
   }
}
-- TODO(GunChleoc): Make real oxen animations
add_worker_animations(animations, "walk", dirname, "walk", {21, 31}, 20)
add_worker_animations(animations, "walkload", dirname, "walk", {21, 31}, 10)


tribes:new_carrier_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_ox",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Ox"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("barbarians_worker", "oxen"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   default_target_quantity = 10,
   ware_hotspot = { -2, 13 },

	-- TRANSLATORS: Helptext for a worker: Ox
   helptext = pgettext("barbarians_worker", "Oxen help to carry items along busy roads. They are reared in a cattle farm."),
   animations = animations,
}
