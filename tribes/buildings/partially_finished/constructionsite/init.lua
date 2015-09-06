dirname = path.dirname(__file__)

tribes:new_constructionsite_type {
   msgctxt = "building",
   name = "constructionsite",
   -- TRANSLATORS: This is a name used in lists of buildings for buildings under construction
   descname = pgettext("building", "Construction Site"),
   directory = dirname,
   size = "small", -- Dummy; overridden by building size
   vision_range = 2,

   animations = {
		-- The constructionsite is a mess. Not nice and clean, but rather some
		-- logs lying around on piles, maybe some tools.
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 5, 5 },
		},
		idle_with_worker = {
			pictures = path.list_directory(dirname, "idle_with_worker_\\d+.png"),
			hotspot = { 33, 36 },
		}
	},

	aihints = {},
}
