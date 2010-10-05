-- =======================================================================
--                 Texts for the Atlantean tutorial mission
-- =======================================================================

-- =========================
-- Some formating functions
-- =========================
-- Rich Text
function rt(text_or_opts, text)
   k = "<rt>"
   if text then
      k = ("<rt %s>"):format(text_or_opts)
   else
      text = text_or_opts
   end

   return k .. text .. "</rt>"
end

-- Headings
function h1(s)
   return "<p font=FreeSerif font-size=18 font-weight=bold font-color=D1D1D1>"
      ..  s .. "<br></p><p font-size=8> <br></p>"
end

function h2(s)
   return "<p font=FreeSerif font-size=12 font-weight=bold font-color=D1D1D1>"
      ..  s .. "<br></p><p font-size=4> <br></p>"
end

-- Simple flowing text. One Paragraph
function p(s)
   return "<p line-spacing=3 font-size=12>" .. s .. "<br></p>" ..
      "<p font-size=8> <br></p>"
end

-- Direct speech by one of the persons that appear in the map
function speech(img, clr, g_title, g_text)
   local title, text = g_title, g_text
   if not text then
      title = nil
      text = g_title
   end

   local s = ""
   if title then
      s = rt("<p font-size=20 font-weight=bold font-face=FreeSerif " ..
         ("font-color=%s>"):format(clr) .. title ..
         "</p><p font-size=8> <br></p>"
      )
   end

   return s .. rt(("image=%s"):format(img), p(text))
end

function jundlina(title, text)
   return speech("map:princess.png", "2F9131", title, text)
end

function loftomor(text)
   return speech("map:loftomor.png", "FDD53D", "Lofotomor", text)
end
function sidolus(text)
   return speech("map:sidolus.png", "FF1A30", "Sidolus", text)
end
function colionder(text)
   return speech("map:colionder.png", "33A9FD", "Colionder", text)
end

-- Nice formatting for objective texts: A header and one paragraph
-- of text
function obj_text(heading, body)
   return rt(h2(heading) .. p(body))
end

-- Append an objective text to a dialog box in a nice fashion.
function new_objectives(...)
   local s = rt(
   "<p font=FreeSerif font-size=18 font-weight=bold font-color=D1D1D1>"
   .. _"New Objectives" .. "</p>")

   for idx,obj in ipairs{...} do
      s = s .. obj.body
   end
   return s
end

-- =======================================================================
--                                Objectives
-- =======================================================================
obj_ensure_build_wares_production = {
   name = "obj_ensure_build_wares_production",
   title = _ "Ensure the supply of build wares",
   body = obj_text(_"The supply of build wares", _
[[Build a quarry, two woodcutter's houses, two forester's houses and a
 sawmill.]]
   ),
}

obj_expand = {
   name = "obj_expand",
   title = _ "Expand your territory and explore the island",
   body = obj_text(_"Expand and Explore", _
[[The island is huge and as long as we are not sure that we are alone
 here, we cannot relax. Explore and conquer it, this is the
 only way to protect us from threats on the island and from Atlantis.]]
   ),
}

--- TODO: this should be started when build wares are done
obj_make_food_infrastructure = {
   name = "obj_make_food_infrastructure",
   title = _ "Establish a solid food production",
   body = obj_text(_"Establish a food production", _
[[Food is very important for mines and military training areas. Establish
 a well working food environment by building at least one farm, one blackroot
 farm and a mill. The two kinds of flour together with water from a well 
 will be baked to bread in a bakery, so build a bakery and a well too.<br><br>
 The other two important food wares are smoked fish and smoked meat. Raw meat
 is delivered from a hunter. A fisher gets the fish out of the sea while a 
 fish breeder makes sure that a school of fish does not go extinct by breeding
 more. Make sure that there are always fish left, otherwise the fish breeder
 won't be able to breed new ones. The smoking happens in a smokery, you
 will need at least two of those.<br><br>
 - Build a Farm and a Blackroot farm
 - Build a sawmill to make flour and blackroot flour
 - Build a well
 - Build a bakery to bake bread from flour, blackroot flour and water.
 - Build a hunter's house to get raw meat
 - Build a fisher's house close to water to get raw fish
 - Build a fishbreeder's house close to the fisher to make sure the fish do not die out
 - Build two smokeries to smoke raw meat and fish.]]
   )
}

