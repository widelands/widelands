-- =========================
-- Some formating functions
-- =========================

use("aux", "formatting")
use("aux", "format_scenario")

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
   name = "build_woodeconomy ",
   title =_"Build up a wood economy",
   number = 5,
   body = objective_text(_"Build up a wood economy", 
_[[* Build three lumberjack's houses, a forester's house and a sawmill.]]),
}

obj_build_quarry = {
   name = "build_quarry ",
   title =_"Build a quarry",
   number = 1,
   body = objective_text(_"Build a quarry", 
_[[* Build a quarry to the south of your headquarters.]]),
}

obj_build_military_buildings = {
   name = "build_military_buildings ",
   title =_"Protect your colony",
   number = 1,
   body = objective_text(_"Protect your colony", 
_[[* Build some barracks and sentries around the colony.]]),
}

obj_build_marblemine = {
   name = "build_marblemine ",
   title =_"Start mining marble",
   number = 1,
   body = objective_text(_"Start mining marble", 
_[[* Expand to the east, to start mining marble from the mountain.]]),
}

obj_build_mining_infrastructure = {
   name = "build_mining_infrastructure ",
   title =_"Build a stonemason and mining infrastructure",
   number = 7,
   body = objective_text(_"Build a stonemason and mining infrastructure", 
_[[* Build a stonemason and then a complete mining and production infrastructure (coal mine and / or charcoal burner, iron ore mine, toolsmith's shop, weapon smithy, armour smithy and smelting works).]]),
}

obj_build_Food_infrastructure = {
   name = "build_food_infrastructure",
   title =_"Provide your miners with food",
   number = 1,
   body = objective_text(_"Provide your miners with food", 
_[[* To produce some sustaining food for our miners, we could build up a tavern. To supply them with some good and strong drinks, we could build up a brewery and a winery.<br>Of course this means we will need more resources for preparing this food – like fish, meat or bread.<br>To provide these foodstuffs, you would have to build a fisher's house, a farm, a mill and a bakery.<br>Maybe you will also need a hunter's house, a piggery, a vineyard and some wells.<br>It's up to you what you want to build.<br>But remember – coal and iron ore mines need beer, marble and gold mines need wine and all mines need at least rations, which are produced out of bread OR meat OR fish.]]),
}

obj_build_bigger_military_buildings = {
   name = "build_bigger_military_buildings ",
   title =_"Protect your eastern frontier",
   number = 1,
   body = objective_text(_"Protect your eastern frontier", 
_[[* Build up stronger military buildings, such as an outpost, a barrier or a tower, on the eastern frontier. To watch deep inside the enemy territory, build a tower.]]),
}

obj_remove_the_barbarians = {
   name = "remove_the_barbarians ",
   title =_"Destroy the barbarian tribe",
   number = 2,
   body = objective_text(_"Destroy the barbarian tribe", 
_[[* As soon as you have enough soldiers, attack and completely destroy the barbarian buildings.]] .. "<br>" ..
_[[* Finally, build up a fortress on the peninsula (near where the barbarian headquarters stood before), to avoid new settlements of other tribes in that region.]]),
}

-- =====================
-- Messages to the User
-- =====================
diary_page_5 = {
   title =_ "A new island",
   posy= 1,
   body = lutius(_"Diary of Lutius",
_[[Finally! This island appears to be made for us, it is a gift from the Gods to my people and myself.<br> Until now, we have only seen a small part of this island, but this part alone is already bringing sunlight in my mind. I feel as if we have found a priceless treasure, a land like paradise.<br> On this island, we will start our new life. We will build up an outpost for our exile, which perhaps will become a new, beautiful home for every one of us.]]),
}

diary_page_5_1 = {
   title =_ "The green forests",
   posy = 1,
   body= lutius(_"Diary of Lutius",
_[[As Saledus and I walked through these wonderful forests in the north, we felt that our future had just begun. This land is so peaceful, good and beautiful, I could stay here until the end of my life.<br> But until that day, may many years pass! For now, we must concentrate on the present, and build a few lumberjack's houses and a sawmill to produce some basic building materials.<br> But we shouldn't forget to preserve this natural bounty. These forests should last forever, so we really should build a forester's house too.]])
.. new_objectives(obj_build_woodeconomy),
}

diary_page_5_2 = {
   title =_ "The rocks",
   posy = 1,
   body= lutius(_"Diary of Lutius",
_[[Later, I walked down to the rocks in the south and looked for a place where we can build a quarry to get some hard stone for our larger buildings.<br> Again, I felt like I'm in paradise when I noticed that some marble-like stones were among the rocks. It seems that we will soon be able to build strong, luxurious buildings, like those we were accustomed to in Fremil.]])
.. new_objectives(obj_build_quarry),
}

diary_page_6 = {
   title =_ "Wood for the colony",
   posy=1,
   body= lutius(_"Diary of Lutius",
_[[Good news! Today our wood economy was completed. Now we can be sure to have enough wood for our future building projects.]]),
}

diary_page_7 = {
   title =_ "Stone for the colony",
   posy=1,
   body= lutius(_"Diary of Lutius",
_[[Great! Today the building of the quarry was completed. Now we get enough stone to construct larger buildings.]]),
}

saledus_1 = {
   title =_ "Dangerous shipparts",
   posy = 1,
   body= saledus(_"Saledus looks unhappy",
_[[Greetings Lutius, I don't want to start a panic, but I found something which gives me a sinking feeling in my stomach and spreads waves of fear in my heart. As I walked down to the southern shore I found the remains of another ship. I don't know whether these parts are all that is left of that ship. In any case, these parts do not seem to be old.<br> Perhaps the people aboard that ship were caught in the same storm which brought us to Malac' Mor – and were brought to this island.<br> I beg you to be cautious and to build some barracks or sentries around our colony.]])
.. new_objectives(obj_build_military_buildings),
}

saledus_2 = {
   title =_ "Marble on the mountain",
   body= saledus(_"Saledus smiles",
_[[Lutius, I've got good news for you:<br> As I walked to the east, I found a larger mountain.<br> I am not absolutely sure – a geologist should check if I am right – but I believe we could mine marble from some places on the mountain.<br> It would be a good source to bolster our supply of quality marble, beyond the meagre quantities available from the quarry. Perhaps you were right when you said that this island is like paradise.<br> Let us expand to that mountain and start mining marble.]])
.. new_objectives(obj_build_marblemine),
}

saledus_3 = {
   title =_ "Further mountains",
   body= saledus(_"Saledus is amazed",
_[[By the Gods, this is an unbelievable strike of fortune! There are two more mountains and it seems that one has a large quantity of coal and the other of iron ore. We should immediately start to build iron ore and coal mines (or at least charcoal burners), smelting works, toolsmith's shops, armour and weapon smithies.<br> However: for all these bigger buildings, we need better and more elegant building materials. So, let's build<br> a stonemason, who cuts columns from marble.<br> After all, now we can begin living as we did in Fremil.]])
.. new_objectives(obj_build_mining_infrastructure),
}

amalea_1 = {
   title =_ "Food for the miners",
   body= amalea(_"Amalea comes in...",
_[[Hi Lutius, I hope you forgive my criticism, but it seems to me as if you forgot something.<br> It's nice, and of course a good idea, to build up mines, which will give us a more comfortable life,<br> but the people working in the mines are unhappy with the current situation. They have to do hard work and have no time for making their own food.<br> What do you think about helping them out?<br> I can't tell you what exactly we need... but here are my recommendations:]])
.. new_objectives(obj_build_Food_infrastructure),
}

diary_page_8 = {
   title =_ "The Barbarians",
   posy=1, 
   body= lutius(_"Diary of Lutius",
_[[By the Gods! This is absolutely terrible. It seems as if we stand close to a test – and it seems as if Saledus was right with his fear concerning the shipwreck he found.<br> Today, as I walked down to the eastern shore,<br> I got a shock. I caught sight of one of those hated, evil, barbarian tribes with whom we have had so many problems before.<br> At first, I was naive and hoped that they were peaceful, but as soon as one of them saw me they charged towards me and started attacking me with their throwing-spears. Thanks be to the Gods that<br> I was able to flee and hide myself, before retreating back to our colony under cover of darkness.<br> Anyway, we must build up stronger military buildings as soon as possible.]])
.. new_objectives(obj_build_bigger_military_buildings),
}

diary_page_9 = {
   title =_ "Command for Attack",
   posy=1,
   body= lutius(_"Diary of Lutius",
_[[That's good. The first large military building is complete and the mining infrastructure is getting better and better. Now we should think of preparing ourselves for battle.]])
.. new_objectives(obj_remove_the_barbarians),
}

diary_page_10 = {
   title =_ "Victory",
   posy=1,
   body= lutius(
_[[Today is a proud day. We have fought for our new home and risen victorious.<br> There are still a few barbarians hiding on our island, but I am sure that we will find them soon.<br> Every barbarian who doesn't attack us will be put in a boat with the other peaceful ones, and sent back to their country.]])
  .. rt("<p font-size=10> <br></p>" ..
      h1(_"Victory")
   ),
}

seven_days_later = {
   title =_ "As time is running by",
   w=200,
   h=150,
   body=rt(p(_"7 days later...")),
}

diary_page_11 = {
   title =_ "A Message",
   posy=1,
   body= lutius(_"Diary of Lutius",
_[[Today a pigeon landed on our island.<br> It brought a message which fills me with dark thoughts and brings back my fears.<br> The message was addressed to me, begging me to come back to Fremil.<br> It says that the Empire is at war with the Barbarians. They were attacked from the north by the tribe I was forbidden to attack.<br> For me, it is clear what I must do...<br><br> You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]]),
}

amalea_2 = {
   title =_ "Our new tavern",
   body= amalea(_"Amalea smiles",
_[[Greetings, my friend, I just visited our new tavern<br> 'At the palms'. The beer they serve is really tasty. You really should take a drink there, too.<br> It is good to see that we have now got a warmer and more familiar environment on our island.<br> Thank you, Lutius.]]),
}

