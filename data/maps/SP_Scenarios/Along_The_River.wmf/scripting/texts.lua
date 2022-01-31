-- =======================================================================
--                             Along The River
-- =======================================================================
-- Formats message box with objectives.
function advisor(title, text)
   return speech("map:advisor.png", "EEBB00", title, text)
end

briefing_intro = {
   title = _("Your Advisor"),
   body = advisor( _("Along the River"),
      _([[Sire, let me inform you about the surrounding tribes.]])
      .. paragraphdivider() ..
      _([[From the north we are endangered by a belligerent tribe, led by warlord Törver, that is rapidly building up its armies. They may soon have become too strong to be defeated.]])
      .. paragraphdivider() ..
      _([[A large area to the north-east is agonized by a wild horde of unfettered pirates, led by their insane Arvid. Their presence blocks our ways to other tribes.]])
      .. paragraphdivider() ..
      _([[Over the river lies the prosperous land of the mysterious king Runar. We have been in war with him before the pirates occupied the eastern area. His strength is based on his vast industry and large armies.]])
      .. paragraphdivider() ..
      _([[The area beyond the pirates belongs to a developed tribe led by their mighty chieftain Hjalmar. They have had a long time of preparement to build up their deterring armies.]])
      .. paragraphdivider() ..
      _([[The situation looks quite perilous and my advice is that we direct our efforts towards building up a strong army.]]))
}

briefing_warn_for_red = {
   title = _("Your Advisor"),
   body = advisor(_("Our northern enemy"),
      _([[Sire, although our soldiers would face any enemy without the slightest fear, our rival in the north is still stronger than all our armies combined. We should avoid a conflict until we are able to defeat them.]])
      .. paragraphdivider() ..
      _([[Be patient and recognize that a strong army is indispensable in this situation. Törver has a very bad reputation.]]))
}

briefing_defeat_red = {
   title = _("Your Advisor"),
   body = advisor(_("Peril from the north"),
      _([[Sire, we received an alarming message from one of our explorers. Törver is expanding his front line and approaching our territory.]])
      .. paragraphdivider() ..
      _([[We cannot avoid attacking their armies forever. If we manage to conquer our northern enemy, we will enlarge our territory and find more resources to strengthen our armies.]])
      .. paragraphdivider() ..
      _([[We can then also use their access to the lake in order to explore more land.]]))
      .. new_objectives(obj_defeat_red),
}

briefing_red_mercenaries = {
   title = _("Your Advisor"),
   body = advisor(_("On the good way"),
      _([[Sire, the battle with Törver progresses gloriously and we have already beaten the strongest part of his army. But this enemy will not be defeated easily. In a desperate move Törver has called in a group of hardened mercenaries.]]))
}

briefing_build_port = {
   title = _("Your Advisor"),
   body = advisor(_("The great lake"),
      _([[Sire, now that we have conquered the land of Törver, we can make use of their excellent access to the adjacent great lake. A port in the north will give us better opportunities to have contact with other tribes.]])
      .. paragraphdivider() ..
      _([[In Törver’s headquarters we found papers describing the construction of ships. Unfortunately, the texts are written in a language that is unknown to us. Our wise men will try to translate these descriptions.]]))
      .. new_objectives(obj_build_port),
}

briefing_conquer_pirate_sentry = {
   title = _("Your Advisor"),
   body = advisor(_("Searching for gold"),
      _([[Sire, our men have located an isolated enemy pirate sentry. The abandoned huts indicate that there is gold nearby.]])
      .. paragraphdivider() ..
      _([[We should however be careful if we want to conquer the sentry. There can be more enemy buildings that are still out of sight, and the pirates are known to be very good fighters. It is best to establish a strong military presence here before we proceed.]]))
      .. new_objectives(obj_conquer_pirate_sentry),
}

briefing_found_pirate_treasure = {
   title = _("Your Advisor"),
   body = advisor(_("Found a treasure"),
      _([[Sire, we have found 25 units of gold ore inside the pirate sentry. They may have been looted from the gold diggers’ huts nearby. This gold ore will be transported to our headquarters.]])
      .. paragraphdivider() ..
      _([[With our successful expansion into this area we should consider building a warehouse to the direct southwest of the sentry in order to further explore this area. There is a good building spot only three steps away from the sentry.]]))
      .. new_objectives(obj_build_warehouse),
}

