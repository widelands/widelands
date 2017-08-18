-- =========================
-- Some formating functions
-- =========================

include "scripting/formatting.lua"
include "scripting/format_scenario.lua"

function reebaud(title, text)
   return speech("map:reebaud.png", "6622CC", title, text)
end
function hauke(title, text)
   return speech("map:hauke.png", "88CCCC", title, text)
end
function maukor(title, text)
   return speech("map:maukor.png", "990000", title, text)
end

-- =======================================================================
--                           Texts - No logic here
-- =======================================================================


-- ===========
-- objectives
-- ===========
obj_build_wood_economy = {
   name = "build_wood_economy",
   title=_"Build a wood economy and a reed farm",
   number = 1,
   body = objective_text(_"Wood Economy",
      listitem_bullet(_[[Build at least one woodcutter´s house and two forester´s houses to start producing logs, and a reed farm to start growing reed.]])
   ),
}
obj_build_brick_economy = {
   name = "build_brick_economy",
   title=_"Start producing bricks",
   number = 1,
   body = objective_text(_"Brick Economy",
      listitem_bullet(_[[Build a brick burner´s house, a coal mine, a rock mine, a clay pit and a well to start producing bricks.]])
   ),
}
obj_build_food_economy = {
   name = "build_food_economy",
   title=_"Produce food for the miners",
   number = 1,
   body = objective_text(_"Food Economy",
      listitem_bullet(_[[Build a tavern, a fisher´s house, a smokery, a berry farm and a fruit collector´s house.]])
   ),
}
obj_build_food_economy_2 = {
   name = "build_food_economy_2",
   title=_"Enhance your food economy",
   number = 1,
   body = objective_text(_"Enhance Food Economy",
      listitem_bullet(_[[Build at least two farms, a bee-keeper´s house, a bakery, a brewery, and another tavern.]])
   ),
}
obj_build_mining = {
   name = "build_mining",
   title=_"Build one mine of each type and a mining infrastructure",
   number = 1,
   body = objective_text(_"Build mines and mining infrastructure",
      listitem_bullet(_[[Build an iron mine and a gold mine. Also build a furnace, a blacksmithy and a small armour smithy.]])
   ),
}
obj_expand = {
   name = "expand",
   title=_"Discover more of the island",
   number = 1,
   body = objective_text(_"Expand and discover",
      listitem_bullet(_[[Expand further and discover more of the island.]])
   ),
}
obj_recruit_soldiers = {
   name = "recruit_soldiers",
   title=_"Start recruiting soldiers",
   number = 1,
   body = objective_text(_"Recruiting Soldiers",
      listitem_bullet(_[[Build a barracks, a reindeer farm and a seamstress.]])
   ),
}
obj_train_soldiers = {
   name = "train_soldiers",
   title=_"Train your soldiers",
   number = 1,
   body = objective_text(_"Training Soldiers",
      listitem_bullet(_[[Train a soldier to the highest possible level and send him to a military building.]])
   ),
}
obj_defeat_enemy = {
   name = "defeat_enemy",
   title=_"Defeat the enemy!",
   number = 1,
   body = objective_text(_"Defeat the enemy!",
      listitem_bullet(_[[Destroy the enemy tribe´s headquarters.]])
   ),
}
obj_escape = {
   name = "escape",
   title=_"Escape from the floods",
   number = 1,
   body = objective_text(_"Flee from the floods",
      listitem_bullet(_[[Build a port, a weaving mill and a shipyard, then start an expedition to flee before the stormflood drowns the island.]])
   ),
}



