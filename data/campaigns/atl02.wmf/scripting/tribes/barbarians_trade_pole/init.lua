push_textdomain("scenario_atl02.wmf")

dirname = "campaigns/atl02.wmf/scripting/tribes/barbarians_trade_pole/"

wl.Descriptions():new_immovable_type {
   name = "barbarians_trade_pole",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Trade Pole"),
   size = "small",
   icon = dirname .. "tradepole_1.png",
   programs = {
      main = {
         "animate=idle",
      }
   },
   animations = {
      idle = {
         directory = dirname,
         basename = "tradepole",
         hotspot = {8, 27}
      }
   }
}

pop_textdomain()
