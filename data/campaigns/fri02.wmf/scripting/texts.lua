-- =========================
-- Some formating functions
-- =========================

function reebaud(title, text)
   return speech("map:reebaud.png", styles.color("campaign_fri_reebaud"), title, text)
end
function hauke(title, text)
   return speech("map:hauke.png", styles.color("campaign_fri_hauke"), title, text)
end
function murilius(title, text)
   return speech("map:murilius.png", styles.color("campaign_fri_murilius"), title, text)
end

-- =======================================================================
--                           Texts - No logic here
-- =======================================================================

-- ===========
-- objectives
-- ===========

obj_new_home = {
   name = "new_home",
   title=_("Build a basic economy"),
   number = 1,
   body = objective_text(_("Supply All Basic Materials"),
      li(_([[Build a quarry, a reed farm, a clay pit, a well, a charcoal kiln, a brick kiln, and houses for a woodcutter and a forester.]]))
   ),
}
obj_expand_south = {
   name = "expand_south",
   title=_("Expand to the south"),
   number = 1,
   body = objective_text(_("Expand Your Territory to Explore the South"),
      li(_([[The north looks not very useful to us, but who knows what we might find in the south?]]))
   ),
}
obj_train_recycle = {
   name = "train_recycle",
   title=_("Train and recycle"),
   number = 1,
   body = objective_text(_("Recycle Weapons to Regain Metal"),
      li(_([[When soldiers train, their old weapons are discarded. They can be turned into metal in a recycling center. Use the weapons in the store to train soldiers and recycle some scrap metal.]])) ..
      li_arrow(_([[The blacksmithy only produces a pick in its second production cycle. If it is supplied with only one iron ingot at a time, it might never reach this cycle. You need to stop the blacksmithy until at least two iron are stored in its input queue.]]))
   ),
}
obj_aqua_farm = {
   name = "aqua_farm",
   title=_("Build an aqua farm"),
   number = 1,
   body = objective_text(_("Build an Aqua Farm to Start Producing Fish"),
      li(_([[Build an aqua farm to produce fish. The aqua farm needs the holes left by a clay pit nearby to use as ponds to grow fish in. Only a fisher working at an aqua farm can fish from these ponds.]]))
   ),
}
obj_supply_murilius = {
   name = "supply_murilius",
   title=_("Decide whether you want to cooperate with the Empire"),
   number = 1,
   body = objective_text(_("Gather Wares as a Gift or Destroy the Empire’s Warehouse"),
      p(_([[Decide whether you want to work with Murilius against the Barbarians. If so, fill his warehouse with these wares:]])) ..
      li(_("30 × Log")) ..
      li(_("40 × Granite")) ..
      li(_("30 × Fish")) ..
      li(_("30 × Beer")) ..
      li(_("150 × Water")) ..
      li(_("40 × Ration")) ..
      li(_("10 × Meal")) ..
      li(_("30 × Coal")) ..
      li(_("40 × Iron Ore")) ..
      li(_("20 × Iron")) ..
      li(_("20 × Gold Ore")) ..
      li(_("10 × Gold")) ..
      -- TRANSLATORS: Continues from "Decide whether you want to work with Murilius against the Barbarians. If so, fill his warehouse with these wares: [list of wares]"
      p(_([[Otherwise, just destroy the warehouse and risk having Murilius as an enemy. He forbade you to expand your territory until the wares have been handed over.]]))
   ),
}
obj_scout = {
   name = "scout",
   title=_("Spy on Murilius"),
   number = 1,
   body = objective_text(_("Send a Scout to Spy on the Empire"),
      li(_([[Build a scout’s house near the border to discover the Empire’s secrets.]]))
   ),
}
obj_poem = {
   name = "poem",
   title=_("Decipher the poem"),
   number = 1,
   body = objective_text(_("Understand the Poem the Scout has Found in the Empire"),
      p(_([[How many sites where soldiers stay,]])) ..
      p(_([[How many sites where thou hold’st sway,]])) ..
      p(_([[Though not their whereabouts sees he]])) ..
      p(_([[Who’s ordering around here thee!]]))
   ),
}
obj_defeat_barbarians = {
   name = "defeat_barbarians",
   title=_("Defeat the Barbarians!"),
   number = 1,
   body = objective_text(_("Defeat the Barbarians"),
      li(_([[Defeat the hostile tribe living northwest of the Empire’s outpost!]]))
   ),
}
obj_defeat_murilius = {
   name = "defeat_murilius",
   title=_("Defeat Murilius!"),
   number = 1,
   body = objective_text(_("Defeat the Arrogant Outpost of the Empire"),
      li(_([[The only way to put an end to the Empire’s desire to steal our liberty is to destroy this outpost once and for all. Conquer its entire land.]])) ..
      li_arrow(_([[Conquer every single field that still belongs to Murilius.]]))
   ),
}
obj_defeat_both = {
   name = "defeat_both",
   title=_("Defeat Murilius and the Barbarians!"),
   number = 1,
   body = objective_text(_("Defeat the Barbarians and the Empire Colony"),
      li(_([[The Barbarians and the Empire are too dangerous to allow them to live near us. We must defeat them both.]]))
   ),
}

