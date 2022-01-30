-- =========================
-- Some formating functions
-- =========================

function reebaud(title, text)
   return speech ("map:reebaud.png", "55BB55", title, text)
end
function hauke(title, text)
   return speech ("map:hauke.png", "7799BB", title, text)
end
function maukor(title, text)
   return speech ("map:maukor.png", "990000", title, text)
end

-- =======================================================================
--                           Texts - No logic here
-- =======================================================================

-- ===========
-- objectives
-- ===========

obj_build_wood_economy = {
   name = "build_wood_economy",
   title=_("Build a wood economy and a reed farm"),
   number = 1,
   body = objective_text(_("Wood Economy"),
      li(_([[Build at least one woodcutter’s house and two forester’s houses to start producing logs, and a reed farm to start growing reed.]]))
   ),
}
obj_build_brick_economy = {
   name = "build_brick_economy",
   title=_("Start producing bricks"),
   number = 1,
   body = objective_text(_("Brick Economy"),
      li(_([[Build a brick kiln, a coal mine, a rock mine, a clay pit and a well to start producing bricks.]]))
   ),
}
obj_gather_materials = {
   name = "gather_materials",
   title=_("Gather building materials"),
   number = 1,
   body = objective_text(_("Stockpile Building Materials"),
      p(_([[Stockpile some building materials in your headquarters. Gather at least:]])) ..
      li(_("30 × Log")) ..
      li(_("30 × Granite")) ..
      li(_("10 × Clay")) ..
      li(_("40 × Brick")) ..
      li(_("20 × Coal")) ..
      li(_("30 × Reed")) ..
      p(_([[Remember that you can set target quantities for most of these wares in your economy settings.]]))
   ),
}
obj_build_food_economy = {
   name = "build_food_economy",
   title=_("Produce food for the miners"),
   number = 1,
   body = objective_text(_("Food Economy"),
      li(_([[Build a tavern, a fisher’s house, a smokery, a berry farm and a fruit collector’s house.]]))
   ),
}
obj_build_food_economy_2 = {
   name = "build_food_economy_2",
   title=_("Enhance your food economy"),
   number = 1,
   body = objective_text(_("Enhance Food Economy"),
      li(_([[Build at least two farms, a beekeeper’s house (next to the berry farm), a bakery, a brewery, and another tavern.]]))
   ),
}
obj_build_mining = {
   name = "build_mining",
   title=_("Build an iron mine and a mining infrastructure"),
   number = 1,
   body = objective_text(_("Build Mines and Mining Infrastructure"),
      li(_([[Build an iron mine, a furnace, a blacksmithy and a small armor smithy.]]))
   ),
}
obj_recruit_soldiers = {
   name = "recruit_soldiers",
   title=_("Start recruiting soldiers"),
   number = 1,
   body = objective_text(_("Recruiting Soldiers"),
      li(_([[Build a barracks, a reindeer farm and a sewing room.]]))
   ),
}
obj_expand = {
   name = "expand",
   title=_("Discover more of the island"),
   number = 1,
   body = objective_text(_("Expand and Discover"),
      li(_([[Expand further and discover more of the island.]]))
   ),
}
obj_train_soldiers = {
   name = "train_soldiers",
   title=_("Train your soldiers"),
   number = 1,
   body = objective_text(_("Training Soldiers"),
      li(_([[Train a soldier to the highest possible level. Soldiers are trained in training camps and training arenas. We will need a gold mine, a tailor’s shop to sew sturdier garments, and a large armor smithy for better weapons and helmets.]]))
   ),
}
obj_defeat_enemy = {
   name = "defeat_enemy",
   title=_("Defeat the enemy!"),
   number = 1,
   body = objective_text(_("Defeat the Enemy!"),
      li(_([[Destroy the enemy tribe’s headquarters.]]))
   ),
}
obj_escape = {
   name = "escape",
   title=_("Escape from the floods"),
   number = 1,
   body = objective_text(_("Flee from the Floods"),
      li(_([[Build a port, a weaving mill and a shipyard, then start an expedition to flee before the storm flood drowns the island.]])) ..
      li_arrow(_([[You can build the port either on the easternmost tip of the island or in the place where the enemy’s headquarters stood.]])) ..
      li_arrow(_([[If you did not prepare well enough to master this challenge in time, you can reload the scenario from the backup savegame automatically created for you shortly before you reached the enemy’s headquarters.]]))
   ),
}

-- ==================
-- Texts to the user
-- ==================

