-- =========================
-- Some formating functions
-- =========================

function lutius(title, text)
   return speech("map:Lutius.png", "2F9131", title, text)
end
function saledus(title, text)
   return speech("map:Saledus.png", "8F9131", title, text)
end
function amalea(title, text)
   return speech("map:Amalea.png", "AF7511", title, text)
end


-- =======================================================================
--                        Texts only - no logic here
-- =======================================================================

-- ===========
-- Objectives
-- ===========
obj_build_woodeconomy = {
   name = "build_woodeconomy",
   title =_("Build up a wood economy"),
   number = 5,
   body = objective_text(_("Wood Economy"),
      li(_([[Build three lumberjack’s houses, a forester’s house and a sawmill.]]))
   ),
}

obj_build_quarry = {
   name = "build_quarry",
   title =_("Build a quarry"),
   number = 1,
   body = objective_text(_("Quarry"),
      li(_([[Build a quarry to the south of your headquarters.]]))
   ),
}

obj_build_military_buildings = {
   name = "build_military_buildings",
   title =_("Protect your colony"),
   number = 1,
   body = objective_text(_("Protect Your Colony"),
      li(_([[Build some blockhouses and sentries around the colony.]]))
   ),
}

obj_build_marblemine = {
   name = "build_marblemine",
   title =_("Start mining marble"),
   number = 1,
   body = objective_text(_("Marble"),
      li(_([[Expand to the east, to start mining marble from the mountain.]]))
   ),
}

obj_build_mining_infrastructure = {
   name = "build_mining_infrastructure",
   title =_("Build a stonemason’s house and mining infrastructure"),
   number = 7,
   body = objective_text(_("Stonemason’s house and Mining Infrastructure"),
      li(_([[Build a stonemason’s house and then a complete mining and production infrastructure (coal mine and / or charcoal kiln, iron mine, toolsmithy, weapon smithy, armor smithy and smelting works).]]))
   ),
}

obj_build_food_infrastructure = {
   name = "build_food_infrastructure",
   title =_("Provide your miners with food"),
   number = 1,
   body = objective_text(_("Food For Your Miners"),
      li(_([[Build a tavern to produce some food to sustain our miners.]])) ..
      li_arrow(_([[Of course this means you will need more resources for preparing this food – like fish, meat or bread.]])) ..
      li_arrow(_([[To provide these foodstuffs, you can build a fisher’s house, a hunter’s house, a farm, a mill and a bakery.]])) ..
      li(_([[Build a brewery and a winery to supply the miners with some good and strong drinks.]])) ..
      li_arrow(_([[Maybe you will also need a piggery, a vineyard and some wells.]])) ..
      li_arrow(_([[It’s up to you what you want to build. But remember – coal and iron mines need beer, marble and gold mines need wine and all mines need at least rations, which are produced out of bread OR meat OR fish.]]))
   ),
}

obj_build_bigger_military_buildings = {
   name = "build_bigger_military_buildings",
   title =_("Protect your eastern frontier"),
   number = 1,
   body = objective_text(_("Protect Your Eastern Frontier"),
      li(_([[Build stronger military buildings, such as an outpost, a barrier or a tower, on the eastern frontier.]])) ..
      li_arrow(_([[Build a tower to watch deep inside the enemy territory.]]))
   ),
}

obj_remove_the_barbarians = {
   name = "remove_the_barbarians",
   title =_("Destroy the Barbarian tribe"),
   number = 2,
   body = objective_text(_("Destroy the Barbarian Tribe"),
      li(_([[Build a barracks to start training soldiers.]])) ..
      li(_([[Attack and completely destroy the Barbarian buildings as soon as you have enough soldiers.]])) ..
      li(_([[Build a fortress on the peninsula (near where the Barbarian headquarters stood before). Finally, this will avoid new settlements of other tribes in that region.]]))
   ),
}

-- =====================
-- Messages to the User
-- =====================
diary_page_5 = {
   title =_("A New Island"),
   posy= 1,
   body = lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[Finally! This island appears to have been made for us, it is a gift from the Gods to my people and myself. Until now, we have only seen a small part of this island, but this part alone is already bringing sunlight to my mind. I feel as if we have found a priceless treasure, a land like paradise.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[On this island, we will start our new life. We will build up an outpost for our exile, which perhaps will become a new, beautiful home for every one of us.]])),
}

diary_page_5_1 = {
   title =_("The Green Forests"),
   posy = 1,
   body= lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[As Saledus and I walked through these wonderful forests in the north, we felt that our future had just begun. This land is so peaceful, good and beautiful, I could stay here until the end of my life.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[But until that day, many years may pass! For now, we must concentrate on the present, and build a few lumberjack’s houses and a sawmill to produce some basic building materials.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[But we mustn’t forget to preserve this natural bounty. These forests should last forever, so we really have to build a forester’s house too.]]))
      .. new_objectives(obj_build_woodeconomy)
}

diary_page_5_2 = {
   title =_("The Rocks"),
   posy = 1,
   body= lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[Later, I walked down to the rocks in the south and looked for a place where we could build a quarry to get some granite for our larger buildings.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[Again, I felt like I was in paradise when I noticed that some marble-like stones were among the rocks. It seems that we will soon be able to build strong, luxurious buildings, like those we were accustomed to in Fremil.]]))
      .. new_objectives(obj_build_quarry)
}

diary_page_6 = {
   title =_("Wood for the Colony"),
   posy=1,
   body= lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[Good news! Today our wood economy was completed. Now we can be sure that we will have enough wood for our future building projects.]]))
}

