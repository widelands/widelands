push_textdomain("world")

dirname = path.dirname(__file__)

wl.World():new_immovable_type{
   name = "bar-ruin02",
   descname = _ "Ruin",
   size = "medium",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 43, 43 },
      },
   }
}

pop_textdomain()
