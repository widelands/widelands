-- =======================================================================
--                      Texts for the tutorial mission
-- =======================================================================

-- We want the soldier here so we can get some actual stats.
local tribe = wl.Game():get_tribe_description("barbarians")
local soldier = wl.Game():get_worker_description(tribe.soldier)

local empire_tribe = wl.Game():get_tribe_description("empire")
local empire_soldier = wl.Game():get_worker_description(empire_tribe.soldier)

function h2_image(imagepath, text)
   return h2(img(imagepath) .. space() .. text)
end

-- =============
-- Texts below
-- =============

introduction = {
   title = _("Introduction"),
   body = (
      h1(_("Soldiers, Training and Warfare")) ..
      p(_([[In this scenario, I’m going to tell you about soldiers, their training and their profession: warfare. Although Widelands is about building up, not burning down, there is an enemy you sometimes have to defeat. Yet warfare is mainly focused on economics, not on military strategies, and its mechanics deserve explanation.]])) ..
      p(_([[I’ve set up a small village that contains the most important buildings. You also have enough wares, so you do not have to take care of your weapons production. In a real game, you will not have this luxury.]]))
   ),
   h = messagebox_h_step(-2)
}

abilities = {
   position = "topright",
   title = _("Soldiers’ abilities"),
   body = (
      p(_([[New soldiers are created differently from normal workers: recruits will walk from a warehouse (or your headquarters) to the barracks where they will receive some basic training and become soldiers. Basic Barbarian soldiers do not use armor, they only need an ax.]])) ..
      p(_([[Take a look at the soldiers that are on their way to our military buildings. They look different from normal workers: they have a health bar over their head that displays their remaining health, and they have four symbols, which symbolize the individual soldier’s current levels in the four different categories: health, attack, defense and evade.]])) ..
      -- TRANSLATORS: the current stats are: 3 health, 5 attack, 0 defense, 2 evade.
      p((_([[If a Barbarian soldier is fully trained, he has level %1% health, level %2% attack, level %3% defense and level %4% evade. This is one fearsome warrior then! The individual abilities have the following meaning:]])):bformat(soldier.max_health_level, soldier.max_attack_level, soldier.max_defense_level, soldier.max_evade_level)) ..
      h2_image("tribes/workers/barbarians/soldier/health_level0.png", _("Health:")) ..
      -- TRANSLATORS: the current stats are: 13000 health, 2800 health gain.
      p(_([[The total life of a soldier. A Barbarian soldier starts with %1% health, and he will gain %2% health with each health level.]])):bformat(soldier.base_health, soldier.health_incr_per_level) ..
      h2_image("tribes/workers/barbarians/soldier/attack_level0.png", _("Attack:")) ..
      -- TRANSLATORS: the current stats are: 1400 damage, gains 850 damage points.
      p(_([[The amount of damage a soldier will inflict on the enemy when an attack is successful. A Barbarian soldier with attack level 0 inflicts ~%1% points of health damage when he succeeds in hitting an enemy. For each attack level, he gains %2% damage points.]])):bformat(soldier.base_min_attack + (soldier.base_max_attack - soldier.base_min_attack) / 2, soldier.attack_incr_per_level) ..
      -- The Atlanteans' image, because the Barbarian one has a white background
      h2_image("tribes/workers/atlanteans/soldier/defense_level0.png", _("Defense:")) ..
      -- TRANSLATORS: the current stats are: 3%. The calculated health value is 3395
      -- TRANSLATORS: The last two %% after the placeholder are the percent symbol.
      p(_([[The defense is the percentage that is subtracted from the attack value. The Barbarians cannot train in this skill and therefore have always defense level 0, which means that the damage is always reduced by %1%%%. If an attacker with an attack value of 3500 points hits a Barbarian soldier, the Barbarian will lose 3500·%2%%% = %3% health.]])):bformat(soldier.base_defense, (100 - soldier.base_defense), 3500 * (100 - soldier.base_defense) / 100) ..
      h2_image("tribes/workers/barbarians/soldier/evade_level0.png", _("Evade:")) ..
      -- TRANSLATORS: the current stats are: 25% evade, increases in steps of 15%.
      -- TRANSLATORS: The last two %% after the placeholder are the percent symbol.
      p(_([[Evade is the chance that the soldier is able to dodge an attack. A level 0 Barbarian has a %1%%% chance to evade an attack, and this increases in steps of %2%%% for each level.]])):bformat(soldier.base_evade, soldier.evade_incr_per_level))
}

