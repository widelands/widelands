-- =======================================================================
--                        Texts only - no logic here
-- =======================================================================

-- ===========
-- Objectives
-- ===========
obj_build_woodeconomy = {
   name = "build_woodeconomy ",
   title =_"Build up a wood economy",
   body ="<rt><p>" ..
_"* Build three lumberjack's houses, a forester's house and a sawmill." ..
"</p></rt>",
}

obj_build_quarry = {
   name = "build_quarry ",
   title =_"Build a quarry",
   body ="<rt><p>" ..
_"* Build a quarry to the south of your headquarters." ..
"</p></rt>",
}

obj_build_military_buildings = {
   name = "build_military_buildings ",
   title =_"Protect your colony",
   body ="<rt><p>" ..
_"* Build some barracks and sentries around the colony." ..
"</p></rt>",
}

obj_build_marblemine = {
   name = "build_marblemine ",
   title =_"Start mining marble",
   body ="<rt><p>" ..
_"* Expand to the east, to start mining marble from the mountain." ..
"</p></rt>",
}

obj_build_mining_infrastructure = {
   name = "build_mining_infrastructure ",
   title =_"Build a stonemason and mining infrastructure",
   body ="<rt><p>" ..
_"* Build a stonemason and then a complete mining and production infrastructure (coal mine and / or charcoal burner, iron ore mine, toolsmith's shop, weapon smithy, armour smithy and smelting works)." ..
"</p></rt>",
}

obj_build_Food_infrastructure = {
   name = "build_food_infrastructure",
   title =_"Provide your miners with food",
   body ="<rt><p>" ..
_"* To produce some sustaining food for our miners, we could build up a tavern. To supply them with some good and strong drinks, we could build up a brewery and a winery.<br> Of course this means we will need more resources for preparing this food - like fish, meat or bread.<br> To provide these foodstuffs, you would have to build a fisher's house, a farm, a mill and a bakery.<br> Maybe you will also need a hunter's house,<br> a piggery, a vineyard and some wells.<br> It's up to you what you want to build.<br> But remember - coal and iron ore mines need beer, marble and gold mines need wine and all mines need at least rations, which are produced out of bread OR meat OR fish." ..
"</p></rt>",
}

obj_build_bigger_military_buildings = {
   name = "build_bigger_military_buildings ",
   title =_"Protect your eastern frontier",
   body ="<rt><p>" ..
_"* Build up stronger military buildings, such as an outpost, a barrier or a tower, on the eastern frontier. To watch deep inside the enemy territory, build a tower." ..
"</p></rt>",
}

obj_remove_the_barbarians = {
   name = "remove_the_barbarians ",
   title =_"Destroy the barbarian tribe",
   body ="<rt><p>" ..
_"* As soon as you have enough soldiers, attack and completely destroy the barbarian buildings.<br>" ..
_"* Finally, build up a fortress on the peninsula<br> (near where the barbarian headquarters stood befre), to avoid new settlements of other tribes in that region." ..
"</p></rt>",
}


-- =====================
-- Messages to the User
-- =====================
diary_page_5 = {
   title =_ "A new island",
   width = 450,
   height = 350,
   posy= 1,
   body = "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Diary of Lutius" ..
"</p></rt><rt image=map:Lutius.png><p line-spacing=3 font-size=12><br>" ..
_[[ "Finally! This island appears to be made for us, it is a gift from the Gods to my people and myself.<br> Until now, we have only seen a small part of this island, but this part alone is already bringing sunlight in my mind. I feel as if we have found a priceless treasure, a land like paradise.<br> On this island, we will start our new life. We will build up an outpost for our exile, which perhaps will become a new, beautiful home for every one of us." ]] ..
"</p></rt>",
}