intro_1 = {
   title =_("Introduction"),
   body=reebaud(_("Welcome!"),
      -- TRANSLATORS: Reebaud – Introduction 1
      _([[Welcome to the beautiful isle of Langeness! I am Reebaud, the chieftain of this pristine island.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Introduction 1. Schauenburg and Holstein are dukedoms
      _([[My tribe and I have just arrived from the mainland. There were fights with the Danes in the North, the Schauenburgers and Holsteiners in the East and South… So I decided to lead my tribe away to start a quiet, peaceful life on this untouched island.]])),
}
intro_2 = {
   title =_("Introduction"),
   body=reebaud(_("A new home"),
      -- TRANSLATORS: Reebaud – Introduction 2
      _([[This is as good a place as any to start building our new home. First of all, we must ensure a stable supply of basic building materials.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Introduction 2
      _([[I have asked Hauke, one of my advisors, to oversee the construction of our settlement.]])),
}
intro_3 = {
   title =_("Introduction"),
   body=hauke(_("The first buildings"),
      -- TRANSLATORS: Hauke – Introduction 3. "Peace" is the traditional frisian greeting
      _([[Peace, Chieftain Reebaud! I will start with the colonization of the island straight away.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – Introduction 3
      _([[Like all Frisians, we build our houses mainly with bricks. We improve them by adding some logs and granite. The roofs are thatched with reed, naturally. Nothing is better suited to keep the rain out.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – Introduction 3
      _([[We should start by taking care of our wood production. Trees don’t seem to grow well on this island; better build at least two houses for foresters and one for a woodcutter.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – Introduction 3
      _([[Also, we had better build a reed farm soon. Even reed farms need reed to be built, so we must take care not to run short before we can grow our own. Reed is easily cultivated, so the reed farmer doesn’t need anything more to start growing reed fields.]]))
      .. new_objectives(obj_build_wood_economy),
}
intro_4 = {
   title =_("Introduction"),
   body=hauke(_("Building our new home"),
      -- TRANSLATORS: Hauke – Introduction 4
      _([[Our wood and reed supplies are ensured. Now, we must take care of our brick supply. Bricks are burned in a brick kiln out of granite and clay, so we first need to produce those. On the mainland, we used to cut granite out of rocks, but I don’t see any here. Perhaps these hills contain enough stones to mine some granite.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – Introduction 4
      _([[Clay is easy to make: A clay digger working at a clay pit will simply dig up some earth and mix it with water. Just make sure that he has plenty of space for digging around his clay pit. It might be a good idea to build some sentinels to expand.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – Introduction 4
      _([[To burn bricks, the brick kiln needs a fire. We need to use coal as fuel. Build a coal mine to make sure we don’t run out. No coal means no bricks, no bricks means no mines, no mines mean no coal… you can see the problem.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – Introduction 4
      _([[Also, build a well – nobody likes to drink salt water, and it is no good for making clay.]]))
      .. new_objectives(obj_build_brick_economy),
}
intro_5 = {
   title =_("Introduction"),
   body=reebaud(_("Our first settlement"),
      -- TRANSLATORS: Reebaud – Introduction 5
      _([[You did well. Our entire building material production is working now. Let’s wait until we have gathered some more materials. We could expand our territory some more, to see what else can be found on this island.]]))
      .. new_objectives(obj_gather_materials),
}

warning_no_reed = {
   title =_("Reed Shortages"),
   body=hauke(_("Build another reed farm"),
      -- TRANSLATORS: Hauke – no reed
      _([[I noticed we have been short of reed for a while now. Our buildings seem to need quite a lot of it. We should build another reed farm or two. Reed is also used for making fishing nets and other tools; therefore, resolving this shortage should be a priority.]]))
      .. li_arrow (_([[You should assign the new reed farm the highest priority for reed in the construction site window.]])),
}
warning_no_clay = {
   title =_("Clay Shortages"),
   body=hauke(_("Build another clay pit"),
      -- TRANSLATORS: Hauke – no clay
      _([[Our brickmakers have been complaining about a shortage of clay for a while now. No clay means no bricks can be produced, so this shortage is slowing us down a lot. It would be better to build one or two more clay pits, and wells to supply them with water.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – no clay
      _([[Clay is also used in shipbuilding and to improve charcoal kilns. If we ever need to build a ship, or the mines cannot provide us with enough coal, we will need more clay, so the shortage must be resolved.]])),
}
warning_no_bricks = {
   title =_("Brick Shortages"),
   body=hauke(_("Build another brick kiln"),
      -- TRANSLATORS: Hauke – no bricks
      _([[I noticed we are running out of bricks. This slows down construction of our buildings a lot, since we need so many bricks. We should really start to construct another brick kiln, and assign it the highest priority for bricks in the construction site window so as to resolve the shortage as soon as possible.]]))
      .. li_arrow (_([[You should assign the new brick kiln the highest priority for bricks in the construction site window.]])),
}
warehouse_on_expand = {
   title =_("Warehouses"),
   body=hauke(_("Build a warehouse in the east"),
      -- TRANSLATORS: Hauke – build a warehouse
      _([[We are expanding our territory further and further to the east, and wares need to be transported over longer distances. We should build warehouses near our eastern border in certain distances and preferably store construction materials in the easternmost one. Our headquarters is too far in the west to be useful for long-time ware storage.]])),
}

food_1 = {
   title =_("Hungry Miners"),
   body=hauke(_("The miners are protesting…"),
      -- TRANSLATORS: Hauke – no rations left
      _([[Chieftain Reebaud! I bring bad news. The miners are working so hard that they have no time to prepare their own food. They demand to be supplied with it.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – no rations left
      _([[We need to build a tavern for preparing rations. To get the raw food, we have to build a fisher’s house, and as nobody likes raw fish, a smokery to smoke it. Furthermore, build a berry farm which will plant berry bushes, and a fruit collector’s house so the bushes will be harvested.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – no rations left
      _([[We should start expanding our territory faster in order to have enough space for many berry farms, and a longer coastline for fishing.]]))
      .. new_objectives(obj_build_food_economy),
}
food_2 = {
   title =_("Enhancing the Food Economy"),
   body=hauke(_("Feeding the soldiers"),
      -- TRANSLATORS: Hauke – better food
      _([[Well done. The miners are satisfied and have started working again.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – better food
      _([[But when the mines will have to be enhanced, rations won’t keep the miners happy. Better take precautions – build a second tavern, then consider enhancing one to a drinking hall.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – better food
      _([[A drinking hall needs better food. Build a bakery, a brewery, and two or three farms to start growing barley. Keep in mind that barley grows very slowly; therefore, these few farms won’t suffice when we actually need the bread and beer. We should keep expanding quickly to have more space for farming.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – better food
      _([[The bakery and brewery should be enhanced as soon as the workers have enough experience. They can then produce mead, and bread sweetened with honey. Build a beekeeper’s house next to the berry farm to produce some honey, which we can store until we need it.]]))
      .. new_objectives(obj_build_food_economy_2),
}

mining_1 = {
   title =_("Build a Mining Economy"),
   body=reebaud(_("Mining economy"),
      -- TRANSLATORS: Reebaud – mining economy
      _([[We can now produce all the kinds of food that we are used to. I wonder what treasures these hills might contain? We should find out.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – mining economy
      _([[Build an iron mine and perhaps a gold mine. As they produce only ores, we will need a furnace to smelt them. Also build a blacksmithy to ensure we won’t run out of tools – and yes, a small armor smithy as well. It produces helmets as well as swords. I hope we won’t meet enemies here, but it’s better to be prepared.]]))
      .. new_objectives(obj_build_mining),
}
recruit_1 = {
   title =_("Be Prepared"),
   body=reebaud(_("Start recruiting soldiers"),
      -- TRANSLATORS: Reebaud – recruit 1
      _([[Our mines are working well. We can now produce all the tools we need, and some weapons as well.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – recruit 1
      _([[We could use these weapons to equip some new soldiers. I hope we won’t find enemies here, but it is always better to be prepared. New soldiers are equipped in a barracks with a short sword and a fur garment, the uniform of all honorable warriors.]])),
}
recruit_2 = {
   title =_("Be Prepared"),
   body=hauke(_("Start recruiting soldiers"),
      -- TRANSLATORS: Hauke – recruit 2
      _([[We will need a barracks, of course; a reindeer farm to produce fur, and a sewing room to sew the fur into garments. The reindeer farm produces some meat as a by-product of fur, and it also trains reindeer to help our carriers on busy roads.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – recruit 2
      _([[I advise to build all those buildings within a short distance of each other, and to construct two warehouses nearby: One should be set to store only weapons, armor, ores, metal and coal; the other to store building materials.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – recruit 2
      _([[We should also build other related buildings nearby, such as a furnace, and some wells and farms to supply the reindeer farm. This design will make our military economy more efficient, because it reduces the waiting time for wares in the processing buildings.]]))
      .. new_objectives(obj_recruit_soldiers),
}
expand_1 = {
   title =_("Explore the Island"),
   body=reebaud(_("Expanding"),
      -- TRANSLATORS: Reebaud – expand
      _([[Our barracks has started working, and we are recruiting new soldiers now.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – expand
      _([[But we can’t be sure we’re safe here yet. It would be better to expand further to the east to find out whether we really are alone here.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – expand
      _([[Expanding would also provide more space for farming. We could use some extra barley to recruit more reindeer, as our roads are quite busy.]]))
      .. new_objectives(obj_expand),
}

enemies_1 = {
   title =_("Enemies!"),
   body=reebaud(_("Enemy sighted!"),
      -- TRANSLATORS: Reebaud – enemy sighted
      _([[Another tribe! On OUR island! What are you doing here, invaders?]])),
}
enemies_2 = {
   title =_("Enemies!"),
   body=maukor(_("Enemy sighted!"),
      -- TRANSLATORS: Maukor – enemy sighted. The great storm flood of 1362 was supposed to be a sign of the victory of the Christian God over the old northern gods
      _([[This is MY island! YOU are the intruder here! I pray to the one and solely true God that we will kill you all, and your souls shall be tormented in hell for ever!]])),
}
enemies_3 = {
   title =_("Enemies!"),
   body=reebaud(_("Enemy sighted!"),
      -- TRANSLATORS: Reebaud – enemy sighted
      _([[Unbelievers! I pray to OUR gods that this faithless tribe shall be utterly shattered! War upon them!]])),
}

training_1 = {
   title =_("Training Soldiers"),
   body=reebaud(_("Stronger in battle"),
      -- TRANSLATORS: Reebaud – train soldiers 1
      _([[We are recruiting soldiers easily enough, but they are fairly weak. We must train our soldiers if they are to beat the enemy.]])
       .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – train soldiers 1
      _([[Soldiers are trained in basic attack, health and defense by a training camp. They can learn the finer points in a training arena.]])),
}
training_2 = {
   title =_("Training Soldiers"),
   body=reebaud(_("Frisian fighting"),
      -- TRANSLATORS: Reebaud – train soldiers 2
      _([[Soldiers are trained in two steps in defense and health. They get fur garments studded with iron and also gold to cushion blows; and helmets forged from iron, and gold as well, to lengthen their life in battle.]])
       .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – train soldiers 2
      _([[They learn to attack with three different swords: The long sword, broadsword and double-edged sword. The latter two contain gold, and all of them iron. The advanced soldiers are taught to use two swords at once: They keep their first double-edged sword, and are given various other swords to wield in their other hand. This makes six steps of attack training.]])
       .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – train soldiers 2
      _([[We do not train soldiers to evade attacks; a true warrior can block his opponent’s blows with a skillful twist of the sword instead of jumping away like a coward.]])),
}
training_3 = {
   title =_("Training Soldiers"),
   body=hauke(_("Planning a training center"),
      -- TRANSLATORS: Hauke – train soldiers 3
      _([[We will need a tailor’s shop to sew more protective garments, and another sewing room to keep producing normal fur garments. Also, you should enhance the small armor smithy to produce better weapons and helmets, and build a new small one so we will still get basic weapons. We need at least one gold mine now, because the best weapons and armor use gold.]])
       .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – train soldiers 3
      _([[We should build these buildings close to the barracks, since it is, as I already mentioned earlier, much more efficient to centralize all our military facilities in one recruiting and training center, than if they were strewn out over a long distance. Build a third warehouse there which will store only the food needed for training.]])
       .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – train soldiers 3
      _([[When soldiers train to use new weapons, their old equipment is discarded. Take into account that the useless scraps will clutter up your roads until they are stored in a warehouse somewhere.]])
       .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – train soldiers 3
      _([[I recommend to train at least one soldier to the highest level in health, defense and attack. Until then, the enemy is just too strong for us.]]))
      .. new_objectives(obj_train_soldiers),
}
training_4 = {
   title =_("Training Soldiers"),
   body=reebaud(_("Our soldiers are strong"),
      -- TRANSLATORS: Reebaud – attack enemy 1
      _([[We finally have our first fully trained soldier. Let’s attack!]])),
}
training_5 = {
   title =_("Training Soldiers"),
   body=hauke(_("Are our soldiers strong enough?"),
      -- TRANSLATORS: Hauke – attack enemy 2
      _([[With respect, Chieftain, please be not so hasty! The enemies are very strong, and I’m still not convinced we can defeat them. I’d prefer not to send any soldiers into a battle they will likely lose. I’d prefer to wait a bit longer until we can certainly beat the enemy without losing soldiers.]])),
}
training_6 = {
   title =_("Training Soldiers"),
   body=reebaud(_("Our soldiers are strong enough"),
      -- TRANSLATORS: Reebaud – attack enemy 3
      _([[Of course I also prefer to keep our soldiers alive, but we can’t save every life. Soldiers die in battle, that’s just the way things are. You may decide how long to wait until attacking, but don’t take too long, or the enemies will exploit our weakness.]]))
      .. new_objectives(obj_defeat_enemy),
}
warning_early_attack_1 = {
   title =_("Beware!"),
   body=hauke(_("Not strong enough"),
      -- TRANSLATORS: Hauke – beware of early attacks 1
      _([[Chieftain Reebaud! Our scouts report that the enemy is advancing fast. I fear we are not strong enough yet to defend ourselves if he attacks first. You must devise something to hold him off.]])),
}
warning_early_attack_2 = {
   title =_("Beware!"),
   body=reebaud(_("Not strong enough"),
      -- TRANSLATORS: Reebaud – beware of early attacks 2
      _([[The scouts informed me that the enemy can’t build towers. We could build a fortress, which is the building that will push the border as far east as possible in one go. The enemy will only be able to see it if they build a fortress right at the border.]])
       .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – beware of early attacks 2
      _([[We can prevent this, or at least make it harder to do, by ordering some foresters to plant trees at the border. The enemy will have to cut down many of them to make room for it, so we may be safe until we open the battle at a time of our choosing.]])),
}
rising_water_1 = {
   title =_("Enemy Defeated"),
   body=reebaud(_("Enemy defeated!"),
      -- TRANSLATORS: Reebaud – enemy defeated 1
      _([[They are defeated! I thank you, all my gods, for granting us victory over the unbelievers!]])),
}
rising_water_2 = {
   title =_("Enemy Defeated"),
   body=maukor(_("Enemy defeated!"),
      -- TRANSLATORS: Maukor – enemy defeated 2
      _([[I pray to God to hear my last words. Let not the false fiend triumph! Punish them for this ignoble feat!]])),
}
rising_water_3 = {
   title =_("Storm Flood"),
   body=reebaud(_("The island is flooding!"),
      -- TRANSLATORS: Reebaud – enemy defeated 3
      _([[His God has granted his wish! The ocean is rising! The island is almost flat – we will all drown!]])),
}
rising_water_4 = {
   title =_("Storm Flood"),
   body=hauke(_("The island is flooding!"),
      -- TRANSLATORS: Hauke – rising water
      _([[Despair not, I did espy some hope! The easternmost tip of the island, as well as the space where the fiends’ headquarters stood, seems well suited to build a port on. If we manage to construct a ship, and fill it with all we need to build a new settlement, we may be able to escape.]])
     .. paragraphdivider() ..
      -- TRANSLATORS: Hauke – rising water
      _([[The storm flood seems to be coming from the west. If we hurry, we will just make it! We only need to build a port and a shipyard – oh, and a weaving mill to weave the durable cloth, which is made from reed and fur, for the sails. Then we can launch an expedition from the port. Pray to the gods and make haste!]]))
      .. new_objectives(obj_escape),
}

local nr_transfer_soldiers = {15, 10, 5}
nr_transfer_soldiers = nr_transfer_soldiers[difficulty]
rising_water_5 = {
   title =_("Don’t Forget the Soldiers"),
   body=reebaud(_("Soldiers to the port!"),
      -- TRANSLATORS: Reebaud – rising water
      ngettext([[I almost forgot in this haste – wherever we will land, we’ll need some construction materials to get started, and soldiers to secure and expand our new colony. Even after loading the expedition, the ship should have enough free space for a few wares and workers. On top of that, there should be room for up to %d soldier, if nobody minds being all crammed together. Let’s send some building supplies and our best men to the port when it’s completed, to get ready to embark when we set sail!]],
      [[I almost forgot in this haste – wherever we will land, we’ll need some construction materials to get started, and soldiers to secure and expand our new colony. Even after loading the expedition, the ship should have enough free space for a few wares and workers. On top of that, there should be room for up to %d soldiers, if nobody minds being all crammed together. Let’s send some building supplies and our best men to the port when it’s completed, to get ready to embark when we set sail!]],
      nr_transfer_soldiers):bformat(nr_transfer_soldiers))
      .. li_arrow(_([[All wares and workers that are loaded onto the expedition as well as some of the soldiers in the port will be carried over to the next scenario.]]))
}

victory_1 = {
   title =_("Victory"),
   body=reebaud(_("We escaped!"),
      -- TRANSLATORS: Reebaud – victory
      _([[The gods have answered our prayers. We will escape the island’s drowning, and I am confident we will be able to build a new home again in a more peaceful corner of this world!]]))
      .. objective_text(_("Congratulations"),
      -- TRANSLATORS: Reebaud – victory
      _([[You have completed this mission. You may move on to the next scenario now to help us build a new home, far from the false God’s vengeful reach…]])),
   allow_next_scenario = true,
}