-- ==================
-- Texts to the user
-- ==================
intro_1 = {
   title =_ "Introduction",
   body=reebaud(_"Welcome!",
      -- TRANSLATORS: Reebaud – Introduction 1
      _([[Welcome to the beautiful isle of Langeneß! I am Reebaud, the chieftain of this pristine island.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Introduction 1
      _([[My tribe and I have just arrived from the mainland. There were fights with the Danes in the North, the Vikings in the East… So I decided to lead my tribe away to start a quiet, peaceful life on this untouched island.]])),
}
intro_2 = {
   title =_ "Introduction",
   body=reebaud(_"A new home",
      -- TRANSLATORS: Reebaud – Introduction 2
      _([[This is as good a place as any to start building our new home. First of all, we must ensure a stable supply of basic building materials.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Introduction 2
      _([[Like all Frisians, we build our houses mainly with bricks. We improve them by adding some wood and stones. The roofs are thatched with reed, naturally. Nothing is better suited to keep the rain out.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Introduction 2
      _([[Let´s start by taking care of our wood production. Trees don´t seem to grow well on this island; better build at least two houses for foresters and one for a woodcutter.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Introduction 2
      _([[Also, we had better build a reed farm. Even reed farms need reed to be built, so we must take care not to run short before we can grow our own. Reed is easily cultivated, so the reed farmer doesn´t need anything more to start growing reed.]]))
      .. new_objectives(obj_build_wood_economy),
}
intro_3 = {
   title =_ "Introduction",
   body=reebaud(_"Building our now home",
      -- TRANSLATORS: Reebaud – Introduction 3
      _([[Our wood and reed supplies are ensured. Now, we must take care of our brick supply. Bricks are burned in a brick burner´s house out of stone and clay, so we first need to produce those. On the mainland, we used to cut granite out of rocks, but I don´t see any here. Perhaps these hills contain enough stones to mine some granite.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Introduction 3
      _([[Clay is easy to make: A clay burner simply digs up some earth and mixes it with water. Just make sure that he has plenty of space for digging around his clay pit.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Introduction 3
      _([[To burn bricks, the brick burner needs a fire. We use coal as fuel. Build a coal mine to make sure we don´t run out. No coal means no bricks, no bricks means no mines, no mines mean no coal… you can see the problem.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Introduction 3
      _([[Also, build a well – nobody likes to drink salt water, and it is no good for making clay.]]))
      .. new_objectives(obj_build_brick_economy),
}
intro_4 = {
   title =_ "Introduction",
   body=reebaud(_"Our first settlement",
      -- TRANSLATORS: Reebaud – Introduction 4
      _([[Good – our entire building material production works now. Let´s wait until we have gathered some more materials.]])),
}

food_1 = {
   title =_ "Hungry miners",
   body=hauke(_"The miners protest…",
      -- TRANSLATORS: Hauke – no rations left
      _([[Chieftain Reebaud! I bring bad news. The miners are working so hard that they have no time to prepare their own food. They demand to be supplied with it.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – no rations left
      _([[You had better build a tavern to prepare rations. Also, build a fisher´s house, and as nobody likes raw fish, a smokery to smoke it. You should also build a berry farm, which plants berry bushes, and a fruit collector´s house, so the bushes are harvested.]]))
      .. new_objectives(obj_build_food_economy),
}
food_2 = {
   title =_ "Enhancing the food economy",
   body=hauke(_"Feeding the soldiers",
      -- TRANSLATORS: Hauke – better food
      _([[Well done. The miners are satisfied and have started working again.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – better food
      _([[But when the mines have to be expanded, rations won´t keep the miners happy. Better take precautions – build a second tavern and consider enhancing the one you have to a drinking hall.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – better food
      _([[A drinking hall needs better food. Build two farms to start growing barley, a bakery and a brewery.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – better food
      _([[The bakery and brewery should be enhanced as soon as the workers have enough experience. They can then produce mead, and bread sweetened with honey. Build a bee-keeper´s house next to the berry farm to ensure we have enough honey.]]))
      .. new_objectives(obj_build_food_economy_2),
}

mining_1 = {
   title =_ "Build a mining economy",
   body=reebaud(_"Mining economy",
      -- TRANSLATORS: Reebaud – mining economy
      _([[We can now produce all the kinds of food we are used to. I wonder what treasures these hills might contain? We should find out.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – mining economy
      _([[Build an iron mine and a gold mine. As they produce only ores, we will need a furnace to smelt them. Also build a blacksmithy to ensure we won´t run out of tools – and yes, a small armour smithy as well. I hope we won´t meet enemies here, but it´s better to be prepared.]]))
      .. new_objectives(obj_build_mining),
}
expand_1 = {
   title =_ "Discover the island",
   body=reebaud(_"Expanding",
      -- TRANSLATORS: Reebaud – expand
      _([[Our mines are working well. We can now produce all the tools we need, and some weapons as well.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – expand
      _([[But we can´t be sure we´re safe here yet. It would be better to expand further to find out whether we really are alone here.]]))
      .. new_objectives(obj_expand),
}

recruiting_1 = {
   title =_ "Enemies!",
   body=reebaud(_"Enemy sighted!",
      -- TRANSLATORS: Reebaud – recruit soldiers
      _([[Another tribe! On OUR island! What are you doing here, invaders?]])),
}
recruiting_2 = {
   title =_ "Enemies!",
   body=maukor(_"Enemy sighted!",
      -- TRANSLATORS: Maukor – recruit soldiers. The great stormflood of 1362 was supposed to be a sign of the victory of the Christian God over the old northern gods
      _([[This is MY island! YOU are the intruder here! I pray to the one and solely true God that we will kill you all, and your souls shall be tormented in hell for ever!]])),
}
recruiting_3 = {
   title =_ "Enemies!",
   body=reebaud(_"Enemy sighted!",
      -- TRANSLATORS: Reebaud – recruit soldiers
      _([[Unbelievers! I pray to OUR gods that this faithless tribe shall be utterly shattered! War upon them!]])),
}
recruiting_4 = {
   title =_ "Enemies!",
   body=reebaud(_"Enemy sighted!",
      -- TRANSLATORS: Reebaud – recruit soldiers
      _([[We must start recruiting soldiers. Soldiers are recruited in a barracks. They are equipped with a basic sword, produced by the small armour smithy, and fur clothes, the uniform of all honourable warriors.]])
       .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – recruit soldiers
      _([[Build a reindeer farm to start producing fur, and a seamstress to sew the fur into clothes. And of course we must have a barracks.]]))
       .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – recruit soldiers
      _([[The reindeer farm has the advantage that it not only produces fur, and meat as a by-product – it also trains some reindeer to help our carriers on busy roads.]])
      .. new_objectives(obj_recruit_soldiers),
}

training_1 = {
   title =_ "Training Soldiers",
   body=reebaud(_"Stronger in battle",
      -- TRANSLATORS: Reebaud – train soldiers
      _([[We recruit soldiers easily enough, but they are fairly weak. We must train our soldiers if they are to beat the enemy.]])
       .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – train soldiers
      _([[Soldiers are trained in the basics of attack, health and defence by a small training camp. They learn the finer points in a large training arena.]])
       .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – train soldiers
      _([[We will need a master seamstress to sew better armour, and another seamstress to keep producing normal fur clothes. Also, enhance the small armour smithy to produce better weapons and helmets, and build a new small one so we still get more basic weapons.]])
       .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – train soldiers
      _([[I want to have at least one soldier at the front who is trained to the highest possible level in all categories: Level 6 attack, level 2 health and level 2 defence. We do not train soldiers to evade attacks; a true warrior can block his opponent´s blows instead of jumping away like a coward.]]))
      .. new_objectives(obj_train_soldiers),
}
training_2 = {
   title =_ "Training soldiers",
   body=reebaud(_"Our soldiers are strong",
      -- TRANSLATORS: Reebaud – attack enemy
      _([[We finally have out first fully trained soldier. Let´s attack!]]))
      .. new_objectives(obj_defeat_enemy),
}
rising_water_1 = {
   title =_ "Enemy defeated",
   body=reebaud(_"Enemy defeated!",
      -- TRANSLATORS: Reebaud – enemy defeated
      _([[They are defeated! I thank you, all my gods, for granting us victory over the unbelievers!]])),
}
rising_water_2 = {
   title =_ "Enemy defeated",
   body=maukor(_"Enemy defeated!",
      -- TRANSLATORS: Maukor – enemy defeated
      _([[I pray to God to hear my last words. Let not the false fiend triumph! Punish them for this ignoble feat!]])),
}
rising_water_3 = {
   title =_ "Stormflood",
   body=reebaud(_"The island is flooding!",
      -- TRANSLATORS: Reebaud – enemy defeated
      _([[His God has granted his wish! The ocean is rising! The island is almost flat – we will all drown!]])),
}
rising_water_4 = {
   title =_ "Stormflood",
   body=hauke(_"The island is flooding!",
      -- TRANSLATORS: Hauke – rising water
      _([[Despair not, I spy some hope! The easternmost tip of the island seems well suited to build a port on. If we manage to construct a ship, and fill it with all we need to build a new settlement, we may be able to escape.]])
     .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – rising water
      _([[The stormflood seems to be coming from the west. If we hurry, we will just make it! We only need to build a port and a shipyard – oh, and a weaving mill to weave cloth for the sails. Then we launch an expedition from the port. Pray to the gods and make haste!]]))
      .. new_objectives(obj_escape),
}

victory_1 = {
   title =_ "Victory",
   body=reebaud(_"We escaped!",
      -- TRANSLATORS: Reebaud – victory
      _([[The gods have answered our prayers. We will escape the island´s drowning, and I am confident we will be able to build a new home again in a more peaceful corner of this world!]]))
      .. objective_text(_"Congratulations",
      _[[You have completed this mission. You may move on to the next scenario now to help us build a new home, far from the false God´s vengeful reach…]]),
}
