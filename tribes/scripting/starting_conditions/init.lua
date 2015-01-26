dirname = path.dirname(__file__)

tribes:new_starting_condition {
   name = "atlanteans_headquarters",
   descname = _"Headquarters",
   tribe = "atlanteans",
   script_path = dirname .. "starting_conditions/atlanteans/sc00_headquarters.lua";
   order = 0;
}

tribes:new_starting_condition {
   name = "atlanteans_fortified",
   descname = _"sc01_fortified_village",
   tribe = "atlanteans",
   script_path = dirname .. "starting_conditions/atlanteans/sc01_fortified_village.lua";
   order = 1;
}

tribes:new_starting_condition {
   name = "barbarians_headquarters",
   descname = _"Headquarters",
   tribe = "barbarians",
   script_path = dirname .. "starting_conditions/barbarians/sc00_headquarters.lua";
   order = 0;
}

tribes:new_starting_condition {
   name = "barbarians_fortified",
   descname = _"sc01_fortified_village",
   tribe = "barbarians",
   script_path = dirname .. "starting_conditions/barbarians/sc01_fortified_village.lua";
   order = 1;
}

tribes:new_starting_condition {
   name = "empire_headquarters",
   descname = _"Headquarters",
   tribe = "empire",
   script_path = dirname .. "starting_conditions/empire/sc00_headquarters.lua";
   order = 0;
}

tribes:new_starting_condition {
   name = "empire_fortified",
   descname = _"sc01_fortified_village",
   tribe = "empire",
   script_path = "tribes/scripting/starting_conditions/empire/sc01_fortified_village.lua";
   order = 1;
}