-- ==================
-- Texts to the user
-- ==================

-- TRANSLATORS: Reebaud – Introduction 1
local intro_1_start = _([[I began to believe the journey would never end. I have no idea how long we were at sea, tossed about by envious storms… it must have been months.]])
-- TRANSLATORS: Reebaud – Introduction 1
local intro_1_iron = _([[The storms maintained a firm grip on the ship throughout our voyage. There was hardly a day on which we did not wonder whether it would be our last. We were compelled to drop the precious metals we had taken with us overboard to steady the ship, feeling as though it were a squalid sacrifice to the gods to give us hope to live a few hours longer.]])
-- TRANSLATORS: Reebaud – Introduction 1
local intro_1_picks = _([[The few tools we managed to salvage from Langeness did not cope well. The miners’ picks proved especially vulnerable, and we had to throw them in the ocean when it became evident that they had been corroded by the sea salt so much that we could never again mine anything with them.]])
-- TRANSLATORS: Reebaud – Introduction 1
local intro_1_end = _([[But we survived. Finally, we reached the mouth of a calmer fjord. We sailed upriver, and arrived here.]])

local intro_1_body = ""
if lost_metals then intro_1_body = intro_1_iron .. paragraphdivider() end
if lost_miners then intro_1_body = intro_1_body .. intro_1_picks .. paragraphdivider() end
intro_1 = {
   title =_("Welcome Back!"),
   body=reebaud(_("A new home"), intro_1_start .. paragraphdivider() .. intro_1_body .. intro_1_end),
}

