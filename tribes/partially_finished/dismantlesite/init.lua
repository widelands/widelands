dirname = path.dirname(__file__)

tribes:new_dismantlesite_type {
   name = "dismantlesite",
   -- TRANSLATORS: This is a name used in lists of buildings for buildings being taken apart
   descname = _"Dismantle Site",
   size = "small", -- Dummy; overridden by building size
   buildable = false,
   vision_range = 2,

 	-- TRANSLATORS: Helptext for a building: Dismantle Site
   helptext = "", -- TODO(GunChleoc): Have a look at this after merging the branch.

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 5, 5 },
		},
	},
}