battlearena1 = {
   position = "topright",
   title = _("The Battle Arena"),
   body = (
      p(_([[Now I have talked about training and levels. Let me elaborate on that.]])) ..
      p(_([[A newly created soldier has no experience and is not very good at fighting. To make him stronger, you can build training sites.]])) ..
      p(_([[One of these training sites is the battle arena. It is a big and expensive building, and it trains soldiers in evade. Since soldiers get very hungry during their workout, this building needs a lot of food and strong beer. In a real game, you should have a good infrastructure before you build it.]])) ..
      li(_([[To see evade training in action, build a battle arena.]])) ..
      p(_([[While we’re waiting for the battle arena, you’ll probably notice some soldiers walking around. They are automatically exchanged from time to time. I’ll teach you about that later.]]))
   )
}

obj_battlearena = {
   name = "build_battlearena",
   title = _("Battle Arena"),
   number = 1,
   body = objective_text(_("Build a battle arena"),
      li(_([[Build a battle arena. It is a big building.]])) ..
      li_arrow(_([[Since the construction will take some time, you can change the game speed using %1$s and %2$s.]]):bformat(wl.ui.get_shortcut("game_speed_up"), wl.ui.get_shortcut("game_speed_down")))
   )
}

battlearena2 = {
   position = "topright",
   title = _("The Battle Arena"),
   body =
      h1(_("The Battle Arena Has Been Constructed")) ..
      p(_([[Very good. Our battle arena has been finished, and the soldiers are already walking towards it.]])) ..
      -- Not perfectly correct (some training steps need either bread or meat), but we do not want to confuse new players
      p(_([[The needed wares are also delivered there. For successful training, you need pitta bread and strong beer, as well as either fish or meat.]]) .. " " ..
      _([[For more information, you can have a look at the building’s help window, accessible via the question mark in every building’s window.]])) ..
      p(_([[To learn how far your soldiers have progressed in their training, you can have a look at their icons. They are modified by red dots:]])) ..
      li_image("tribes/workers/barbarians/soldier/evade_level0.png", _([[No red dots means that the soldier is not trained, so he has level 0. All your new recruits have this.]])) ..
      li_image("tribes/workers/barbarians/soldier/evade_level1.png", _([[With every successful training step, your soldier becomes stronger. This is indicated by a red dot. This soldier is on level 1 in evade training.]])) ..
      li_image("tribes/workers/barbarians/soldier/evade_level2.png", _([[When your soldier has reached the highest possible level (in this case level 2), this is indicated by a white background color.]])),
   h = messagebox_h_step(1)
}

trainingcamp1 = {
   position = "topright",
   title = _("The Training Camp"),
   body = (
      h1(_("The Training Camp")) ..
      p(_([[There is a second training site: the training camp. It is a big building too, and to complement the battle arena, it trains attack and health (remember, the Barbarian soldiers cannot be trained in defense).]])) ..
      li(_([[Build a training camp.]]))
   )
}

obj_trainingcamp = {
   name = "build_trainingcamp",
   title = _("Training Camp"),
   number = 1,
   body = objective_text(_("Build a training camp"),
      p(_([[The battle arena only trains the soldiers in evade. To get the strongest possible soldier, you also need to build a training camp, which trains them in attack and health.]])) ..
      li(_([[Build a training camp.]]))
   )
}

trainingcamp2 = {
   position = "topright",
   title = _("The Training Camp"),
   body = (
      p(_([[Great, our training camp has now been finished, too. Now nothing will hinder us from getting the strongest warriors the world has ever seen.]])) ..
      p(_([[To train in the training camp, our soldiers need food like in the battle arena, but no strong beer. Instead, they need different axes for attack training and helmets for health training.]])) ..
      p(_([[This equipment is produced in smithies out of coal, iron, and sometimes gold. You will learn more about this in the second scenario of the Barbarian campaign.]])) ..
      p(_([[You should also keep in mind that each of the tribes in Widelands has its own way of training, so the buildings and wares are different. Also, the ability levels cannot be compared: an Imperial soldier has a base attack of %1% at level 0, while a Barbarian soldier at the same level only has a base attack of %2%.]]):bformat(empire_soldier.base_min_attack, soldier.base_min_attack))
   )
}

scouting1 = {
   position = "topright",
   title = _("Scout’s Hut"),
   body = (
      h1(_("Explore Your Surroundings")) ..
      p(_([[Large parts of the map haven’t been explored yet. It might be a good idea to know which direction is best for expanding our empire.]])) ..
      p(_([[You can try your luck and expand randomly, but this is risky. A safer way is to use a scout to explore unseen parts of the map. What if he finds mountains overflowing with gold?]])) ..
      p(_([[We will need a scout’s hut to start exploring. It is a small building.]])) ..
      li_object("barbarians_scouts_hut", _([[Build a scout’s hut.]]), plr.color)
   )
}

