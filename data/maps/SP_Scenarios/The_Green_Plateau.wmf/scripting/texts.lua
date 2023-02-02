-- =======================================================================
--                     TEXTS - No logic inside this file
-- =======================================================================

include "scripting/richtext_scenarios.lua"

-- ===========
-- Objectives
-- ===========
obj_forbidden_island = {
   name = "forbidden_island",
   title = _("The Forbidden Island"),
   number = 1,
   body = objective_text(_("The Forbidden Island"),
      _([[Finally! We have just taken our first step towards victory! Last night, we landed on the forbidden island and defeated the few guards that were watching the north-western part of the island. I am quite sure that none of the other warlords has detected us so far, so we should keep quiet and build up our infrastructure. Soon we will be strong enough to raid their positions.]])
   ),
}



obj_capture_ancient_castle = {
   name = "capture_ancient_castle",
   title = _("Capture the Ancient Castle"),
   number = 1,
   body = objective_text(_("Capture the Ancient Castle"),
      li(_([[Defeat Lanissa and capture the ancient castle.]]))
   ),
}

obj_defeat_erwyn = {
   name = "defeat_erwyn",
   title = _("Defeat Erwyn"),
   number = 1,
   body = objective_text(_("Defeat Erwyn"),
      li(_([[Defeat Erwyn. He commands the strongest opposing military force left on this island.]]))
   ),
}
obj_defeat_jomo = {
   name = "defeat_jomo",
   title = _("Defeat Jomo"),
   number = 1,
   body = objective_text(_("Defeat Jomo"),
      li(_([[Defeat Jomo. He is taking care of the soldiers’ food supply.]]))
   ),
}

-- =====================
-- Messages to the User
-- =====================

mb_w = messagebox_w_step(-1)
mb_h = messagebox_h_step(-2)
mb_y = styles.get_size("map_plateau_message_pos_y")

briefing_1_the_forbidden_island = {
   title = _("First Briefing"),
   w = mb_w,
   h = mb_h,
   posy = mb_y,
   body = objective_text(_("The Forbidden Island"),
      _([[Finally! We have just taken our first step towards victory! Last night, we landed on the forbidden island and defeated the few guards that were watching the north-western part of the island. I am quite sure that none of the other warlords has detected us so far, so we should keep quiet and build up our infrastructure. Soon we will be strong enough to raid their positions.]])
   ),
}

briefing_2_found_ancient_castle = {
   title = _("Second Briefing"),
   w = mb_w,
   h = mb_h,
   posy = mb_y,
   body = objective_text(_("An Ancient Castle"),
      p(_([[By the Gods! One of our scouts has discovered a mighty castle at the center of the old plateau. The castle must be quite old and seems to have been built in a foreign style. It’s quite obvious that this is not barbarian craft.]])) ..
      p(_([[Let’s hope that Lanissa – the warlord holding sway over that castle – has not discovered our movements yet. Perhaps we have a chance of conquering that mighty building without a bigger fight! However, it is essential that we capture it. It will be the key to our reign over this island!]])))
      .. new_objectives(obj_capture_ancient_castle)
}

briefing_3_captured_ancient_castle = {
   title = _("Second Briefing"),
   w = mb_w,
   h = mb_h,
   posy = mb_y,
   body = objective_text(_("Ancient Castle Captured"),
      p(_([[Wonderful! Our troops have finally defeated Lanissa and her soldiers. The ancient castle is ours!]])) ..
      p(_([[It is amazing how far one can see from the highest tower of the castle. We can watch the whole island. So now I wonder why Lanissa did not see us and thus did not prepare. Be that as it may, some things will never come to the light of day. The only important issue at the moment are the opposing troops still left. We have discovered enemy positions held by Erwyn to the north and east of the castle. He is known to be a strong warlord and surely commands the strongest warriors. In the south, we caught sight of some food infrastructures guarded by Jomo – a younger warlord – and we should take care of those infrastructures so we can cut off our enemies’ food supply.]])))
      .. new_objectives(obj_defeat_erwyn) .. new_objectives(obj_defeat_jomo)
}

briefing_erwyn_defeated = {
   title = _("Another Briefing"),
   w = mb_w,
   h = mb_h,
   posy = mb_y,
   body = objective_text(_("Erwyn Defeated"),
      _([[Great! Erwyn gave up when he saw his last buildings burning down. That’s one less strong warlord on this island!]]))
}

briefing_jomo_defeated = {
   title = _("Another Briefing"),
   w = mb_w,
   h = mb_h,
   posy = mb_y,
   body = objective_text(_("Jomo Defeated"),
      _([[Great! Jomo gave up when he saw his last buildings burning down. That’s one less warlord on this island!]]))
}

last_briefing_victory = {
   title = _("Last Briefing"),
   w = mb_w,
   h = mb_h,
   posy = mb_y,
   body = objective_text(_("Victory!"),
      p(_([[Finally! The island is completely ours. Now we just have to defend it better than the warlords did.]])) ..
      p(_("Congratulations! You have mastered this scenario. You may play on if you like!")))
}