diary_page_5_1 = {
   title =_ "The green forests",
   width = 450, 
   height = 350,
   posy = 1,
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Diary of Lutius" ..
"</p></rt><rt image=map:Lutius.png><p line-spacing=3 font-size=12><br>" ..
_[[ "As Saledus and I walked through these wonderful forests in the north, we felt that our future had just begun. This land is so peaceful, good and beautiful, I could stay here until the end of my life.<br> But until that day, may many years pass! For now, we must concentrate on the present, and build a few lumberjack's houses and a sawmill to produce some basic building materials.<br> But we shouldn't forget to preserve this natural bounty. These forests should last forever, so we really should build a forester's house too."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build three lumberjack's houses, a forester's house and a sawmill." ..
"</p></rt>",
}

diary_page_5_2 = {
   title =_ "The rocks",
   width = 450,
   height = 350,
   posy = 1,
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Diary of Lutius" ..
"</p></rt><rt image=map:Lutius.png><p line-spacing=3 font-size=12><br>" ..
_[[ "Later, I walked down to the rocks in the south and looked for a place where we can build a quarry to get some hard stone for our larger buildings.<br> Again, I felt like I'm in paradise when I noticed that some marble-like stones were among the rocks. It seems that we will soon be able to build strong, luxurious buildings, like those we were accustomed to in Fremil."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build a quarry to the south of your headquarters." ..
"</p></rt>",
}

diary_page_6 = {
   title =_ "Wood for the colony",
   width=450,
   height=350,
   posy=1,
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Diary of Lutius" ..
"</p></rt><rt image=map:Lutius.png><p line-spacing=3 font-size=12><br>" ..
_[[ "Good news! Today our wood economy was completed. Now we can be sure to have enough wood for our future building projects." ]] ..
"</p></rt>",
}

diary_page_7 = {
   title =_ "Stone for the colony",
   width=450,
   height=350,
   posy=1,
   body="<rt><p font-size=24 text-align=left font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Diary of Lutius" ..
"</p></rt><rt image=map:Lutius.png><p line-spacing=3 font-size=12><br>" ..
_[[ "Great, today the building of the quarry was completed. Now we get enough stone to construct larger buildings." ]] ..
"</p></rt>",
}

saledus_1 = {
   title =_ "Dangerous shipparts",
   width=450,
   height=350,
   posy = 1,
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8F9131>" ..
_"Saledus looks unhappy" ..
"</p></rt><rt image=map:Saledus.png><p line-spacing=3 font-size=12><br>" ..
_[[ "Greetings Lutius, I don't want to start a panic, but I found something which gives me a sinking feeling in my stomach and spreads waves of fear in my heart. As I walked down to the southern strands I found the remains of another ship. I don't know whether these parts are all that is left of that ship. Inn any case: these parts do not seem to be old.<br> Perhaps the people aboard that ship were caught in the same storm which brought us to Malac' Mor - and were brought to this island.<br> I beg you to be cautious and to build some barracks or sentries around our colony."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build some barracks and sentries around the colony." ..
"</p></rt>",
}

saledus_2 = {
   title =_ "Marble on the mountain",
   width=450,
   height=350,
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8F9131>" ..
_"Saledus smiles" ..
"</p></rt><rt image=map:Saledus.png><p line-spacing=3 font-size=12><br>" ..
_[[ "Lutius, I've got good news for you:<br> As I walked to the east, I found a larger mountain.<br> I am not absolutely sure - a geologist should check if I am right - but I believe we could mine marble from some places on the mountain.<br> It would be a good source to bolster our supply of quality marble, beyond the meagre quantities available from the quarry. Perhaps you were right when you said that this island is like paradise.<br> Let us expand to that mountain and start mining marble."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Expand to the east, to start mining marble from the mountain." ..
"</p></rt>",
}

saledus_3 = {
   title =_ "Further mountains",
   width = 450,
   height = 350,
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8F9131>" ..
_"Saledus is amazed" ..
"</p></rt><rt image=map:Saledus.png><p line-spacing=3 font-size=12><br>" ..
_[[ "By the Gods, this is an unbelievable strike of fortune! There are two more mountains and it seems that one has a large quantity of coal and the other of iron ore. We should immediately start to build iron ore and coal mines (or at least charcoal burners), smelting works, toolsmith's shops, armour and weapon smithies.<br> However: for all these bigger buildings, we need better and more elegant building materials. So, let's build<br> a stonemason, who cuts columns from marble.<br> After all, now we can begin living as we did in Fremil."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build a stonemason and then a complete mining and production infrastructure (coal mine and / or charcoal burner, iron ore mine, toolsmith's shop, weapon smithy, armour smithy and smelting works)." ..
"</p></rt>",
}

amalea_1 = {
   title =_ "Food for the miners",
   width=450,
   height=350,
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=AF7511>" ..
_"Amalea comes in..." ..
"</p></rt><rt image=map:Amalea.png><p line-spacing=3 font-size=12><br>" ..
_[[ "Hi Lutius, I hope you forgive my criticism, but it seems to me as if you forgot something.<br> It's nice, and of course a good idea, to build up mines, which will give us a more comfortable life,<br> but the people working in the mines are unhappy with the current situation. They have to do hard work and have no time for making their own food.<br> What do you think about helping them out?<br> I can't tell you what exactly we need... but here are my recommendations:"<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* To produce some sustaining food for our miners, we could build up a tavern. To supply them with some good and strong drinks, we could build up a brewery and a winery.<br> Of course this means we will need more resources for preparing this food - like fish, meat or bread.<br> To provide these foodstuffs, you would have to build a fisher's house, a farm, a mill and a bakery.<br> Maybe you will also need a hunter's house,<br> a piggery, a vineyard and some wells.<br> It's up to you what you want to build.<br> But remember - coal and iron ore mines need beer, marble and gold mines need wine and all mines need at least rations, which are produced out of bread OR meat OR fish." ..
"</p></rt>",
}

diary_page_8 = {
   title =_ "The Barbarians",
   width=450, 
   height=350, 
   posy=1, 
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Diary of Lutius" ..
"</p></rt><rt image=map:Lutius.png><p line-spacing=3 font-size=12><br>" ..
_[[ "By the Gods! This is absolutely terrible. It seems as if we stand close to a test - and it seems as if Saledus was right with his fear concerning the shipwreck he found.<br> Today, as I walked down to the eastern strands,<br> I got a shock. I caught sight of one of those hated, evil, barbarian tribes with whom we have had so many problems before.<br> At first, I was naive and hoped that they were peaceful, but as soon as one of them saw me they charged towards me and started attacking me with their throwing-spears. Thanks be to the Gods that<br> I was able to flee and hide myself, before retreating back to our colony under cover of darkness.<br> Anyway, we must build up stronger military buildings as soon as possible."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build up stronger military buildings, such as an outpost, a barrier or a tower, on the eastern frontier. To watch deep inside the enemy territory, build a tower." ..
"</p></rt>",
}

diary_page_9 = {
   title =_ "Command for Attack",
   width=450,
   height=350,
   posy=1,
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Diary of Lutius" ..
"</p></rt><rt image=map:Lutius.png><p line-spacing=3 font-size=12><br>" ..
_[[ "That's good. The first large military building is complete and the mining infrastructure is getting better and better. Now we should think of preparing ourselves for battle."<br><br>-- NEW OBJECTIVES --<br> ]] ..
_"* As soon as you have enough soldiers, attack and completely destroy the barbarian buildings.<br>" ..
_"* Finally, build up a fortress on the peninsula<br> (near where the barbarian headquarters stood before), to avoid new settlements of other tribes in that region." ..
"</p></rt>",
}

diary_page_10 = {
   title =_ "Victory",
   width=450, 
   height=350,
   posy=1,
   body="<rt image=map:Lutius.png><p line-spacing=3 font-size=12><br>" ..
_[[ "Today is a proud day. We have fought for our new home and risen victorious.<br> There are still a few barbarians hiding on our island, but I am sure that we will find them soon.<br> Every barbarian who doesn't attack us will be put in a boat with the other peaceful ones, and sent back to their country."<br><br>--------------------- VICTORY! ----------------------]] ..
"</p></rt>",
}

seven_days_later = {
   title =_ "As time is running by",
   width=200,
   height=150,
   body="<rt><p line-spacing=3 font-size=12>" ..
_"7 days later..." ..
"</p></rt>",
}

diary_page_11 = {
   title =_ "A Message",
   width=450, 
   height=350,
   posy=1,
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Diary of Lutius" ..
"</p></rt><rt image=map:Lutius.png><p line-spacing=3 font-size=12><br>" ..
_[[ "Today a pigeon landed on our island.<br> It brought a message which fills me with dark thoughts and brings back my fears.<br> The message was addressed to me, begging me to come back to Fremil.<br> It says that the Empire is at war with the Barbarians. They were attacked from the north by the tribe I was forbidden to attack.<br> For me, it is clear what I must do...<br><br> You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission." ]] ..
"</p></rt>",
}

amalea_2 = {
   title =_ "Our new tavern",
   width=450,
   height=350,
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=AF7511>" ..
_"Amalea smiles" ..
"</p></rt><rt image=map:Amalea.png><p line-spacing=3 font-size=12><br>" ..
_[[ "Greetings, my friend, I just visited our new tavern<br> 'At the palms'. The beer they serve is really tasty. You really should take a drink there, too.<br> It is good to see that we have now got a warmer and more familiar environment on our island.<br> Thank you, Lutius." ]] ..
"</p></rt>",
}

