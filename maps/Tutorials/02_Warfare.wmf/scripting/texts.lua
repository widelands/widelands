-- =======================================================================
--                      Texts for the tutorial mission
-- =======================================================================

-- =========================
-- Some formating functions
-- =========================

include "scripting/formatting.lua"
include "scripting/format_scenario.lua"

-- =============
-- Texts below
-- =============

introduction = {
   title = _ "Introduction",
   body = rt(
      h1(_ "Soldiers, Training and Warfare") ..
      p(_[[In this scenario, I'm going to tell you about soldiers, their training and their profession: warfare. Although Widelands is about building up, not burning down, there is an enemy you sometimes have to defeat. Yet warfare is mainly focused on economics, not on military strategies, and its mechanics deserve explanation.]]) ..
      p(_[[I've set up a small village that contains the most important things. You also have enough wares so you do not have to care for the production of your weapons. In a real game, you will not have this luxury.]])
   )
}

training1 = {
   pos = "topright",
   title = _"Training Camp and Battle Arena",
   body = rt(
      p(_[[A new soldier is created like a worker: When a military building needs a soldier, a carrier grabs the needed weapons and armor and walks out. Barbarians do not have an armor, they only need an ax.]]) ..
      p(_[[You surely can imagine that such a soldier is not very well at fighting. But luckily, there is a possibility to train him: By sending him into a training camp or the battle arena.]]) ..
      p(_[[Both are big buildings and their only purpose is to train soldiers. Since soldiers get very hungry during their workout, both buildings need a lot of food. Be sure to have a good infrastucture before you build them.]])
   )
} 

training2 = {
   pos = "topright",
   title = _ "Soldiers' abilities",
   body = rt(
      p(_[[Take a look at the soldiers that are on their way into our military buildings. They look different from normal workers: they have a health bar over their head that displays their remaining health, and they have four symbols, which symbolize the individual soldier’s current levels in the four different categories health, attack, defense and evade.]]) ..
      p(_[[If a barbarian soldier is fully trained, he has level 3 health, level 5 attack, level 0 defense and level 2 evade. This is one fearsome warrior then! The individual abilities have the following meaning:]])
   ) ..
   rt("image=tribes/barbarians/soldier/hp_level0.png", h2(_"Health:"))..
   rt(p(_[[The total life of a soldier. A barbarian soldier starts with 130 health, and with each health level he gains 28 health.]])) ..
   rt("image=tribes/barbarians/soldier/attack_level0.png", h2(_"Attack:")) ..
   rt(p(_[[The amount of damage a soldier will inflict on the enemy when an attack is successful. A barbarian soldier with attack level 0 inflicts ~14 points of health damage when he succeeds in hitting an enemy. For each attack level, he gains 7 damage points.]])) ..
   rt("image=tribes/atlanteans/soldier/defense_level0.png", h2(_"Defense:")) ..
   rt(p(_[[The defense is the percentage that is subtracted from the attack value. The barbarians cannot train in this skill and therefore have always defense level 0, which means that the damage is always reduced by 3%. If an attacker with an attack value of 35 points hits a barbarian soldier, the barbarian will lose 35·0.97 = 34 health.]])) ..
   rt("image=tribes/barbarians/soldier/evade_level0.png", h2(_"Evade:")) ..
   rt(p(_[[Evade is the chance that the soldier is able to dodge an attack. It is 25% for a level 0 evade barbarian and increases in steps of 15% for each level.]])) ..
   rt(p(_[[You probably have noticed that those icons are modified by red dots. They are quite easy to understand:]])) ..
   rt("image=tribes/barbarians/soldier/evade_level0.png", p(_[[No red dots means that the soldier is not trained, so he has level 0. All your new recruits have this.]])) ..
   rt("image=tribes/barbarians/soldier/evade_level1.png", p(_[[With every successful training step, your soldier becomes stronger. This is indicated by a red dot. This soldier is on level 1 in evade training.]])) ..
   rt("image=tribes/barbarians/soldier/evade_level2.png", p(_[[When your soldier has reached the highest possible level (in this case level 2), this is indicated by a white blackground color.]]))
}

training3 = {
   pos = "topright",
   title = _"Soldiers' abilities",
   body =
   rt(
      p(_[[Now how are all these abilities trained?]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Health is trained in the training camp. The soldiers need the helmets produced at the helm smithy.]]) ..
      listitem_bullet(_[[Attack is also trained at the training camp. You need the axes from the axfactory and the warmill - a certain one for each level.]]) ..
      listitem_bullet(_[[Evade is trained in the battle arena. Your soldiers need no weapon or armor, just some tasty strongbeer.]])
   ) ..
   rt(
   -- Not perfectly correct (some training steps need either bread or meat), but we do not want to confuse new players
      p(_[[All training steps need some food. Pitta bread is always needed, and most steps additionally require either fish or meat.]]) ..
      p(_[[You should also keep in mind that each of the three tribes in Widelands has its own way of training, so the buildings and wares are different. Also, the ability levels cannot be compared: An Imperial soldier with evade level 0 has a 30% chance of evading, while the Barbarian one only has 25%.]]) ..
      p(_[[Now let's build these trainingsites.]])
   ),
   obj_name = "build_trainingsites",
   obj_title = "Build a training camp and a battle arena",
   obj_body = rt(
      p(_[[Soldiers are trained in trainingsites. The Barbarian have two different types: the training camp for the training of health and attack, and the battle arena for the training of evade.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Build one of each.]]) ..
      listitem_arrow(_[[Remember that you can speed time up by using PAGE_UP. The more expensive a building is, is longer takes it to construct it.]])
   )
}

heroes_rookies = {
   pos = "topright",
   title = _"Heroes and Rookies",
   body = rt(
      h1(_"Heroes and Rookies") ..
      p(_[[Great. Our training sites are starting their work. Do you see how food and weapons are delivered? Soon, we will have strong warriors. Now let me tell you what we can do with them.]]) ..
      p(_[[In every military building, you can set the preference for heroes (trained soldiers) or rookies. From time to time, a soldier walks out of the building and is replaced by a stronger/weaker one automatically.]]) ..
      p(_[[The initial setting depends on the type of the building. Concerning the Barbarians, the sentry is the only building that prefers rookies by default. You should change this setting to fit your current needs.]]) ..
      p(_[[When you are expanding into no man's land, you can make your buildings prefer rookies. When you are planning to attack, send heroes into that region. Conquered buildings always prefer heroes.]])
   )
}

soldier_capacity = {
   pos = "topright",
   title = _"Soldier capacity",
   body = rt(
      h1(_"Adjusting the number of soldiers") ..
      p(_[[There is another way how you can control the strength of a military buildings: By the number of soldiers stationed there. Just click on the arrow buttons to decrease or increase the desired number of soldiers. Every buildings has a maximum capacity. In case of the barrier, it is five, for example.]]) ..
      p(_[[If you wish to send a certain soldier away, you can simply click on it. It will then be replaced by another soldier.]]) ..
      -- TODO(codereview): Is there a possibility to create non-breaking spaces?
      p(_[[Let me also describe what the numbers in the statistic string mean. It can contain up to three numbers, e.g. '1 (+5) soldier (+2)'.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[The first number describes how many soldiers are currently in this building. In this example, only one soldier is left behind (one soldier always has to guard a military building).]]) ..
      listitem_bullet(_[[The second number tells you how many additional soldiers reside in this building, but are currently outside. The five soldiers are maybe attacking an enemy. They will return when they have been successful.]]) ..
      listitem_bullet(_[[The third number indicates the missing soldiers. From the eight soldiers (1 + 5 + 2) you wish to have here, two have maybe died. They will be replaced by soldiers from your warehouse, if possible.]])
   )
}

dismantle = {
   pos = "topright",
   title = _"Dismantle your sentry",
   body = rt(
      h1(_"Dismantling military buildings") ..
      p(_[[You can only reduce the number of soldiers to one. The last soldier of a building will never come out (unless this building is attacked). If you want to have your soldier elsewhere, you have to dismantle the building (buildings of an alien tribe cannot be dismantled, only be burnt down).]]) ..
      p(_[[But destroying a military building is always linked with a risk: The land is still yours, but it is no longer protected. Any enemy that builds his own military sites can take away that land without a fight, causing your building to burst into flames. Furthermore, some parts of the land can now be hidden under the fog of war. You should therefore only dismantle military buildings inside your territory, where you are safe from enemies.]]) ..
      p(_[[Have you seen your sentry? Since it cannot contain many soldiers and is next to a stronger barrier, it is rather useless.]]) ..
      paragraphdivider() ..
      --TRANSLATORS: 'it' refers to the Barbarian sentry
      listitem_bullet(_[[Dismantle it.]])
   ) ..
   rt(p(_[[You can also use this opportunity to get familiar with the other options: The heroes/rookies preference and the capacity.]])),
   obj_name = "dismantle_sentry",
   obj_title = _"Dismantle your sentry",
   obj_body = rt(
      p(_[[You can control the number of soldiers in military sites with the arrow buttons. If you even want to get your last soldier out it, you have to destroy it. But it will no longer protect your territory, which makes it vulnerable for hostile attacks.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Dismantle your sentry.]])
   )
}

fortress_enhancement = {
   pos = "topright",
   title = _ "Enhance This Fortress",
   body = rt(
      h1(_ "Enhancing Buildings") ..
      p(_[[Well done. Now you know how to draw back your soldiers from the places where you don't need them. Now I'll tell you how to reinforce your front line.]]) ..
      p(_[[This fortress is already quite strong and conquers a lot of space. But there is an even bigger building: the citadel.]]) ..
      p(_[[Citadels can't be built directly. Instead, you'll have to construct a fortress first and then enhance it to a citadel. To do so, click on the fortress, then choose the ‘Enhance to Citadel’ button.]]) ..
      p(_[[Your soldiers will leave the fortress while the construction is going on. This means that your fortress will lose its military influence, as I described above.]]) ..
      listitem_bullet(_[[Enhance your fortress to a citadel now.]])
   ),
   obj_name = "enhance_fortress",
   obj_title = _"Enhance your fortress to a citadel",
   obj_body = rt(
      h1(_ "Enhance Your Fortress") ..
      paragraphdivider() ..
      listitem_bullet(_[[Enhance your fortress to a mighty citadel.]]) ..
      listitem_arrow(_[[The citadel can house 12 soldiers and is the biggest military building the barbarians can build. It also costs a lot of resources and takes a long time to build. It is most suited to guard strategically important points like constricted points or mountains.]])
   )
}

attack_enemy = {
   pos = "topright",
   field = wl.Game().map:get_field(29,4),
   title = _"Defeat your Enemy",
   body = rt(
      h1(_"Defeat the Enemy") ..
      p(_[[Great work, the citadel is finished. But what is that: A hostile tribe has settled next to us while the citadel was under construction! Do you see how it took away a part of our land? And our lumberjack has no lost his working place. That is what I talked about. Let's take our land back and defeat the enemy.]]) ..
      p(_[[To attack a building, click on the door, choose the number of soldiers that you wish to send and click on the attack button. Your soldiers will come from all nearby military buildings. Likewise, the defenders will come from all nearby military buildings of the enemy and intercept your forces.]]) ..
      listitem_bullet(_[[Attack and conquer all military buildings of the enemy and destroy their headquarters.]])
   ),
   obj_name = "defeated_the_empire",
   obj_title = _ "Defeat the enemy tribe",
   obj_body = rt(
      h1(_"Defeat Your Enemy") ..
      listitem_bullet(_[[Defeat the nearby enemy.]]) ..
      listitem_arrow(_[[To attack a building, click on its doors, choose the number of attacking soldiers, then send them via the ‘Attack’ button.]])
   )
}

conclude_tutorial = {
   title = _ "Conclusion",
   body = rt(
      h1(_"Conclusion") ..
      p(_[[Thank you for playing this tutorial. I hope you enjoyed it and you learned how to create and train soldiers, how to control where they go and how to defeat an enemy. Did you see how easy you could overwhelm your enemy? Having trained soldiers is a huge advantage.]]) ..
      p(_[[But a war is expensive, and not always the path leading to the goal. When you play a game (no campaign), there are also peaceful win conditions. You should definitely try them out, they're worth it.]]) ..
      p(_[[You are now ready to play the campaigns. They will teach you about the different economies of the tribes. You can also play the remaining tutorials, but they are not crucial for succeeding in the campaigns.]])
   )
}
