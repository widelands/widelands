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
   return speech("map:loftomor.png", "FDD53D", "Loftomor", text)
end
function sidolus(text)
   return speech("map:sidolus.png", "FF1A30", "Sidolus", text)
end
function colionder(text)
   return speech("map:colionder.png", "33A9FD", "Colionder", text)
end
function opol(text)
   return speech("map:opol.png", "FF3FFB", "Opol", text)
end
function ostur(text)
   return speech("map:ostur.png", "375FFC", "Ostur", text)
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

obj_make_food_infrastructure = {
   name = "obj_make_food_infrastructure",
   title = _ "Establish a solid food production",
   body = obj_text(_"Establish a food production", _
[[Food is very important for mines and military training areas. Establish
a well working food environment by building at least one farm, one blackroot
farm and a mill. The two kinds of flour together with water from a well
will be baked into bread in a bakery, so build a bakery and a well, too.<br><br>
The other two important food wares are smoked fish and smoked meat. Raw meat
is delivered from a hunter. A fisher gets the fish out of the sea while a
fish breeder makes sure that a school of fish does not go extinct by breeding
more. Make sure that there are always fish left, otherwise the fish breeder
won't be able to breed new ones. The smoking happens in a smokery, you
will need at least two of those.<br><br>
- Build a Farm and a Blackroot farm<br>
- Build a mill to make flour and blackroot flour<br>
- Build a well<br>
- Build a bakery to bake bread from flour, blackroot flour and water.<br>
- Build a hunter's house to get raw meat<br>
- Build a fisher's house close to water to get raw fish<br>
- Build a fishbreeder's house close to the fisher to make sure the fish do not die out<br>
- Build two smokeries to smoke raw meat and fish.]]
   )
}

obj_spidercloth_production = {
   name = "obj_spidercloth_production",
   title = _ "Build a spiderfarm and a weaving mill",
   body = obj_text(_"Establish a spidercloth production",
[[The weavers produce spidercloth and tabards in the weaving-mill. Spidercloth
is needed for the construction of some buildings and clothing while tabards
are the uniforms of soldiers. The weaving-mill needs gold yarn and spider yarn
as inputs. Spider yarn is produced by the spiderfarm while gold yarn is
produced by the gold weaver out of gold.<br><br>
- Build a Weaving-Mill<br>
- Build a Spiderfarm<br>
- Build a Goldweaver<br>]]
 )
}

obj_make_heavy_industrie_and_mining = {
   name = "obj_make_heavy_industrie_and_mining",
   title = _ "Build industry and mines",
   body = obj_text(_"Build industry and processing industrie",
[[Iron ore, gold ore and coal are mined in the respective mines. The crystal mine
is digging for crystal, quartz and diamond - all of them are rare materials and
very seldom found. While it searches for them, it produces a lot of stone.<br>
The ores have to be smelted in smelting works before they can be used. The
refined materials are then used in the weapon smithy, the armor smithy and the
tool smithy.<br><br>
- Build a mine of each kind. Make sure to send geologists to the mountain first.<br>
- Build a smelting works.<br>
- Build an armor smithy and a weapon smithy.<br>
- Build a tool smithy.<br>
]])
}

obj_make_training_buildings = {
   name = "obj_make_training_buildings",
   title = _"Build training buildings",
   body = obj_text(_"Build training buildings to improve your soldiers", _
[[As all other tribes, the Atlanteans are also able to train soldiers: the
dungeon trains attack - the major attribute of the Atlanteans - and the
labyrinth trains evasion, health points and defense. The items produced by the
industry are used to train better soldiers in the two training buildings.<br><br>
- Build a dungeon and a labyrinth.<br>]])
}

obj_horsefarm_and_warehouse = {
   name = "obj_horsefarm_and_warehouse",
   title = _"Build a warehouse and a horsefarm",
   body = obj_text(_"Build a warehouse and a horsefarm",
[[As your road network gets longer and more complicated, you should employ
horses to help out your carriers. Horses are bred in horsefarms using water and
corn. A warehouse will also help to ensure your transportation system does not
collapse.]])
}