-- =======================================================================
--                                  Texts
-- =======================================================================

-- This is an array or message descriptions
initial_messages = {
{
   title = _ "Proudest to the death",
   body = rt(
      h1(_"Favored by the god") ..
      p(_
[[On the hidden and lost island of Atlantis, a proud tribe settled since the
 world was very young. Ruled by the bloodline of King Ajanthul - the first human
 to be empowered by the sea god Lutas to breath above the sea level - and the
 wise clerics - keeping the connection to Lutas - they prospered and became
 civilized.]]
      ) .. p(
[[This story happens during the regency of King Askandor, the 43th successor of
 King Ajanthul. He has been a good king, ruling Atlantis with wisdom and
 foresight. But with age, he became afraid of dying and so he began looking for
 a cure to death even though most clerics warned him. Some said, endless life
 was only for the gods and the seek for it was forbidden.]]
      )
   ),
},
{
   title = _ "The god's disgrace",
   body = rt(
      h1("The god's punishment") ..
      p(_
[[But all seemed well, and only the horses seemed to feel something.
In the nights, they went crazy and were full of fear. Not before long, the
horsebreeder Xydra figured out what was wrong with them: The sea level before
their stable was rising in an ever accelerating speed.]]
   ) .. p(_
[[The clerics went into their meditation and the reason for the rising water
 was soon to be found: The god lost faith in the atlanteans because of the
 boldness of their king. He decided to withdraw the rights that were
 granted to King Ajanthul and his children. And so, he called them back
 below the sea again.]]
   )
)
},
{
   title = _ "Uproar and confusion",
   body = rt(
      h1("Chaos emerges...") ..
      p(_
[[Guilt ridden, the king suicided. Without monarch, the people turned to the
 clerics. But those had no substantial help to offer: most accepted their
 fate, others tried to change the god's mind by offering animals in his
 temple. But to no avail...]]
   ) .. p(_
[[Jundlina, the late king's daughter and the highest priestess of the god
 was the most determined cleric. As countless offerings didn't change the
 situation, she convinced herself, that the god needed an offer of big
 personal value to be soothed. So she offered him her most beloved:
 her husband, father of her only child.]]
   ) .. p(_
[[But not even this changed the mind of the god. The water kept on rising.
 Nearly driven crazy by guilt, pain and anger, Jundlina became a heretic:
 Secretly, she gathered people of the common folk who were not in line with the
 decision of the clerics to accept the god's will. Together with them, she set
 the temple on fire and stole a ship to flee from the god's influence of
 Atlantis. This small group started praying to Satul, the fire god and the worst
 enemy of Lutas.]]
   ) .. p(_
[[Leaving the dying Atlantis and their past behind, they started a quest to
 find a place sheltered by the fire and protected from the sea.]]
   )
)
},
} -- end of initial messages.

