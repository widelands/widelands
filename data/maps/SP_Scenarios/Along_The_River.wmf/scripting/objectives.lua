-- =======================================================================
--                             Along The River
-- =======================================================================

obj_defeat_red = {
   name = "defeat_red",
   title = _("Defeat Törver"),
   number = 1,
   body = objective_text(_("Defeat the Armies of Törver"),
      li_image("map:banner_plr_02.png", _([[Törver is the enemy in the north, who is fighting under the red banner.]]))
   ),
}

obj_build_port = {
   name = "build_port",
   title = _("Build a port"),
   number = 1,
   body = objective_text(_("Build a Port in the North"),
      li_image("tribes/buildings/warehouses/barbarians/port/menu.png", _([[We should build a port in Törver’s former land to be ready to send an expedition as soon as we found out how to build ships.]]))
   ),
}

obj_conquer_pirate_sentry = {
   name = "conquer_pirate_sentry",
   title = _("Conquer pirate sentry"),
   number = 1,
   body = objective_text(_("Conquer the Pirate Sentry"),
      li_image("tribes/buildings/militarysites/barbarians/sentry/menu.png", _([[Conquer the pirate sentry near the brook to the east. Hopefully we will find out what happened to the gold in this area.]]))
   ),
}

obj_build_warehouse = {
   name = "build_warehouse",
   title = _("Build warehouse"),
   number = 1,
   body = objective_text(_("Build a Warehouse near the Pirate Sentry"),
      li_image("tribes/buildings/warehouses/barbarians/warehouse/menu.png", _([[Build a warehouse close to the conquered pirate sentry near the brook. We will need it as a basis for further exploration. Furthermore we could use it to store some gold that we will mine from the surrounding hills.]]))
   ),
}

obj_bring_shovels_1 = {
   name = "bring_shovels_1",
   title = _("Bring shovels"),
   number = 1,
   body = objective_text(_("Bring Four Shovels to the new Warehouse"),
      li_image("tribes/wares/shovel/menu.png", _([[They will be handed out to some volunteers for drying up the swamps.]]))
   ),
}

obj_bring_shovels_2 = {
   name = "bring_shovels_2",
   title = _("Bring shovels"),
   number = 1,
   body = objective_text(_("Bring Four Shovels to the new Warehouse"),
      li_image("tribes/wares/shovel/menu.png", _([[They will be handed out to additional volunteers for drying up the swamps.]]))
   ),
}

obj_defeat_pirates = {
   name = "defeat_pirates",
   title = _("Defeat the pirates"),
   number = 1,
   body = objective_text(_("Defeat the Pirates"),
      li_image("map:banner_plr_05.png", _([[Free our neighbourhood from the pirate menace. The pirates are fighting under the grey banner.]]))
   ),
}

obj_free_scout = {
   name = "free_scout",
   title = _("Free our scout from the pirates"),
   number = 1,
   body = objective_text(_("Conquer the Stronghold on the Island"),
      li_image("tribes/buildings/militarysites/barbarians/barrier/menu.png", _([[This is where the scout is held captive.]]))
   ),
}

obj_defeat_green = {
   name = "defeat_green",
   title = _("Defeat Hjalmar"),
   number = 1,
   body = objective_text(_("Defeat the Armies of Hjalmar"),
      li_image("map:banner_plr_04.png", _([[Hjalmar is the enemy north of the former pirates, who is fighting under the green banner.]]))
   ),
}

obj_defeat_yellow = {
   name = "defeat_yellow",
   title = _("Defeat Runar"),
   number = 1,
   body = objective_text(_("Defeat the Armies of Runar"),
      li_image("map:banner_plr_03.png", _([[Runar is the enemy south-east of the former pirates, who is fighting under the yellow banner.]]))
   ),
}
