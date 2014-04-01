-- =========================
-- Some formating functions
-- =========================

include "scripting/formatting.lua"
include "scripting/format_scenario.lua"

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
--                           Texts - No logic here
-- =======================================================================


-- ===========
-- objectives
-- ===========
obj_build_barracks = {
   name = "build_barracks",
   title=_"Build a barracks",
   number = 1,
   body = objective_text(_"Barracks",
[[• ]] .. _[[Build a barracks at the red house symbol on the east side of the forests, to the right of your provisional headquarters.]]),
}

obj_build_lumberjack = {
   name = "build_lumberjack",
   title=_"Build a lumberjack’s house",
   number = 1,
   body = objective_text(_"Lumberjack’s House",
[[• ]] .. _[[Build a lumberjack’s house at the red house symbol, south of your provisional headquarters.]] .. "<br><br>" ..
[[→ ]] .. _([[Cutting down enough trees for more buildings will take a while. ]] ..
[[You can speed the game up by using PAGE UP and slow it down again with PAGE DOWN.]])),
}

obj_build_sawmill_and_lumberjacks = {
   name = "build_sawmill_and_lumberjacks",
   title=_"Build 2 lumberjack’s houses and a sawmill",
   number = 3,
   body = objective_text(_"Two Lumberjack’s Houses and a Sawmill",
[[• ]] .. _[[Build two more lumberjack’s houses and a sawmill as soon as there is enough space for them.]]),
}

obj_build_forester = {
   name = "build_forester",
   title=_"Build a forester’s house",
   number = 1,
   body = objective_text(_"Forester’s House",
[[• ]] .. _[[Build a forester’s house to preserve the wood resources of this island.]]),
}

obj_build_quarry = {
   name = "build_quarry",
   title=_"Build a quarry",
   number = 5,
   body = objective_text(_"Quarry",
[[• ]] .. _[[Build a quarry in the south to cut some stones and marble out of the rocks.]] .. "<br><br>" ..
[[→ ]] .. _[[These might be used for future buildings.]]),
}



-- ==================
-- Texts to the user
-- ==================
diary_page_1 = {
   title =_ "A Dark Night",
   body=lutius(_"Diary of Lutius",
   _([[What has become of our Empire? I really ask this question – why did my king forbid me ]] ..
[[from fighting against that monstrous barbarian tribe, who first acted as a peaceful friend ]] ..
[[and then attacked my army in the darkest night?]]) .. "<br><br>" ..
_([[With an army of 150 men, I was assigned to patrol on our northern frontier, ]] ..
[[which lies near the Galdin Mountains in a great, beautiful and ancient forest. ]] ..
[[Soon, we met a barbarian tribe, which at first was friendly. A few of my men even traded with them ]] ..
[[and their kids came to us, to admire our clean and tidy uniforms and weapons.]]) .. "<br><br>" ..
_([[But during our fifth night in that region, they attacked us with no reason. ]] ..
[[I lost dozens of good men, and found myself imprisoned and brought to their chieftain. ]] ..
[[He spat at me, mocked me and told me to leave this land forever. ]] ..
[[He did not want new land and did not want our land, but he told me that THIS forest was their land]] ..
[[ – and it would stay theirs until the ghosts took the last man of his folk!]]) .. "<br><br>" ..
_([[Three weeks later, I returned to Fremil to speak with our king. ]] ..
[[He was furious at the Barbarians, yet forbade any attack on these tribes. ]] ..
[[He believed that our army was too weak to survive in a war against the barbarians.]]) .. "<br><br>" ..
_([[Still, I saw no way that I could erase my shame without fighting against these barbarians. ]] ..
[[So, I left Fremil by boat to find a new world and a new life for myself, somewhere in the south. ]] ..
[[Now I am sailing on the perl-gulf with my family, some of my friends and a few of my best warriors.]])),
   w=500,
   posy=1,
}

diary_page_2 = {
   title=_"The Rough Sea",
   body= lutius(_"Diary of Lutius",
   _([[It seems as if sailing on the perl-gulf was one of our biggest mistakes. ]] ..
[[Nature was against us and drove us into a dark, wild storm. ]] ..
[[I really don’t know how many hours have passed since the waves rose higher than our boat, ]] ..
[[but still it would be suicide to go outside.]]) .. "<br><br>" ..
_([[Our ship is badly damaged and is taking in more and more water. ]] ..
[[We can thank the Gods if we survive this black night with our lives.]])),
   w=400,
   posy=1,
}

diary_page_3 = {
   title=_"Survivor",
   body= lutius(_"Diary of Lutius",
   _([[Finally, the Gods were with us!]]) .. "<br><br>" ..
_([[We landed on an unknown coast and found peaceful rest under the palm trees growing at the shore. ]] ..
[[But that is about as good as it gets. The truth is: when I woke up this morning, I saw nothing but sand around us.]]) ..
"<br><br>" ..
_([[It really seems as if we have landed under the only palm trees existing in this far-away sandy desert. ]] ..
[[I fear we won’t find the help we need to get our ship repaired in good time.]])),
   w=400,
   posy=1,
}