first_briefing_messages = {
{
   title = _ "The princess' memoir",
   body = jundlina(_ "Day 21 after Enlightenment", _
[[We left Atlantis and sailed east. We entered the forbidden sea on the sixth
 day without noticing any chasers from Atlantis and without Lutus having smash
 our ship. Now, we are out of his reach. One day later, we sighted an island
 which seems to have one of these fire spitting mountains on it. I deemed this
 a sign from the fire god and we landed on its shore.]]
   ) .. p(_
[[We spent the last week building two vision towers on the mountains close to
 our landing zone. And of course a hall for us all. We have very talented
 constructors in our group - still, the buildings do not match the art we
 had on Atlantis. I hope they will withstand the next rain. Still, the towers
 will warn us if a ship from Atlantis follows us and if the island is inhabited,
 we will see attackers a long time before they arrive.]]
   ) .. p(_
[[We have established ourselves on this island, the next step is now to make it
 a home. I reckon we need to establish a sustainable economy and explore our
 surroundings. I called for specialist and will follow their advise]]
   )
},
{
   title = _ "Loftomor and Sidolus arrive",
   body = jundlina( "Jundlina", _
[[May Satul warm you both. Loftomor, you have been the islands most renowned
 architect. Sidolus, you are a seasoned warrior and strategist. If have called
 you before me to seek your counsel: What needs to be done to make this our new
 home?]]
   )
},
{
   title = _ "Loftomor speaks",
   body = loftomor(_
[[May Satul warm you, Jundlina! The most important things for building a
 settlement are the build materials. There are some trees here, so we should
 build housings for some woodcutters and of course also for some forester, so
 we do not run out of trees. Oh and we mustn't forget the sawmill, for most
 buildings can't be made out of trunks alone. Stronger buildings also need
 stone, but there is plenty to the north-east of here; we just need to build a
 quarry and my stonemasons will go to work promptly.]]
   ) .. new_objectives(obj_ensure_build_wares_production)
},
{
   title = _ "Sidolus speaks",
   body = sidolus(_
[[May Satul warm you, Jundlina! I agree to what Loftomor proposes. We need
 a good supply of build materials for we have to expand our territory swiflty.
 I will not feel safe on this island as long as we have not seen all shores
 on it. I brought plenty of good men from Atlantis, the military might is
 available, we only need some housings to life in.]]
   ) ..  new_objectives(obj_expand)
}
}

food_story_message = {
{
   title = _ "Jundlina is satisfied",
   body = jundlina("Jundlina", _
[[I just received word that our build infrastructure is well established and
 running good - only spider cloth production is still missing in our arsenal.
 But there seem to be more pressing matters. I called Culionder before me, my
 personal cook to speak some things through with him.]]
   )
},
{
   title = _ "Colionder arrvives",
   body = colionder(_
[[May Satul warm you, Jundlina. Can I be to of any service to you or the
 god?]]
   )
},
{
   title = _ "Jundlina replies",
   body = colionder(_
[[May Satul warm you too, Colionder. Yes, I wondered if there is not a way
 that we can centralize our food production. We loose a lot of productivity
 because our people are accustomed to make their own food. I feel if we
 could split responsibilities, it would be better for us all. I want your
 thoughts on this.]]
   )
},
{
   title = _ "Colionder is in thought",
   body = colionder(_
[[Ahh, but I think this is impossible to change. The preparing and eating of
 food is something deep ingrained in the Atlanteans - it is just a ceremony
 that we need for our well being. So I guess we cannot take this away from 
 the individuals completely. But we might find a compromise in between: For me,
 making break is a troublesome task: TODO malen the blackroot and corn to flour,
 than baking the bread is TODO: langweilig/wiederholend. I feel a more
 industrial approach would be helpful here. I for one would love to just have
 fresh bread delivered to my house every day. Oooh and even more important: the
 smoking of fish and meat to cleanse them and improve their taste is terrible.
 My house is full of smoke and stinks for weeks after it. Don't you think that
 this could be done in a special building were the side effects do not matter?
 I think those two things would be accepted by the people and would reduce the
 cooking times without taking away the ritual.]]
 )
},
{
   title _ "Jundlina agrees",
   body = jundlina(_
[[Your words sound wise to me, Colionder. So be it then.]]
   ) .. new_objectives(obj_make_food_infrastructure)
},
}

-- =======================================================================
--                         Leftover buildings found
-- =======================================================================
first_leftover_building_found = {
{
   title = _ "Strange buildings",
   body = jundlina(_ "I say",
[[Maybe we are not alone here..]]
   )
}
}

second_leftover_building_found = {
{
   title = _ "Strange buildings",
   body = jundlina(_ "I say",
[[Yet another of those]]
   )
}
}

third_leftover_building_found = {
{
   title = _ "Strange buildings",
   body = jundlina(_ "I say",
[[And Yet another of those. But i am confident that we are alone on
this island as we have already conquered and seen so much of it.]]
   )
}
}