obj_scouting1 = {
   name = "build_scouts_hut",
   title = _("Scout’s Hut"),
   number = 1,
   body = objective_text(_("Build a scout’s hut"),
      p(_([[It’s good to know your surroundings. A scout will explore unseen parts of the map for you.]])) ..
      li(_([[Build a Scout’s Hut.]]))
   )
}

scouting2 = {
   position = "topright",
   title = _("Scout is ready"),
   body = (
      h1(_("Ready to go!")) ..
      p(_([[Well done! Your scout’s hut has been built. While our scout is moving in, let’s learn a little bit more about him.]])) ..
      p(_([[First of all, he will need to take some food with him on his scouting trips. Fortunately, we have some rations in our warehouse.]])) ..
      li_object("barbarians_scout", _([[When provided with food, he will walk randomly around the area. You should follow his movement before fog of war hides what he saw.]]), plr.color)
   )
}

obj_scouting2 = {
   name = "explore_map",
   title = _("Scout"),
   number = 1,
   body = objective_text(_("Explore the map"),
      p(_([[Wait for the scout to explore unseen areas.]])) ..
      li(_([[Observe your scout.]]))
   )
}

scouting3 = {
   position = "topright",
   title = _("Scout made progress"),
   body = (
      h1(_("New horizons?")) ..
      p(_([[As you can see, scouts can go quite far. The good news is that there are no enemies around.]])) ..
      p(_([[You can use the minimap to see the exploration’s progress.]])) ..
      p(_([[Don’t forget that scouting is hard work and as such cannot be done without provision of food.]]))
   ),
   h = messagebox_h_step(-3)
}
heroes_rookies = {
   position = "topright",
   title = _("Heroes and Rookies"),
   body = (
      h1(_("Heroes and Rookies")) ..
      p(_([[While our soldiers are training, let me tell you what we can do with them.]])) ..
      p(_([[In every military building, you can set the preference for heroes (trained soldiers) or rookies. From time to time, a soldier will walk out of the building and be replaced by a stronger/weaker one automatically – this is what you saw earlier.]])) ..
      p(_([[The initial setting depends on the type of the building. For the Barbarians, the sentry is the only building that prefers rookies by default. You should change this setting to fit your current needs.]])) ..
      p(_([[When you are expanding into no man’s land, you can make your buildings prefer rookies. When you are planning to attack, send heroes into that region. Conquered buildings always prefer heroes.]]))
   )
}

soldier_capacity = {
   position = "topright",
   title = _("Soldier capacity"),
   body = (
      h1(_("Adjusting the number of soldiers")) ..
      p(_([[There is another way how you can control the strength of a military building: by the number of soldiers stationed there. Just click on the arrow buttons to decrease or increase the desired number of soldiers. Every building has a maximum capacity. In case of the barrier, it is five, for example.]])) ..
      p(_([[If you wish to send a certain soldier away, you can simply click on it. It will then be replaced by another soldier.]])) ..
      p(_([[Let me also describe what the numbers in the status string mean. This string can contain an arrow and up to three numbers, e.g. ‘↑ 1 (+5) soldier (+2)’.]])) ..
      li(_([[The arrow shows whether the building is set to prefer heroes (upwards arrow) or rookies (downwards arrow).]])) ..
      li(_([[The first number describes how many soldiers are currently in this building. In this example, only one soldier is left inside (each military building is always guarded by at least one soldier).]])) ..
      li(_([[The second number tells you how many additional soldiers reside in this building, but are currently outside. The five soldiers may be attacking an enemy. They will return when they have been successful.]])) ..
      li(_([[The third number indicates the missing soldiers. From the eight soldiers (1 + 5 + 2) you wish to have here, two may have died. They will be replaced by new soldiers from your warehouse, if possible.]]))
   )
}