intro_2 = {
   title =_("Welcome Back!"),
   body=reebaud(_("A new home"),
      -- TRANSLATORS: Reebaud – Introduction 2
      _([[There was a small group of foreigners living here. They welcomed us by brandishing their axes at us, and shouted in an incoherent language.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Introduction 2
      _([[Our ship was in no state to continue sailing, so we killed the locals and took their land. I just hope there aren’t more of them, so we can live in peace.]])),
}
intro_3 = {
   title =_("Welcome Back!"),
   body=reebaud(_("A new home"),
      -- TRANSLATORS: Reebaud – Introduction 3
      _([[This place looks bleak and barren, my companions say, but I think it looks… beautiful.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Introduction 3
      _([[My scouts no longer report any signs of living creatures nearby. We will start building our new home here.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Introduction 3
      _([[We must start by building all the basic buildings. I see no mountains nearby where we might mine coal. We’d better build a charcoal kiln to burn logs into charcoal. We improve the kilns with clay so they don’t consume so much wood. Our neighbors used to envy us for this clever technique, which they couldn’t copy.]]))
      .. new_objectives(obj_new_home),
}

expand_south_1 = {
   title =_("Go South"),
   body=reebaud(_("Expand and conquer"),
      -- TRANSLATORS: Reebaud – Expand South
      _([[We have built a home in this wonderful landscape. But as our tribe grows, we need more space. We should start expanding our territory.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Expand South
      _([[The scouts reported that the lands to the north are rugged hills, beautiful to behold but not suited for buildings. Let’s find out what we can discover in the south.]]))
      .. new_objectives(obj_expand_south),
}

train_recycle_1 = {
   title =_("Metal Shortage"),
   body=hauke(_("We are out of metals"),
      -- TRANSLATORS: Hauke – Train and recycle 1
      _([[Chieftain Reebaud! We have a serious problem. We have discovered a mountain where we might be able to start mining coal and iron, but we have no miners, nor any picks to equip them with.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – Train and recycle 1
      _([[And we don’t have any iron left. We must find a way to get some metal for forging new picks. Unfortunately, I have no idea how to do this.]])),
}
train_recycle_2 = {
   title =_("Metal Shortage"),
   body=reebaud(_("Getting new metal"),
      -- TRANSLATORS: Reebaud – Train and recycle 2
      _([[This is terrible news! Without iron or the means to produce any, we are stuck like this forever! You must come up with a solution.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Train and recycle 2
      _([[I have seen some advanced weapons in our warehouses. Perhaps it would be possible to smelt them back into metal? But that seems like a waste of the precious swords…]])),
}
train_recycle_3 = {
   title =_("Metal Shortage"),
   body=hauke(_("New metal"),
      -- TRANSLATORS: Hauke – Train and recycle 3
      _([[Now I have an idea: When our soldiers are trained, they are equipped with new swords and the old weapons are discarded. These are unfit to use in battle, so it would not be a waste to smelt down those.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – Train and recycle 3
      _([[That is what we must do. We will use our weapons to train soldiers, then smelt the scrap metal into iron and gold. And then our blacksmiths can produce picks to equip miners with. I will see to it at once!]]))
      .. new_objectives(obj_train_recycle),
}
train_recycle_4 = {
   title =_("Metal Shortage"),
   body=reebaud(_("Getting new metal"),
      -- TRANSLATORS: Reebaud – Train and recycle 4
      _([[Not so hasty! To build a training camp, we will first need gold, and we don’t have any. We must first dismantle our port, which will give us just enough gold for a training camp. I don’t like this, but it is the only way. Of course, we will need to move all our wares to a warehouse before we can dismantle the port.]])),
}

aqua_farm_1 = {
   title =_("Metal Production"),
   body=reebaud(_("The mines are working"),
      -- TRANSLATORS: Reebaud – Aqua Farms 1
      _([[Our mines have started working. Now, we only need to build a furnace and the metal production will be running smoothly.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Aqua Farms 1
      _([[Do we have a reindeer farm already? If not, we should build one now. We need more reindeer for the crowded roads, and we could already start sewing fur garments to equip new soldiers. Who knows whether we will have to fight eventually?]])),
}
aqua_farm_2 = {
   title =_("Fishing"),
   body=hauke(_("We need fish"),
      -- TRANSLATORS: Hauke – Aqua Farms 2
      _([[Chieftain Reebaud, I have more bad news. There are almost no fish in the waters nearby. While we can prepare rations using only fruit or bread, soldier training requires lots of smoked meat or fish. The scraps of meat a reindeer farm drops out as a by-product when making fur aren’t enough.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – Aqua Farms 2
      _([[Fortunately, one of our fishermen thought of a solution. Our clay pits leave large holes in the ground; a fisher could fill them with water and grow fish in these ponds. He will only need lots of water for the ponds, and fruit to feed the fish.]]))
      .. new_objectives(obj_aqua_farm),
}
aqua_farm_3 = {
   title =_("Fishing"),
   body=hauke(_("We have fish again"),
      -- TRANSLATORS: Hauke – Aqua Farms 3
      _([[Well done. We have built an aqua farm, and our mining economy is stable for now. However, the miners tell me that the resources are already low, and the mines may run completely empty soon.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – Aqua Farms 3
      _([[The mines will still be able to produce then, but it will take about twenty times longer – ten times if it is a deep mine – to produce ores or coal. You probably want to prepare for the increasing demands of time and food.]])),
}
aqua_farm_4 = {
   title =_("Recruiting"),
   body=reebaud(_("Our economy is complete"),
      -- TRANSLATORS: Reebaud – Aqua Farms 4
      _([[You are right – we should build more berry farms, fruit collector’s houses and taverns as well as a drinking hall, and advanced bakeries and breweries.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Aqua Farms 4
      _([[However, we can already start focusing on recruiting and training soldiers. We can now afford to use metal for weapons, helmets and garments. I propose building armor smithies, a barracks, a training arena and a tailor’s shop now.]])),
}

supply_murilius_1 = {
   title =_("We Are Not Alone"),
   -- TRANSLATORS: "Greetings, strangers!". DO NOT TRANSLATE. Reebaud doesn’t understand Latin. You can transliterate into non-Latin scripts though
   body=murilius(_("Salvete peregrini!"),
      -- TRANSLATORS: "Be greeted, strangers! I am Proconsule Murilius of the province 'The Northern Lands'. Name yourselves!". DO NOT TRANSLATE. Reebaud doesn’t understand Latin. You can transliterate into non-Latin scripts though
      (_([[Salvete peregrini! Murilius Proconsul Terrae Septentrionalis provinciae ego sum. Vos nominate!]]))),
}
supply_murilius_2 = {
   title =_("We Are Not Alone"),
   body=reebaud(_("Strangers"),
      -- TRANSLATORS: Reebaud – Supply Murilius 2
      _([[…does anyone understand what he is saying?]])),
}
supply_murilius_3 = {
   title =_("We Are Not Alone"),
   body=murilius(_("Salvete peregrini!"),
      -- TRANSLATORS: Murilius – Supply Murilius 3. This is a foreign language to Murilius, so he sounds very formal.
      _([[Do they not even speak the noble language of the Empire?]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Murilius – Supply Murilius 3
      _([[Strangers! I, Murilius, Proconsul of the Northern Lands, a province of the Empire, demand to know who you are and what business you have in these lands!]])),
}
supply_murilius_4 = {
   title =_("We Are Not Alone"),
   body=reebaud(_("Strangers"),
      -- TRANSLATORS: Reebaud – Supply Murilius 4. "Peace" is the traditional Frisian greeting, nothing more.
      _([[Peace, representative of the Empire! I am Reebaud, the chieftain of this tribe. All we want is to settle down here, build our home and live our lives in peace. We do not seek trouble, but we aren’t afraid to fight, either.]])),
}
supply_murilius_5 = {
   title =_("We Are Not Alone"),
   body=murilius(_("Salvete peregrini!"),
      -- TRANSLATORS: Murilius – Supply Murilius 5. "Reebaude" is the Latin vocative case (form of address) of Reebaud.
      _([[In this case, Reebaude, I must inform you of the presence of an inhospitable, hostile tribe of Barbarians in the vicinity of this very outpost of the Empire.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Murilius – Supply Murilius 5
      _([[If you seek to live peacefully here, you are not faced with a choice whether to allow them to live here – you are required to assist the Empire in driving them out.]])),
}
supply_murilius_6 = {
   title =_("We Are Not Alone"),
   body=murilius(_("Salvete peregrini!"),
      -- TRANSLATORS: Murilius – Supply Murilius 6
      _([[Also, as a sign of your good intentions, we demand a gift of friendship to the Empire. As you should have noticed, I have presented you with a new warehouse. Fill it with the required gifts to hand them over. I permit you to give us these wares:]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Murilius – Supply Murilius 6. A list of wares.
      _([[30 logs, 40 slabs of granite, 30 fish, 30 mugs of beer, 150 buckets of water, 40 rations, 10 meals, 30 lumps of coal, 40 lumps of iron ore, 20 iron ingots, 20 lumps of gold ore, and 10 gold bars.]])),
}
supply_murilius_7 = {
   title =_("We Are Not Alone"),
   body=hauke(_("Strangers"),
      -- TRANSLATORS: Hauke – Supply Murilius 7
      _([[Chieftain Reebaud – the demands of this arrogant Empire Proconsul are just outrageous. He treats us like we are his servants. And we certainly cannot afford to spare that many wares to aid him. I propose that we destroy that ugly warehouse and let him fight his own battles.]])),
}
supply_murilius_8 = {
   title =_("We Are Not Alone"),
   body=murilius(_("Salvete peregrini!"),
      -- TRANSLATORS: Murilius – Supply Murilius 8
      _([[If you fail to deliver these wares, the Empire shall treat you like enemies. And so as to prevent you wasting your precious metal, I forbid you to expand your territory from this moment on until you have given us your gift. Fail to obey, and the Empire shall treat you like enemies.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Murilius – Supply Murilius 8
      _([[You will have to fight those Barbarians if you intend to live in the area; surely you can see that fighting them together with the Empire is easier than alone? And you really do not want the Empire as your enemy. Barbarians though you are, I do believe you will realize the truth of my words after some consideration.]]))
      .. new_objectives(obj_supply_murilius),
}
supply_murilius_9 = {
   title =_("We Are Not Alone"),
   body=reebaud(_("Strangers"),
      -- TRANSLATORS: Reebaud – Supply Murilius 9
      _([[This is the greatest outrage of all. How dare he order us to stop expanding our territory when all we want is to build a new home, and threaten us with war if we refuse to be his slaves?]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Supply Murilius 9
      _([[I’d like to say we should attack him right now, but I fear we aren’t strong enough yet to hold our own against him in battle. We should hurry our soldier training while pretending to be gathering his so-called gift. That would buy us some time.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Supply Murilius 9
      _([[As for expansion – I wonder how strictly he can possibly control us. Perhaps we should send a scout to spy on him…]]))
      .. new_objectives(obj_scout),
}

local indent = styles.get_size("campaign_fri02_poem_indent")

expansion_hint = {
   title =_("The Scout Discovered Something"),
   body=hauke(_("Our scout returned"),
      -- TRANSLATORS: Hauke – Expansion hint
      _([[Chieftain Reebaud! Our scout just returned. He has found a poem in the strange tongue this Proconsul speaks. With some difficulty, I have translated it into our language. If I did not mistranslate, it runs as follows:]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – Expansion hint. A poem, verse 1
      space(indent) .. (_([[How many sites where soldiers stay,]]))
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – Expansion hint. A poem, verse 2
      space(indent) .. (_([[How many sites where thou hold’st sway,]]))
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – Expansion hint. A poem, verse 3
      space(indent) .. (_([[Though not their whereabouts sees he]]))
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – Expansion hint. A poem, verse 4
      space(indent) .. (_([[Who’s ordering around here thee!]]))
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – Expansion hint
      _([[I don’t know what it means, but I’m sure it’s important. Perhaps you can make some sense of it.]]))
      .. new_objectives(obj_poem),
}

supply_murilius_destroy = {
   title =_("The Empire is Angry"),
   -- TRANSLATORS: "You are enemies of the Empire!". DO NOT TRANSLATE. Reebaud doesn’t understand Latin. You can transliterate into non-Latin scripts though
   body=murilius(_("Hostes Imperii estis!"),
      -- TRANSLATORS: Murilius – enemy of the Empire: player destroyed the warehouse
      _([[Barbarians! You have refused my offer of friendship most ungraciously by destroying the beautiful building we presented you with!]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Murilius – enemy of the Empire
      _([[You are enemies of the Empire, and I shall treat you as such!]])),
}
supply_murilius_military = {
   title =_("The Empire is Angry"),
   body=murilius(_("Hostes Imperii estis!"),
      -- TRANSLATORS: Murilius – enemy of the Empire: player built forbidden military buildings
      _([[Barbarians! You have refused my offer of friendship most ungraciously by basely wasting your precious resources on expansion!]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Murilius – enemy of the Empire
      _([[You are enemies of the Empire, and I shall treat you as such!]])),
}
defeat_both = {
   title =_("Two Enemies"),
   body=reebaud(_("Defeat the enemies!"),
      -- TRANSLATORS: Reebaud – defeat both enemies
      _([[Murilius has made it quite clear that he won’t leave us alone. I fear we must defeat him and destroy his colony if we ever want to have peace here.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – defeat both enemies
      _([[The Barbarian tribe he mentioned is also a threat. I don’t want to share this land with anyone, as sharing only leads to conflicts and battles, as we saw in our old home. We must defeat the Barbarians as well.]]))
      .. new_objectives(obj_defeat_both),
}
supply_murilius_thanks = {
   title =_("The Empire is Grateful"),
   -- TRANSLATORS: "Greetings, friends of the Empire!". DO NOT TRANSLATE. Reebaud doesn’t understand Latin. You can transliterate into non-Latin scripts though
   body=murilius(_("Salvete amici Imperii!"),
      -- TRANSLATORS: Murilius – Supplied Murilius
      _([[Strangers! I see you have kept your word. The Empire rewards its friends, therefore I shall reward you with the friendship of the Empire!]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Murilius – Supplied Murilius. Murilius’s plan is to use the player as cannon fodder, while he himself only attacks when he has no choice
      _([[Now I permit you to assist us in conquering the Barbarians, who live in a sheltered valley northwest to us. I charge you, expand along the northern end of the mountains enclosing this outpost; you will find the Barbarians there – attack them! We will fend off attacks launched by the enemy.]]))
      .. new_objectives(obj_defeat_barbarians),
}
defeat_murilius_1 = {
   title =_("The Reward For Your Service"),
   -- TRANSLATORS: "Friends of the Empire". DO NOT TRANSLATE. Reebaud doesn’t understand Latin. You can transliterate into non-Latin scripts though
   body=murilius(_("Amici Imperii"),
      -- TRANSLATORS: Murilius – defeat Murilius 1
      _([[You have done well, strangers! As I did tell you earlier, you will be rewarded with the friendship of the Empire!]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Murilius – defeat Murilius 1
      _([[I propose you start by sending legates to the Capital, so as to discover the most recent political activities. Once there, you can receive instructions for your tribe from the Senate. Also, you will need to find out what tribute the Capital will demand from you, and of course, who shall be king of your tribe.]])),
}
defeat_murilius_2 = {
   title =_("Another Enemy"),
   body=reebaud(_("Defeat the Empire!"),
      -- TRANSLATORS: Reebaud – defeat Murilius 2
      _([[Did I understand him correctly? After all we have done for him, after so many of my soldiers have laid down their lives for his sake, he wants us to be servants of this Empire, and take away my leadership? And does the hypocrite dare call this a reward?]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – defeat Murilius 2
      _([[We will punish this self-proclaimed Proconsul for his treachery! I shall burn down his entire colony, and kill every single person in his realm. He will be very sorry indeed for insulting our tribe thus.]]))
      .. new_objectives(obj_defeat_murilius),
}
victory_1 = {
   title =_("Victory"),
   body=reebaud(_("Victory!"),
      -- TRANSLATORS: Reebaud – victory
      _([[We have defeated all our enemies! Now no other tribe will ever dare settle here again. We are safe, and we can finally live our lives in peace.]]))
      .. objective_text(_("Congratulations"),
      _([[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]])),
   allow_next_scenario = true,
}