obj_build_ships = {
   name = "obj_build_ships",
   title = _ "Build 3 ships to escape from the island",
   body = obj_text(_"Escape from the island",
[[There is a lake at the top of the island. Build 3 ships in these waters
and you might be able to rescue your people when the island is swallowed
completely by the ocean. Build a shipyard close to the lake to start
building ships.]]
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
to be empowered by the sea god Lutas to breathe above the sea level - and the
wise clerics, who provided the link to Lutas - they prospered and became
civilized.]]
      ) .. p(
[[This story happens during the regency of King Askandor, the 43rd successor of
King Ajanthul. He has been a good king, ruling Atlantis with wisdom and
foresight. But with age, he became afraid of dying and so he began looking for
a cure to death even though most clerics warned him. Some said, endless life
was only for the gods and to seek for it was forbidden.]]
      )
   ),
},
{
   title = _ "The god's disgrace",
   body = rt(
      h1("The god's punishment") ..
      p(_
[[But all seemed well. Only the horses seemed to feel something was wrong.
In the nights, they went crazy and were full of fear. It was not long before the
horsebreeder Xydra figured out what was wrong with them: The sea level in front
of their stable was rising in an ever accelerating speed.]]
   ) .. p(_
[[The clerics went into their meditation and the reason for the rising water
was soon to be found: The god Lutas had lost faith in the Atlanteans because of
the boldness of their king. He decided to withdraw the rights that were granted
to King Ajanthul and his children. And so, he called them back below the sea
again.]]
   )
)
},
{
   title = _ "Uproar and confusion",
   body = rt(
      h1("Chaos emerges...") ..
      p(_
[[Guilt-ridden, the king committed suicide. Without a monarch, the people
turned to the clerics, but those had no substantial help to offer. Most
accepted their fate while others tried to change the god's mind by offering
animals in his temple. But to no avail...]]
   ) .. p(_
[[Jundlina, the late king's daughter and the highest priestess of the god
was the most determined cleric. As countless offerings didn't change the
situation, she convinced herself that to soothe the god, an offer of great
personal value was needed. So she offered him her most beloved:
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
   body = jundlina(_ "Jundlina writes down her memories", _
[[We left Atlantis and sailed east. We entered the forbidden sea on the sixth
day without noticing any pursuers from Atlantis and without Lutas having smashed
our ship. Now, we are out of his reach. One day later, we sighted an island
which seems to have one of these fire spitting mountains on it. I deemed this
a sign from the fire god and we landed on its shore.]]
   ) .. p(_
[[We spent the last week building two watchtowers on the mountains close to
our landing area; and, of course, a hall for us all. We have very talented
constructors in our group - still, the buildings do not match the art we
had on Atlantis. I hope they will withstand the next rain. At least, the towers
will warn us if a ship from Atlantis follows us and if the island is inhabited,
we will see attackers a long time before they arrive.]]
   ) .. p(_
[[We have established ourselves on this island. The next step is now to make it
a home. I reckon we need to establish a sustainable economy and explore our
surroundings. I called for specialists and will follow their advice.]]
   )
},
{
   title = _ "Loftomor and Sidolus arrive",
   body = jundlina( "Jundlina", _
[[May Satul warm you both. Loftomor, you have been the island's most renowned
architect. Sidolus, you are a seasoned warrior and strategist. I have called
you before me to seek your counsel: What needs to be done to make this our new
home?]]
   )
},
{
   title = _ "Loftomor speaks",
   body = loftomor(_
[[May Satul warm you, Jundlina! The most important things for building a
settlement are the building materials. There are some trees here, so we should
build housings for some woodcutters and of course also for foresters, so
we do not run out of trees. Oh, and we mustn't forget the sawmill, for most
buildings can't be made out of trunks alone. Stronger buildings also need
stone, but there is plenty to the north-east of here; we just need to build a
quarry and my stonemasons will go to work promptly.]]
   ) .. new_objectives(obj_ensure_build_wares_production)
},
{
   title = _ "Sidolus speaks",
   body = sidolus(_
[[May Satul warm you, Jundlina! I agree with what Loftomor proposes. We need
a good supply of building materials for we have to expand our territory swiftly.
I will not feel safe on this island as long as we have not seen all shores
on it. I brought plenty of good men from Atlantis. The military might is
available. We only need some housings to live in.]]
   ) ..  new_objectives(obj_expand)
}
}

food_story_message = {
{
   title = _ "Jundlina is satisfied",
   body = jundlina("Jundlina's Memoirs", _
[[Our building infrastructure is done and I spent some days making sure that
they work well together. Now, there are other pressing matters. I called
Colionder, my personal cook, before me to get his help with sorting some things
out.]]
   )
},
{
   title = _ "Colionder arrvives",
   body = colionder(_
[[May Satul warm you, Jundlina. Can I be of any service to you or the
god?]]
   )
},
{
   title = _ "Jundlina replies",
   body = jundlina(_"Jundlina", _
[[May Satul warm you too, Colionder. Yes, I wondered if there is not a way
that we can centralize our food production. We lose a lot of productivity
because our people are accustomed to making their own food. I feel if we
could split responsibilities, it would be better for us all. I want your
thoughts on this.]]
   )
},
{
   title = _ "Colionder is in thought",
   body = colionder(_
[[Ahh, but I think this is impossible to change. The preparing and eating of
food is something deeply ingrained in us Atlanteans - it is just a ceremony
that we need for our well being. So I guess we cannot take this away from
the individuals completely. But we might find a compromise in between: For me,
making bread is a troublesome task: grinding the blackroot and corn to flour
and then baking the bread is tedious and boring: I feel a more industrial
approach would be helpful here. I for one would love to just have fresh bread
delivered to my house every day. Oooh and even more important: the smoking of
fish and meat to cleanse them and improve their taste is terrible.  My house is
full of smoke and stinks for weeks after it. Don't you think that this could be
done in a special building were the side effects do not matter?  I think those
two things would be accepted by the people and would reduce the cooking times
without taking away the ritual.]]
 )
},
{
   title = _ "Jundlina agrees",
   body = jundlina(_
[[Your words sound wise to me, Colionder. So be it then.]]
   ) .. new_objectives(obj_make_food_infrastructure)
},
}

food_story_ended_messages = {
   {
      title = _ "Jundlina is satisfied",
      body = jundlina(_ "Jundlina is reviewing the reports", _
[[Seems like our food production is finished and everybody is working together
nicely. It is really starting to feel like home here.]]
      )
   }
}

spidercloth_messages = {
   {
      title = _"A man comes to Jundlina",
      body = opol(_
[[May Satul warm you, Jundlina. My Name is Opol and I am the highest weaver of
the guild abandoning Atlantis and Lutas with you. I come with sad news indeed:
We have no more spidercloth. Not a single piece is to be found in our
warehouses.  Could you not help the weaver guild by arranging the building of a
weaving-mill and a spiderfarm? The spiders deliver the finest silk and we will
produce the finest spidercloth from it. We offer to also produce the tabards
for young soldiers and the golden tabards for officers for you in exchange.
You will need them for sure as soon as you want to recruit new soldiers.]])
 },
 {
    title = _ "Jundlina replies",
    body = jundlina(_ "Jundlina replies", _
[[May Satul warm you too, Opol. I wanted to delay production of spider cloth,
but I understand your urgency. Your suggestion sounds fair to me, I will build
your weaving-mill and spiderfarm. I will also build a goldweaver so that the
golden tabards you make will not be golden by name alone.]]
    ) ..  new_objectives(obj_spidercloth_production)
 }
}

spidercloth_story_ended_messages = {
   {
      title = _ "Opol seeks out Jundlina",
      body = opol(_
[[May Satul warm you, Jundlina!. The weaving-mill, goldweaver and spiderfarm
buildings are complete and are starting their work at this very moment. The
weaving guild is very grateful for your support and we will stand by our word
and deliver the first tabard very soon.]]
      )
   },
   {
      title = _ "Jundlina replies",
      body = jundlina(_
[[This is good news indeed, Opol. Our economy is developing very well. Go back
to your work now, Opol, and may Satul warm you and all your guild members!]]
      )
   }
}


heavy_industry_story = {
   {
      title = _ "Sidolus seeks out Jundlina",
      body = sidolus(_
[[May Satul warm you, Jundlina. Have you considered mining for goods in the
mountains around us? We are running short of quartz, crystal and diamonds for
our buildings. Also we could use more iron, gold and especially coal. If we
find an ample supply of those commodities, we could start producing weapons
and tools; this would help enormously. Of course, we would need smelters,
a weapon smithy, an armor smithy and a tool smithy. And maybe even more.]]
      )
   },
   {
      title = _ "Jundlina replies",
      body = jundlina(_"Jundlina nods",
[[I have considered this, Sidolus. I think, it is about time. We will make this
a priority for now. You shall get your industry and soon!]]
      ) .. new_objectives(obj_make_heavy_industrie_and_mining)
   },
}

training_story = {
   {
      title = _ "Jundlina summons Sidolus",
      body = jundlina(_ "Jundlina is angry", _
[[What is this I hear, Sidolus? I received words about your soldiers
misbehaving and bullying the common folk. What is this nonsense all about? They
are servants for the common good and must not misuse their powers. This is
inexcusable! Explain this, if you will!]]
      )
   },
   {
      title = _ "Sidolus seems contrite",
      body = sidolus(_
[[I agree with you and I have punished the trouble makers severely. But it is
very difficult to keep my men quiet: they are bored and have nothing to do. I
would like to ramp up their drill. I think more routine in their life would
keep them occupied and focused on their duties. I would need a dungeon and a
labyrinth for them.  I have the plans ready, but I guess they won't come
cheap.]]
      )
   },
   {
      title = _"Jundlina replies",
      body = jundlina(_
[[I do not fear the cost and labor as long as it keeps your people at bay. You
shall get your buildings. But note that I will not tolerate any rogue soldier;
if similar things happen again, I will make you responsible for them.
Dismissed.]]
      ) .. new_objectives(obj_make_training_buildings)
   }
}

training_story_end = {
   {
      title = _ "Jundlina received a letter",
      body = jundlina("Jundlina", _
[[I received an invitation from Sidolus. The first day of training in the newly
finished labyrinth will be tomorrow. The dungeon is done as well and will start
its work soon. I am looking forward to tomorrow; I am very interested in how our
soldiers are trained and I wonder if the additional workload will keep them from
bullying the civil population.]]
      )
   }
}

horsefarm_and_warehouse_story = {
   {
      title = _ "Jundlina considers the economy",
      body = jundlina(_"Jundlina", _
[[People are complaining about crowded streets and slow transportation. We
need to help out our carriers on the roads. I decided to build a horsefarm
so that the horses can help with the heavy wares.<br>
Another way to take load from our roads is to build warehouses. We have claimed
the mountain now, it seems a good idea to have a warehouse on the plateau to
avoid having to transport everything up and down the slopes.]]
      ) .. new_objectives(obj_horsefarm_and_warehouse)
   }
}

-- =======================================================================
--                         Leftover buildings found
-- =======================================================================
first_leftover_building_found = {
{
   title = _ "A dangerous finding",
   body = jundlina(_ "Jundlina is in thought", _
[[We found an old building, destroyed and burned by flames. I am very worried
about this finding. The building is not of any kind I've ever seen. It is
certainly not designed by any Atlantean architect I've ever heard about. The
building is crude, the assemblage is sloppy. But the materials are very
enduring: the wood used for it seemed to have been burned in a strange way
before it was used as building material - it is hard as stone.<br><br>
The implications frighten me. Are there others on this island? Where are they
then? They work with fire, are they praying to Satul too? Are they friends or
foe? And why is the building burned down? Has this island seen war? Is a
war being waged on it right now?<br><br>
We must improve our military capabilities. If there are foes on the island, we
have to be prepared when we meet them. We should also enforce the foritfication
of our borders.
]]
   )
}
}

second_leftover_building_found = {
{
   title = _ "Another building.",
   body = jundlina(_ "Jundlina is worried", _
[[We found one more building of this strange making. We have yet to see any of
its builders. But obviously there has been a community on this island before
and obviously it has been militarized. I have to check on our military status
with my generals. I will not allow that a potential enemy will find us
unprepared.]]
   )
}
}

third_leftover_building_found = {
{
   title = _ "One more building",
   body = jundlina(_
[[And yet another of these buildings. We know now that the tribe that built
them once had complete control over the island. But I am no longer so scared of
potential enemies. We know so much of this island already and there is no trace
of this old economy to be found. I am starting to relax, but I will not let
our guard down as we continue to explore this beautiful island. But without
obvious dangers I am feeling very much at home here already.]]
   )
}
}

-- =======================
-- Flooding of the island
-- =======================
field_flooded_msg = {
{
   title = _ "The water is rising!",
   body = jundlina(_"Jundlina", _
[[May Satul save us! Lutas is still trying to get to us. A scout informed me
that the ocean is rising quickly. We have to reach higher ground and evacuate
everybody and as much wares from the headquarters up the mountains. How can we
escape?]]
   )
},
{
   title = _ "A young man approaches",
   body = ostur(_
[[May Satul warm you, Jundlina. My name is Ostur and I construct ships. I have
invented a new kind of ship: smaller than those we are used to, but much
sturdier. If we build them correctly, I am confident that we can go with them
a much longer distance and maybe escape from Lutas' influence.
]])
},
{
   title = _ "Jundlina replies",
   body = jundlina(_ "Jundlina", _
[[Ostur, we have no time. The water rises too fast and if we build the ships on
shore they will vanish in the sea before they can float. I fear we are doomed!]]
   ),
},
{
   title = _"Ostur seems confident",
   pre_func = function()
      local lake_field = map:get_field(75,80)
      p1:reveal_fields(lake_field:region(10))
      scroll_smoothly_to(lake_field)
      sleep(200)
   end,
   body = ostur(_
[[A friend of mine explored the island on his own and he told me of a big lake
at the top of the mountain. I suggest we build the ships in this lake and wait
until the island drowns beneath us. Then we can sail away. It is a bold plan,
but I think, it could work.]]
   )
},
{
   title = _ "Jundlina replies",
   body = jundlina(_ [[
Your red hair is a signal from the fire god. I will trust you and support your
plan. Our survival lays in your hands now, Ostur.
<br><br>Swift now, we need a house for Ostur next to the lake and we need
planks, trunks and spidercloth for the construction there. Forget about
everything else, we need those wares on top of the mountain before our
warehouses are all swallowed by the sea.]]
   ) .. new_objectives(obj_build_ships)
}
}

scenario_won = {
   {
      title = _ "Jundlina is releaved",
      body = jundlina(_"Jundlina", _ [[
Praise Satul! Ostur, the young ship builder did it. We have three ships -
never have I seen sturdier ones - with enough room to carry all of us and some
wares too. And this rescue came just in time: Lutas is about to swallow the rest
of this island, the water rises faster by the hour. But we can make our escape
now and start over in some country farther away... I expect a long journey, but
we will find the land of Satul in the end. This is what I promised my people.
And myself.]]
      ) .. rt(h1(_ "Congratulations") .. p(_
[[You have won this mission. Continue with the next one or keep playing for 
as long as you like.]]
      )),
   },
}


