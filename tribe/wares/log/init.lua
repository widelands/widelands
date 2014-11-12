-- NOCOM(#sirver): figure out how texts should be dealt with the multiple tribes.
help=_[[Logs are a principal building material of the barbarians. The trees are felled by lumberjacks; rangers care for the supply of trees. Logs are also used in the metal workshop to build basic tools, and in the charcoal kiln for the production of coal. The wood hardener refines logs by hardening them in the fire which gives blackwood.]]

dirname = path.dirname(__file__)
tribe:new_ware_type{
   name = "log",
   descname = _ "Log",
   default_target_quantity = 40,
   preciousness = 14,
   menu_picture = dirname .. "menu.png",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 7, },
      },
   }
}