diary_page_7 = {
   title =_("Stones for the Colony"),
   posy=1,
   body= lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[Great! Today the building of the quarry was completed. Now we will get enough granite and marble to construct larger buildings.]])),
}

saledus_1 = {
   title =_("Dangerous Ship Parts"),
   posx = 1,
   posy = 1,
   body= saledus(_("Saledus looks unhappy"),
      -- TRANSLATORS: Saledus
      _([[Sire, I don’t want to start a panic, but I found something which gives me a sinking feeling in my stomach and spreads waves of fear in my heart. As I walked down to the southern shore, I found the remains of another ship. I don’t know whether these parts are all that is left of that ship. In any case, these parts do not seem to be old.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[Perhaps the people aboard that ship were caught in the same storm which brought us to Malac’ Mor – and were brought to this island.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[I beg you to be cautious and to build some blockhouses or sentries around our colony.]]))
      .. new_objectives(obj_build_military_buildings)
}

saledus_2 = {
   posx = 1,
   posy = 1,
   title =_("Marble on the Mountain"),
   body= saledus(_("Saledus smiles"),
      -- TRANSLATORS: Saledus
      _([[Sire, I’ve got good news for you: As I walked to the east, I found a larger mountain. I am not absolutely sure – a geologist should check if I am right – but I believe we could mine marble from some places on the mountain.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[It would be a good source for bolstering our supply of quality marble, beyond the meager quantities available from the quarry. Perhaps you were right when you said that this island was like paradise.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[Please, expand to that mountain and start mining marble.]]))
      .. new_objectives(obj_build_marblemine)
}

saledus_3 = {
   posx = 1,
   posy = 1,
   title =_("Further Mountains"),
   body= saledus(_("Saledus is excited"),
      -- TRANSLATORS: Saledus
      _([[By the Gods, this is an unbelievable stroke of fortune! There are two more mountains, and it seems that one has a large quantity of coal and the other of iron ore. I advise you to immediately build iron and coal mines (or at least charcoal kilns), smelting works, toolsmithies, armor and weapon smithies.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[However: for all these bigger buildings, we need better and more elegant building materials. So you have to build a house for the stonemason, who will cut columns from marble.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[After everything, we can now begin to live like we did in Fremil.]]))
      .. new_objectives(obj_build_mining_infrastructure)
}

amalea_1 = {
   title =_("Food for the Miners"),
   body= amalea(_("Amalea enters…"),
      -- TRANSLATORS: Amalea
      _([[Lutius, don’t you think you forgot something important?]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[It’s nice, and of course a good idea, to build mines, which will give us a more comfortable life, but the people working in the mines are unhappy with the current situation. They have to do hard work and have no time for making their own food.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[What do you think about helping them out? I can’t tell you what we need exactly… but here are my recommendations:]]))
      .. new_objectives(obj_build_food_infrastructure)
}

amalea_2 = {
   title =_("Our New Tavern"),
   body= amalea(_("Amalea smiles"),
      -- TRANSLATORS: Amalea
      _([[I just visited our new tavern ‘At the palms’. The beer they serve is really tasty. You really should have a drink there, too.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[It is good to see that we have now got a warmer and more familiar environment on our island. Thank you, Lutius.]]))
}

diary_page_8 = {
   title =_("The Barbarians"),
   posy=1,
   body= lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[By the Gods! This is absolutely terrible. It seems as if we stand close to a test – and it seems as if Saledus was right with his fear concerning the shipwreck he found.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[Today, as I walked down to the eastern shore, I got a shock. I caught sight of one of those hated, evil, Barbarian tribes with whom we have had so many problems before.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[At first, I was naive and hoped that they were peaceful, but as soon as one of them saw me, they charged towards me and started attacking me with their throwing spears. Thanks be to the Gods that I was able to flee and hide myself, before retreating back to our colony under cover of darkness.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[Anyway, we must build stronger military buildings as soon as possible.]]))
      .. new_objectives(obj_build_bigger_military_buildings)
}

diary_page_9 = {
   title =_("Command to Attack"),
   posy=1,
   body= lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[That’s good. The first big military building is complete and the mining infrastructure is getting better and better. Now we should think about preparing ourselves for battle.]]))
      .. new_objectives(obj_remove_the_barbarians)
}

diary_page_10 = {
   title =_("Victory"),
   posy=1,
   body= lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[Today is a proud day. We have fought for our new home and risen victorious.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[There are still a few Barbarians hiding on our island, but I am sure that we will find them soon. Every Barbarian who doesn’t attack us will be put in a boat with the other peaceful ones, and sent back to their country.]])
      .. paragraphdivider() ..
      _([[We have established a working economy, trained new soldiers and driven the Barbarians from the island. Now we can think about our next steps.]]))
}

seven_days_later = {
   title =_("As Time is Running By"),
   w=200,
   h=150,
   body=p(_("7 days later…")),
}

diary_page_11 = {
   title =_("A Message"),
   posy=1,
   allow_next_scenario = true,
   body= lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[Today a pigeon landed on our island. It brought a message which fills me with dark thoughts and brings back my fears.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[The message was addressed to me, begging me to come back to Fremil. It says that the Empire is at war with the Barbarians. They were attacked from the north by the tribe that I was forbidden to attack. It is clear to me what I must do…]]))
      .. objective_text(_("Victory"),
      _([[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]]))
}
