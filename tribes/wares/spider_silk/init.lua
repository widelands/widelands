dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "spider_silk",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Spider Silk",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"spider silk",
   default_target_quantity = {
		atlanteans = 10
	},
   preciousness = {
		atlanteans = 11
   },
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Spider Silk
		atlanteans = _"Spider silk is produced by spiders, which are bred by spider farms. It is processed into spidercloth in a weaving mill."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 16 },
      },
   }
}