briefing_bring_shovels_1 = {
   title = _("Your Advisor"),
   body = advisor(_("Removing the swamps"),
      _([[Sire, the wet season has created swamps along the brook to the east and we are unable to pass through them. They have to be removed before we can continue. If you bring enough shovels to the new warehouse then volunteers will remove the swamps.]]))
      .. new_objectives(obj_bring_shovels_1),
}

briefing_bring_shovels_2 = {
   title = _("Your Advisor"),
   body = advisor(_("Removing the swamps"),
      _([[Sire, all shovels have been handed out but if we continue at this pace than we will not have removed the swamps before the next wet season. If you bring more shovels to the new warehouse then more volunteers will join in.]]))
      .. new_objectives(obj_bring_shovels_2),
}

briefing_defeat_pirates = {
   title = _("Your Advisor"),
   body = advisor(_("Defeat the pirates"),
      _([[Sire, now that we have access to the pirates’ hideout, we should conquer it and finish them off. Maybe there are even more treasures hidden in their territory.]]))
      .. new_objectives(obj_defeat_pirates),
}

briefing_pirates_defeated = {
   title = _("Your Advisor"),
   body = advisor(_("The pirates were defeated."),
      _([[Sire, we have defeated the pirates. They have hidden some gold in their last bastion which is now being transferred to our headquarters.]]))
}

briefing_pirates_regrouping = {
   title = _("Your Advisor"),
   body = advisor(_("Pirates are regrouping"),
      _([[Sire, as a result of our attacks on the pirates they have started to regroup with pirates from other tribes. They could regain much of their previously lost strength. Our dwindling resources can become a problem with such a mighty enemy in front of us.]])),
}

briefing_pirates_on_the_run = {
   title = _("Your Advisor"),
   body = advisor(_("Pirates on the run"),
      _([[Sire, thanks to our successful efforts to drive out the pirates, lots of them have fled and moved away in small boats. It is unknown in which direction they have escaped.]]))
}

briefing_shipyard = {
   title = _("Your Advisor"),
   body = advisor(_("Ability to build shipyards"),
      _([[Sire, after long study and research our wise men have finally managed to decipher the strange language in Törver’s descriptions that we had found in his headquarters. From now on we are able to build shipyards.]]))
}

briefing_free_scout = {
   title = _("Your Advisor"),
   body = advisor(_("Our scout has been captured"),
      _([[Sire, one of our scouts has been captured by the pirates and brought to their island. He is the only person who knows the way through the swamps towards Hjalmar.]])
      .. paragraphdivider() ..
      _([[According to our fishermen he is locked up in the stronghold on the island. We should use the knowledge about shipbuilding that we took from Törver to build ships and attack their positions.]]))
      .. new_objectives(obj_free_scout),
}

briefing_island_conquered = {
   title = _("Your Advisor"),
   body = advisor(_("Island conquered"),
      _([[Sire, our soldiers have conquered the island and managed to free the scout. It turns out that the pirates have left a large amount of gold, beer, and weapons on the island.]]))
}

briefing_scout = {
   title = _("Your Advisor"),
   body = advisor(_("The scout returns"),
      _([[Sire, our scout has returned to our territory, and he has shown us the road through the swamps towards Hjalmar.]])
      .. paragraphdivider() ..
      _([[He noticed that we can also move along the other side of the mountain, in case we might fail here, but it will be much more difficult to do.]]))
      .. new_objectives(obj_defeat_green),
}

briefing_defeat_yellow = {
   title = _("Your Advisor"),
   body = advisor(_("The last frontier"),
      _([[Sire, we have almost defeated all armies and we are now approaching our final enemy. They have been expecting us for a long time and will be prepared for the heaviest battle in their lifetime.]])
      .. paragraphdivider() ..
      _([[Make sure that our armies are at full strength and then act decisively.]]))
      .. new_objectives(obj_defeat_yellow),
}

briefing_victory = {
   title = _("Your Advisor"),
   body = advisor(_("Victory!"),
      _([[Sire, my congratulations!]])
      .. paragraphdivider() ..
      _([[You have completed all goals and defeated all our enemies. You have not only conquered our rivalling tribes, but you have also captured the hearts of your loving people.]])
      .. paragraphdivider() ..
      _([[Your name will be remembered forever.]]))
}
