dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "spider_silk",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Spider Silk"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "spider silk"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 10
	},
   preciousness = {
		atlanteans = 11
   },
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Spider Silk
		atlanteans = pgettext("atlanteans_ware", "Spider silk is produced by spiders, which are bred by spider farms. It is processed into spidercloth in a weaving mill.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 16 },
      },
   }
}
