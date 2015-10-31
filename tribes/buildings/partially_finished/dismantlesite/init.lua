dirname = path.dirname(__file__)

tribes:new_dismantlesite_type {
   msgctxt = "building",
   name = "dismantlesite",
   -- TRANSLATORS: This is a name used in lists of buildings for buildings being taken apart
   descname = pgettext("building", "Dismantle Site"),
   directory = dirname,
   icon = dirname .. "menu.png",
   -- NOCOM(#codereview): can we get rid of size then?
   size = "small", -- Dummy; overridden by building size
   vision_range = 2,

   animations = {
		idle = {
			template = "idle_??",
			directory = dirname,
			hotspot = { 5, 5 },
		},
	},

	aihints = {},
}