dismantle = {
   position = "topright",
   title = _("Dismantle your sentry"),
   body = (
      h1(_("Dismantling military buildings")) ..
      p(_([[You can only reduce the number of soldiers to one. The last soldier of a building will never come out (unless this building is attacked). If you want to have your soldier elsewhere, you will have to dismantle or burn down the building.]])) ..
      p(_([[However, destroying a military building is always linked with a risk: the land is still yours, but it is no longer protected. Any enemy that builds his own military sites can take over that land without a fight, causing your buildings to burst into flames. Furthermore, some parts of the land can now be hidden under the fog of war. You should therefore only dismantle military buildings deep inside your territory where you are safe from enemies.]])) ..
      p(_([[Have you seen your sentry? Since it cannot contain many soldiers and is next to a stronger barrier, it is rather useless.]])) ..
      -- TRANSLATORS: 'it' refers to the Barbarian sentry
      li(_([[Dismantle it.]])) ..
      p(_([[You can also use this opportunity to become familiar with the other options: the heroes/rookies preference and the capacity.]]))
   )
}

obj_dismantle = {
   name = "dismantle_sentry",
   title = _("Soldiers, dismissed!"),
   number = 1,
   body = objective_text(_("Dismantle your north-western sentry"),
      p(_([[You can control the number of soldiers stationed at a military site with the arrow buttons. If you want to get even your last soldier out, you will have to destroy it. However, it then will no longer protect your territory, which will make it vulnerable to hostile attacks.]])) ..
      li(_([[Dismantle your sentry in the north-west, next to the barrier.]]))
   )
}

fortress_enhancement = {
   position = "topright",
   title = _("Enhance Your Fortress"),
   body = (
      h1(_("Enhancing Buildings")) ..
      p(_([[Well done. Now you know how to draw back your soldiers from the places where you don’t need them. It is time to tell you how to reinforce your front line.]])) ..
      p(_([[Your fortress is already quite strong and conquers a lot of space. But there is an even bigger building: the citadel.]])) ..
      p(_([[Citadels can’t be built directly. Instead, you’ll have to construct a fortress first and then enhance it to a citadel. To do so, click on the fortress, then choose the ‘Enhance to Citadel’ button.]])) ..
      p(_([[Your soldiers will leave the fortress while the construction is going on. This means that your fortress will lose its military influence, as I described above.]])) ..
      li(_([[Enhance your fortress to a citadel now.]]))
   )
}

obj_fortress = {
   name = "enhance_fortress",
   title = _("Enhance Your Fortress"),
   number = 1,
   body = objective_text(_("Enhance your fortress to a citadel"),
      li(_([[Enhance your fortress to a mighty citadel.]])) ..
      li_arrow(_([[The citadel can house 12 soldiers, and it is the biggest military building the Barbarians can build. It also costs a lot of resources and takes a long time to build. It is most suited to guard strategically important points like constricted points or mountains.]]))
   )
}

attack_enemy = {
   position = "topright",
   field = wl.Game().map:get_field(29,4), -- show the lost territory
   title = _("Defeat your Enemy"),
   body = (
      h1(_("Defeat the Enemy")) ..
      p(_([[Great work, the citadel is finished. But what’s that? A hostile tribe has settled next to us while the citadel was under construction! Do you see how they took away a part of our land? And our lumberjack has now lost his place of work. This is what I was talking about. Let’s take our land back and defeat the enemy!]])) ..
      p(_([[To attack a building, click on its doors, choose the number of soldiers that you wish to send and click on the ‘Attack’ button.]]) .. " " .. _([[Your soldiers will come from all nearby military buildings. Likewise, the defenders will come from all nearby military buildings of the enemy and intercept your forces.]])) ..
      li(_([[Attack and conquer all military buildings of the enemy and destroy their headquarters.]]))
   )
}

obj_attack = {
   name = "defeat_the_enemy",
   title = _("Defeat Your Enemy"),
   number = 1,
   body = objective_text(_("Defeat the enemy tribe"),
      li(_([[Defeat the nearby enemy.]])) ..
      li_arrow(_([[To attack a building, click on its doors, choose the number of soldiers that you wish to send and click on the ‘Attack’ button.]]))
   )
}

conclude_tutorial = {
   title = _("Conclusion"),
   body = (
      h1(_("Conclusion")) ..
      p(_([[Thank you for playing this tutorial. I hope you enjoyed it and you learned how to create and train soldiers, how to control where they go and how to defeat an enemy. Did you see how easily you could overwhelm your enemy? Having trained soldiers is a huge advantage.]])) ..
      p(_([[But a war is expensive, and not always the path leading to the goal. When setting up a new game, you can also choose peaceful win conditions. You should definitely try them out, they’re worth it.]])) ..
      p(_([[You are now ready to play the campaigns. They will teach you about the different economies of the tribes. You can also play the remaining tutorials, but they are not crucial for succeeding in the campaigns.]]))
   ),
   allow_next_scenario = true,
}