saledus_1 = {
   title=_"A Foreboding",
   body= saledus(_"Saledus looks around nervously...",
   _([[Sire, I fear we are not safe in this foreign land. ]] ..
[[Who knows what terrible creatures live beyond this forest, in that mighty desert? ]] ..
[[What if these creatures enter the woods and await the time to attack us?]]) .. "<br><br>" ..
_([[Well, perhaps my fear is misguided, but it can’t be wrong to keep watch in the forests]] ..
[[ – so that we can be sure to see any potential enemies before they can see us.]]) .. "<br><br>" ..
_([[We really should build a barracks in the eastern portion of the forest. ]] ..
[[Then one of us can keep watch in the darkness and keep us safe from these creatures.]]))
.. new_objectives(obj_build_barracks),
   w=400,
}


saledus_2 = {
   title=_"Safe For Now",
   body= saledus(_"Saledus speaks with a sigh of relief...",
   _([[Sire, I saw that the construction of the barracks was completed, ]] ..
[[so I have assigned one of my best soldiers to it to keep watch on the desert.]]) .. "<br><br>" ..
_([[This is a good step forward. Now we can feel a bit safer and can look forward to repairing our ship.]])),
   w=400,
}

amalea_1 = {
   title=_"Young Amalea",
   body= amalea(_"Amalea smiles...",
   _([[Hey Lutius, I just met Saledus outside.]]) .. "<br><br>" ..
_([[He told me about the barracks. Well, I am not one of those girls who think we need more barracks]] ..
[[ – instead, we might concentrate on other things now.]]) .. "<br><br>" ..
_([[It is absolutely clear that we need wood to repair our ship. ]] ..
[[So, I walked through the forest yesterday and found a nice place for a lumberjack’s house, ]] ..
[[south of our provisional headquarters.]]))
.. new_objectives(obj_build_lumberjack),
   w=400,
}

amalea_2 = {
   title=_"Young Amalea",
   body= amalea(_"Amalea recommends...",
   _([[Hey Lutius, I got the message that our first lumberjack has started his work today. ]] ..
[[Perhaps it would be a good idea to wait until he cleans enough space for constructing two more lumberjack’s houses, ]] ..
[[so that we can harvest the logs faster.]]) .. "<br><br>" ..
_([[But unfortunately the logs are useless for repairing our ship – we need refined lumber, ]] ..
[[like every ship and every bigger building needs, too. ]] ..
[[So we should build a sawmill – as soon as we have enough space for this.]]))
.. new_objectives(obj_build_sawmill_and_lumberjacks),
   w=400,
}

amalea_3 = {
   title=_"Young Amalea",
   body= amalea(_"Amalea comes in...",
   _([[Hi Lutius, I’ve got two important things to talk about... First the good news:]]) .. "<br><br>" ..
_([[I noticed that the construction of the sawmill is complete, ]] ..
[[so we can begin to refine the logs that the lumberjacks are harvesting into lumber.]]) .. "<br><br>" ..
_([[But that’s just about the only positive thing I know to talk about. ]] ..
[[The bad news is that our lumberjacks harvest at an incredible speed. ]] ..
[[There are almost no trees left on this island.]]) .. "<br><br>" ..
_([[Lutius, these trees provide shelter from the sandstorms that sweep in from the desert ]] ..
[[and shade on the hot days, and they are the home of so many gentle animals. ]] ..
[[We shouldn’t leave this island a complete desert.]]) .. "<br><br>" ..
_([[Lutius, I beg you: find someone who takes care of planting new trees.]]))
.. new_objectives(obj_build_forester),
   w=400,
}

saledus_3 = {
   title=_"Thinking About the Future",
   body= saledus(_"Saledus notes...",
   _([[Hey, my good old friend. I just thought about the rocks standing on the south shore of this land. ]] ..
[[Perhaps we could cut out some useful hard stones and beautiful marble.]]) .. "<br><br>" ..
_([[The repair of our ship will take a few weeks, anyway, and the resources we harvest now ]] ..
[[might be the base of strong and big buildings in another land.]]))
.. new_objectives(obj_build_quarry),
   w=400,
}

saledus_4 = {
   title=_"A Step Forward",
   body= saledus(_"Saledus looks promising...",
   _([[Hey Lutius, today we got a lot closer to our first castle. ]] ..
[[The quarry to the south began its work today and will soon provide us with stones and beautiful marble.]]) .. "<br><br>" ..
_([[Now we truly can look forward to settling down on another island.]])),
   w=400,
}

diary_page_4 = {
   title=_"Mission Complete",
   body= lutius(
_([[Today I got the message that our ship is completely repaired. ]] ..
[[At the moment, my people are loading everything onto our newly repaired ship, ]] ..
[[hoping it will serve us better than last time.]]) .. "<br><br>" ..
_([[Tonight will be our last night on this island. ]] ..
[[Tomorrow morning we will leave, searching for a new place for our exile.]]) .. "<br><br>" ..
_([[I still wonder if we will ever see Fremil again.]])) ..
      objective_text(_"Victory", _
[[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]]
   ),
   posy=1
}

safe_peninsula = {
   title=_"Safe Peninsula",
   body= saledus(_"Saledus speaks:",
   _[[You have made this peninsula a very secure place. It is time to move on!]])
}
