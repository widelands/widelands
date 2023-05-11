-- =======================================================================
--                 Texts for the Atlantean Mission 2
-- =======================================================================

include "scripting/richtext_scenarios.lua"

function jundlina(title, text)
   return speech("map:princess.png", styles.color("campaign_atl_jundlina"), title, text)
end
function loftomor(text)
   return speech("map:loftomor.png", styles.color("campaign_atl_loftomor"), "Loftomor", text)
end
function sidolus(title, text)
   return speech("map:sidolus.png", styles.color("campaign_atl_sidolus"), title, text)
end
function colionder(text)
   return speech("map:colionder.png", styles.color("campaign_atl_colionder"), "Colionder", text)
end
function opol(text)
   return speech("map:opol.png", styles.color("campaign_atl_opol"), "Opol", text)
end
function ostur(text)
   return speech("map:ostur.png", styles.color("campaign_atl_ostur"), "Ostur", text)
end
function kalitath(text)
   return speech("map:kalitath.png", styles.color("campaign_atl_kalitath"), "Kalitath", text)
end
function current_leader(lead, text)
   if lead == 1 then
      return speech("map:princess.png", styles.color("campaign_atl_jundlina"), name[lead], text)
   else
      return speech("map:sidolus.png", styles.color("campaign_atl_sidolus"), name[lead], text)
   end
end

name = {}
-- TRANSLATORS: To be used in placeholders for switching texts according to current leader
name[1] = _("Jundlina")
-- TRANSLATORS: To be used in placeholders for switching texts according to current leader
name[2] = _("Sidolus")


-- =======================================================================
--                                Objectives
-- =======================================================================

obj_find_port = {
   name = "obj_find_port",
   title = _("Find a place to build a port"),
   number = 1,
   body = objective_text(_("Find Portspace"),
   p(_([[To set foot on this island, we must first find a building space where a port can be built. Explore the nearby coast line to find such a building plot.]])) ..
   p(_([[After the port is built we should cancel the other ship’s expedition, to unload its wares as well.]])))
}

obj_basic_infrastructure = {
   name = "obj_basic_infrastructure",
   title = _("Ensure the supply of construction materials"),
   number = 4,
   body = objective_text(_("Establish Basic Production Chain"),
   p(_([[We will need at least a quarry, a few woodcutters, and a sawmill. A forester might or might not be needed depending on how fast the trees are growing around us. Find the balance.]])))
}

obj_tools = {
   name = "obj_tools",
   title = _("Build tools quickly"),
   number = 1,
   body = objective_text(_("Make New Tools"),
      p(_([[Most of our tools have been lost in the ship tragedy. Our people will not be able to fulfill their duties without proper tools. Luckily we still have some iron ore and plenty of coal. Build a toolsmithy, as it is a prerequisite for success on the island. Don’t forget to build a smelting works as well.]])))
}

obj_mining = {
   name = "obj_mining",
   title = _("Search the mountains for resources"),
   number = 3,
   body = objective_text(_("Build Mines"),
      p(_([[As we are able to make tools again, our supply of mining resources needs to be replenished soon. Explore the mountains and build at least one mine of each type to mine coal, iron and gold.]])))
}

obj_explore = {
   name = "obj_explore",
   title = _("Explore the surroundings"),
   number = 1,
   body = objective_text(_("Explore Your Surroundings"),
   p(_([[We are fairly sure that we are not alone on the island. We do not know if the others on this island are friends or foes, so we need to tread carefully. Explore the island by building at least one house for a scout.]])))
}

obj_expand = {
   name = "obj_expand",
   title = _("Expand your territory"),
   number = 1,
   body = objective_text(_("Expand Territory"),
   p(_([[As our settlement is growing, we will need more space for all buildings. Expand our territory by building military buildings. Build some guardhouses and at least one tower for its larger view range.]])))
}

obj_spidercloth = {
   name = "obj_spidercloth",
   title = _("Build a spider farm and a weaving mill"),
   number = 2,
   body = objective_text(_("Spidercloth Production"),
      p(_([[We need some spidercloth urgently as we are running out of it. It is therefore essential to spare every piece we still have. Reduce the input of all buildings that consume spidercloth, not forgetting the construction sites.]])) ..
      p(_([[Afterwards we need to construct a farm under all circumstances, as it needs spidercloth to be built. Additionally, we need a spider farm and a weaving mill.]])))
}

obj_trading_post = {
   name = "obj_trading_post",
   title = _("Build a trading post"),
   number = 1,
   body = objective_text(_("Upgrade Scout’s House To Trading Post"),
   p(_([[To be able to pay the tributes to Kalitath, and to maintain contact with Jundlina, we need to enhance our scout’s house to a trading post.]])))
}

obj_tribute = {
   name = "obj_tribute",
   title = _("Pay all tributes to Kalitath"),
   number = 4,
   body = objective_text(_("Send Trader To Kalitath"),
   p(_([[We need to send the wares to Kalitath with our trader. He will pick them up as soon as a batch is ready in his trading post.]])) ..
   p(_([[The following batches need to be delivered:]])) ..
   li(_([[Five batches of five logs each.]])) ..
   li(_([[Four batches of four lumps of coal each.]])) ..
   li(_([[Three batches of three iron bars each.]])) ..
   li(_([[Two batches of two gold bars each.]])))
}

obj_tribute2 = {
   name = "obj_tribute2",
   title = _("Pay all tributes to Kalitath"),
   number = 4,
   body = objective_text(_("Send Trader To Kalitath"),
   p(_([[We need to send the wares to Kalitath with our trader. He will pick them up as soon as a batch is ready in his trading post.]])) ..
   p(_([[The following batches need to be delivered:]])) ..
   li(_([[Ten batches of five logs each.]])) ..
   li(_([[Eight batches of four lumps of coal each.]])) ..
   li(_([[Six batches of three iron bars each.]])) ..
   li(_([[Four batches of two gold bars each.]])))
}

obj_defeat_maletus = {
   name = "obj_defeat_maletus",
   title = _("Defeat Maletus"),
   number = 1,
   body = objective_text(_("Defeat Imperial Leader Maletus"),
   p(_([[We need to finish the Empire off to have a chance to build a new home.]])))
}

obj_build_temple = {
   name = "obj_build_temple",
   title = _("Build a temple to worship Satul"),
   number = 1,
   body = objective_text(_("Build Temple For Satul"),
   p(_([[We need to build a temple for Satul and we need to worship him well. By this he might show these Barbarians his might and our princess might come back to her senses.]])) ..
   p(_([[And we need to have a scout’s house to not lose track of what is going on at the Barbarians’ territory.]])))
}

obj_worship = {
   name = "obj_worship",
   title = _("Worship Satul and consecrate priestesses"),
   number = 1,
   body = objective_text(_("Worship Satul"),
   p(_([[Now we need to worship Satul and consecrate at least two priestesses.]])))
}

obj_defeat_kalitath = {
   name = "obj_defeat_kalitath",
   title = _("Defeat Kalitath"),
   number = 1,
   body = objective_text(_("Defeat Kalitath"),
   p(_([[Now it’s time to finish off this annoying Barbarian. He has molested us far too long now.]])))
}

-- =======================================================================
--                                  Texts
-- =======================================================================

defeated = {
   {
      title =_("Defeated!"),
      body = sidolus(_("Sidolus has bad news…"),
         -- TRANSLATORS: Sidolus
         _([[Aaaargh, Jundlina. I don’t know how this could have happened, but Satul couldn’t protect us from the enemy. They have destroyed our last warehouse. Thus we have lost this battle and our people is doomed!]]))
         .. objective_text(_("You Have Lost"),
         _([[You have lost and may not continue playing. May you have better luck when you retry this scenario. Click OK to return to the main menu.]])),
      posy=1,
   }
}

-- This is an array or message descriptions
initial_messages = {
   {
      title = _("The Princess’s Memoir"),
      body = jundlina(_("Jundlina writes her diary"),
         -- TRANSLATORS: Jundlina - Diary
         _([[Our escape from the cursed island was a close one: we managed to load three ships and make our escape, but the currents of the sinking island tested our new ships and their design.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina - Diary
         _([[And one failed the test: the last days of loading was done in chaos and without much planning and so one of our ships ended up carrying most of the heavy wares in our possession.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina - Diary
         _([[The heavy weight plus the strong current was too much for the ship and it was pulled below the sea level by Lutas’ forces and we lost most men and all wares it carried.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina - Diary
         _([[All of us were fiercely afraid. We expected the other ships to be sunken as well, but they persisted. We escaped the island and began our smooth sailing to other coasts.]])),
      h=500,
   },
   {
      title = _("New Hope"),
      body = jundlina(_("Jundlina continues"),
         -- TRANSLATORS: Jundlina - Diary
         _([[Many weeks have passed and we did only see small islands not suitable for habitation and without any sign of being blessed by Satul, the fire god.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina - Diary
         _([[Last night however, we sighted a new island. We saw smoke hovering over it. The black comes from the fiery mountain in the north of the island, the grey however comes from settlers.]])),
      h=500,
   },
   {
      title = _("Planning For The Future"),
      body = jundlina(_("Jundlina concludes"),
         -- TRANSLATORS: Jundlina - Diary
         _([[At least the east side of the island is inhabited, that much is certain. We do not know if the people there are friends or foes. We do not even know what tribe they might be from.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina - Diary
         _([[The only thing we know for sure is that they cannot be Atlanteans, for no ship of ours has sailed as far as we did for centuries. I can only hope they are friendly.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina - Diary
         _([[We have not much water left; our food is running low; we lost all our tools and most of our building materials with the third ship. We can’t afford to sail any further, and we must attempt to settle on this island as close as possible to our current position.]]))
         .. new_objectives(obj_find_port),
      h=500,
   },
} -- end of initial messages.

initial_messages1 = {
   {
      title = _("Jundlina Gives Orders"),
      body = jundlina(_("Jundlina decides"),
         -- TRANSLATORS: Jundlina
         _([[As we founded a port here, we can now plan our settlement. We still have plenty of iron ore, so a toolsmithy and a smelting works will quickly provide us with good tools.]]))
         .. new_objectives(obj_tools),
      h=500,
   },
   {
      title = _("Jundlina Gives Orders"),
      body = jundlina(_("Jundlina demands"),
         -- TRANSLATORS: Jundlina
         _([[However, we need to take care of our building materials: The island is full of trees, we might not even need foresters. But we need stone, that much is certain!]]))
         .. new_objectives(obj_basic_infrastructure),
      h=500,
   },
   {
      title = _("Jundlina Gives Orders"),
      body = jundlina(_("Jundlina orders"),
         -- TRANSLATORS: Jundlina
         _([[Let’s also explore this island and expand our territory. But we need to be careful. We should build at least one tower so that we can see potential enemies before they see us.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina
         _([[But we need to be cautious using the stones and other precious material. When we no longer need a building, we will dismantle it instead of burning it down.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina
         _([[This will take more time, but we can reuse some of its materials. Also, I want a scout deployed and exploring at all times.]]))
         .. new_objectives(obj_explore, obj_expand),
      h=500,
   },
} -- end of initial messages.

function spidercloth_1(leader)
   push_textdomain("scenario_atl02.wmf")
   r = {
      {
         -- TRANSLATORS: placeholder is the name of the current leader (either Jundlina or Sidolus)
         title = _("Opol Reports Serious Shortages"),
         body = opol(
            -- TRANSLATORS: Opol (placeholder is the name of the current leader)
            _([[May Satul warm you, %s. As the highest weaver of the guild I have bad news for you. I just realized that we are running out of spidercloth. This is a most perilous case, for as you know, spidercloth is needed for many buildings.]]):bformat(name[leader])
            .. paragraphdivider() ..
            -- TRANSLATORS: Opol
            _([[We should build up a spidercloth production urgently.]]))
      },
      {
          -- TRANSLATORS: placeholder is the name of the current leader (either Jundlina or Sidolus)
          title = _("%s Replies"):bformat(name[leader]),
          body = current_leader(leader,
            -- TRANSLATORS: either Jundlina or Sidolus depending who is in charge
            _([[May Satul warm you too, Opol. I wanted to delay production of spidercloth, but I understand the urgency. We need to build a complete spidercloth production instantly.]]))
            .. new_objectives(obj_spidercloth)
      }
   }
   pop_textdomain()
   return r
end

function spidercloth_2(leader)
   push_textdomain("scenario_atl02.wmf")
   r = {
      {
         title = _("Opol Makes A Point"),
         body = opol(
            -- TRANSLATORS: Opol (placeholder is the name of the current leader)
            _([[%s! It seems there is another problem even more urgent. I fear we haven’t built a farm to feed our spiders yet. As this is one of the buildings needing spidercloth for completion, not completing one in time will result in a crisis that can never be resolved.]]):bformat(name[leader]))
      },
      {
         -- TRANSLATORS: placeholder is the name of the current leader (either Jundlina or Sidolus)
         title = _("%s Acknowledges"):bformat(name[leader]),
         body = current_leader(leader,
            -- TRANSLATORS: either Jundlina or Sidolus depending who is in charge
            _([[I see the risk, Opol. Thank you for reporting. We will build a farm immediately.]]))
      }
   }
   pop_textdomain()
   return r
end

function spidercloth_3(leader)
   push_textdomain("scenario_atl02.wmf")
   r = {
      {
         -- TRANSLATORS: placeholder is the name of the current leader (either Jundlina or Sidolus)
         title = _("Opol Reports Success"),
         body = opol(
            -- TRANSLATORS: Opol (placeholder is the name of the current leader)
            _([[%s, may Satul warm you! The weaving mill and spider farm are complete and the weavers’ guild can start their work again. I promise we will deliver the finest cloth and some tabards soon. Maybe we can build a gold spinning mill later on to make better tabards as well.]]):bformat(name[leader]))
      },
      {
         -- TRANSLATORS: placeholder is the name of the current leader (either Jundlina or Sidolus)
         title = _("%s Cheers"):bformat(name[leader]),
         body = current_leader(leader,
            -- TRANSLATORS: either Jundlina or Sidolus depending who is in charge
            _([[This is good news indeed, Opol. You may go back to your work now, and may Satul warm you and the weavers’ guild!]]))
      }
   }
   pop_textdomain()
   return r
end

enemy_1 = {
   {
      title = _("Sidolus Reports Foreign Inhabitants"),
      body = sidolus(_("Sidolus"),
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Jundlina! I have some disturbing news. We have sighted land governed by foreigners, but we do not yet know what to expect from them.]]))
   },
   {
      title = _("Jundlina Is Concerned"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[Sidolus, I agree this is very concerning. We need to find out more. Hopefully our scout will get us some information soon.]]))
   }
}

enemy_1a = {
   {
      title = _("Sidolus Reports Foreign Inhabitants"),
      body = sidolus(_("Sidolus"),
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Jundlina! I have some disturbing news. We have sighted land governed by foreigners, but we do not yet know what to expect from them.]]))
   },
   {
      title = _("Jundlina Is Concerned"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[Sidolus, I agree this is very concerning. We need to find out more. We definitely need to send out a scout to gather some information.]]))
   }
}

allies = {
   {
      title = _("Allies?"),
      body = sidolus(_("Sidolus"),
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Jundlina! We have made contact with the foreigners. They are in big trouble as they are at war with a mighty tribe in the north. They ask us for support.]]))
   },
   {
      title = _("Jundlina Is Cautious"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[Sidolus, as we could need any ally we can get, we should help them. Let’s see what we can do for them. However we should improve our own military strength to become independent of others.]]))
   }
}

function infrastructure_1(leader)
   push_textdomain("scenario_atl02.wmf")
   r = {
      {
         title = _("Building Material Ensured"),
         body = loftomor(
            -- TRANSLATORS: Loftomor (placeholder is the name of the current leader)
            _([[May Satul warm you, %s! We have finished our supply of building materials.]]):bformat(name[leader]))
      },
      {
         -- TRANSLATORS: placeholder is the name of the current leader (either Jundlina or Sidolus)
         title = _("%s Cheers"):bformat(name[leader]),
         body = current_leader(leader,
            -- TRANSLATORS: either Jundlina or Sidolus depending who is in charge
            _([[This is good news indeed, Loftomor. Now we can consider constructing more sophisticated buildings to enlarge our economy. Please advise whenever you feel we are missing necessary buildings.]]))
      }
   }
   pop_textdomain()
   return r
end

function tools_available(leader)
   push_textdomain("scenario_atl02.wmf")
   r = {
      {
         title = _("Tool Supply Ensured"),
         body = loftomor(
            -- TRANSLATORS: Loftomor (placeholder is the name of the current leader)
            _([[%s, may Satul warm you! We have made some progress in building up our tool industry.]]):bformat(name[leader]))
      },
      {
         -- TRANSLATORS: placeholder is the name of the current leader (either Jundlina or Sidolus)
         title = _("%s Enchanted"):bformat(name[leader]),
         body = current_leader(leader,
            -- TRANSLATORS: either Jundlina or Sidolus depending who is in charge
            _([[This is good news indeed, Loftomor. This enables us to operate all the buildings we are going to build in our new home. But now we should really look to get some resources from the mountains.]]))
            .. new_objectives(obj_mining)
      }
   }
   pop_textdomain()
   return r
end

function mining_established(leader)
   push_textdomain("scenario_atl02.wmf")
   r = {
      {
         title = _("Mining Established"),
         body = loftomor(
            -- TRANSLATORS: Loftomor (placeholder is the name of the current leader)
            _([[%s, may Satul warm you! Another issue solved. We have found some resources and built a mine of each type.]]):bformat(name[leader]))
      },
      {
         -- TRANSLATORS: placeholder is the name of the current leader (either Jundlina or Sidolus)
         title = _("%s Is Excited"):bformat(name[leader]),
         body = current_leader(leader,
               -- TRANSLATORS: either Jundlina or Sidolus depending who is in charge
            _([[Well done, Loftomor, well done. May Satul warm our strong and brave miners. But now we need to ensure we can feed them. We need to build up bakeries and smokeries and all buildings to supply them.]]))
      }
   }
   pop_textdomain()
   return r
end

maletus_defeated_1a = {
   {
      title = _("Tribute Delivered"),
      body = sidolus(_("Sidolus"),
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Kalitath! We have delivered all the wares you demanded. Now we expect our princess to return home.]]))
   },
   {
      title = _("Kalitath Appreciates Efforts"),
      body = kalitath(
         -- TRANSLATORS: Kalitath
         _([[Sidolus, this is very good news. I appreciate what your people has done. However don’t forget to help us defeat the troops of Maletus. Afterwards we might talk about your princess.]]))
   },
}

maletus_defeated_1 = {
   {
      title = _("Empire Defeated"),
      body = sidolus(_("Sidolus"),
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Kalitath! We have defeated the troops of the Empire in the north. Now we expect our princess to return home.]]))
   },
}

maletus_defeated_2a = {
   {
      title = _("Empire Defeated"),
      body = sidolus(_("Sidolus"),
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Kalitath! We have defeated the troops of the Empire in the north. Now we expect our princess to return home.]]))
   },
   {
      title = _("Kalitath Appreciates Efforts"),
      body = kalitath(
         -- TRANSLATORS: Kalitath
         _([[Sidolus, this is very good news. I appreciate what your people has done. However don’t forget to fulfill your tribute within time. Afterwards we might talk about your princess.]]))
   },
}

maletus_defeated_2 = {
   {
      title = _("Tribute Delivered"),
      body = sidolus(_("Sidolus"),
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Kalitath! We have delivered all the wares you demanded. Now we expect our princess to return home.]]))
   },
}

maletus_defeated_3 = {
   {
      title = _("Kalitath Acknowledges"),
      body = kalitath(
         -- TRANSLATORS: Kalitath
         _([[Sidolus, we appreciate this. But your princess and I already made plans for our common future on this island. She will explain it to you.]]))
   },
   {
      title = _("Jundlina Explains"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[Sidolus, during my stay with the Barbarians I learned a lot about their hospitality and bravery. They really impressed me and their warriors and gods seemed to be very strong in the battle against the Empire. Especially their leader Kalitath made a very good host and I started to think about uniting our tribes to make for a peaceful future on this island. So I want you to prepare our people for a marriage and peaceful coexistence with the Barbarians. This includes learning about their gods and preparing to worship them.]]))
   },
   {
      title = _("Kalitath Demands"),
      body = kalitath(
         -- TRANSLATORS: Kalitath
         _([[Sidolus, now follow the orders of your princess. Meanwhile we might decide that we will need some of the land conquered by you for our common future. So we will simply take your military buildings with our forces. Naturally you won’t have the right to do the same, for the sake of your princess.]]))
   },
}

heretics_again = {
   {
      title = _("Heretics!"),
      body = sidolus(_("Sidolus is very sad"),
         -- TRANSLATORS: Sidolus
         _([[May Satul warm us all. Our princess has been confused in her imprisonment by this Barbarian. But I will never worship any Barbarian god. I can’t imagine how we could change her mind though.]]))
   },
   {
      title = _("Attracting Satul’s Help"),
      body = loftomor(
         -- TRANSLATORS: Loftomor
         _([[Sidolus, I agree we should not bow in front of any other god than Satul. But we might need the god’s help in this case. So I suggest to build a temple and worship Satul with exquisite sacrifices. By this we could inaugurate some priestesses, who might beg Satul to show his powers to the Barbarians and our princess.]]))
   },
   {
      title = _("Build Temple"),
      body = sidolus(_("Sidolus"),
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Loftomor. This is a very good idea indeed. So I task you to build a temple worth worshipping our god. Until we are ready to beg for Satul’s power we should keep an eye on our princess and this Barbarian. As we should stop our trader’s deliveries to not waste any wares to this Barbarian any more, I want you to ensure we have a scout out there to gather information.]]))
         .. new_objectives(obj_build_temple),
   },
}

temple_completed = {
   {
      title = _("Temple Completed"),
      body = loftomor(
         -- TRANSLATORS: Loftomor
         _([[May Satul warm you, Sidolus! My builders just completed the temple for Satul. Now we can start worshipping him and consecrating our priestesses.]]))
   },
   {
      title = _("Worship Satul"),
      body = sidolus(_("Sidolus"),
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Loftomor. Thank you for building such a great temple. Now we need to bring food and our most precious wares to the temple to make for satisfying sacrifices. And don’t forget that the initiates who will serve Satul to become his priestesses need golden tabards and diamonds.]]))
         .. new_objectives(obj_worship),
   },
}

priestesses_consecrated = {
   {
      title = _("May Satul Warm Us"),
      body = sidolus(_("Sidolus"),
         -- TRANSLATORS: Sidolus
         _([[Our priestesses have been consecrated, and we sacrificed our finest goods for Satul’s pleasure. May he warm us and show his might to our princess and this barbaric tribe.]]))
   },
}

princess_back = {
   {
      title = _("Jundlina Arrives"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[May Satul warm you, Sidolus. And may he forgive me my heretic behaviour. The sign of his might and the fear in the Barbarians’ faces suddenly made me recognise the truth again. I was wrong about peace and coexistence, as it would have meant servitude forever. And this is not what Atlanteans are made for.]]))
   },
      {
      title = _("Sidolus Relieved"),
      body = sidolus(_("Sidolus"),
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Jundlina. I am glad you are here again. I hope we may trust you, as much as you will trust and be faithful to Satul. How come the Barbarians let you go?]]))
   },
   {
      title = _("Jundlina Explains"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[Sidolus, they did not let me go. I met our scout and he managed to sneak me out, as I realized they still kept me hostage after I expressed some concerns about our previous agreement. So we need to settle the Barbarian issue now and forever on this island. There is no other way to live here in peace than to defeat them.]]))
         .. new_objectives(obj_defeat_kalitath),
   }
}

victory = {
   {
      allow_next_scenario = true,
      title = _("Jundlina Is Relieved"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[Praise Satul, we have founded a new home. I deeply regret I was weak in my belief. May Satul guide me better in the future.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina
         _([[Now it is time to celebrate but I expect we can’t relax long. We need to build up our society and our capability to explore the seas again.]]))
         .. objective_text(_("Congratulations"),
      _([[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]]))
   }
}

tribute = {
   {
      title = _("Tribute Demand"),
      body = kalitath(
         -- TRANSLATORS: Kalitath
         _([[Strangers, we need to receive some sign of your good intentions. If you are really on our side, you should send us some gold and a couple of other wares on a regular basis.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Kalitath
         _([[We are only interested in wares that can be used to forge weapons. I mean real weapons like our axes, and not the sort of toothpicks you use. So feel free to send us 25 logs, 16 lumps of coal, 9 iron ingots and 4 gold bars.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Kalitath
         _([[Oh, and I almost forgot this one. We demand to have your princess in our headquarters as well to coordinate our war plannings.]]))
         .. new_objectives(obj_tribute)
   },
   {
      title = _("Pitfall!"),
      body = sidolus(_("Sidolus is upset"),
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Jundlina! This behaviour is truly barbaric! I fear this is a trap to keep you as a hostage. And the demand itself is really an insult. I strongly recommend to decline this unsubstantiated claim.]]))
   },
   {
      title = _("Jundlina Concedes"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[Sidolus, I agree with you. However I think we have no choice, so we need to obey this demand. It is your obligation now to run our small people until I come back.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina
         _([[First you need to upgrade our scout’s house to a trading post. Then you need to send the demanded tributes of gold, iron, coal, and logs.]]))
         .. new_objectives(obj_trading_post)
   }
}

trading = {
   {
      title = _("Trading Post Built"),
      body = loftomor(
         -- TRANSLATORS: Loftomor
         _([[Sidolus, may Satul warm you! We have finished the new trading post. I hope this will allow Jundlina to return to us soon.]]))
   },
   {
      title = _("Sidolus Agrees"),
      body = sidolus(_("Sidolus"),
         -- TRANSLATORS: Sidolus
         _([[Well done, Loftomor, well done. May Satul warm our princess and our whole people. Now we need to begin sending goods to this Barbarian.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Sidolus
         _([[I think we should start with the deliveries we could afford the easiest. But we need to find more metals for sure.]]))
   }
}

tribute_not_started = {
   {
      title = _("Urgent Tribute Demand"),
      body = kalitath(
         -- TRANSLATORS: Kalitath
         _([[Strangers, we haven’t received any help yet. If you are really on our side, you should start sending some wares urgently.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Kalitath
         _([[As we are facing great losses due to your laziness, you shall now send us twice the previous amount of each ware.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Kalitath
         _([[I am sure you do not want to break our alliance… do you?]]))
   },
   {
      title = _("Blackmail!"),
      body = sidolus(_("Sidolus is concerned"),
         -- TRANSLATORS: Sidolus
         _([[May Satul warm us all. Our so-called ally is blackmailing us, and we can’t punish him for his insult as he has our princess at his hands.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Sidolus
         _([[Our only chance is to double our efforts and start delivering something soon.]]))
         .. new_objectives(obj_tribute2)
   },
}

tribute_started = {
   {
      title = _("Tribute Started"),
      body = kalitath(
         -- TRANSLATORS: Kalitath
         _([[Strangers, we have received the first batch of wares. So we may finally trust you.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Kalitath
         _([[However, our enemy in the North is very strong. I am afraid that without significant help from your side we might lose the war despite any help in terms of wares you might provide.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Kalitath
         _([[They name themselves an Empire, and their leader is called Maletus. I want you to train your soldiers and help us defeat his lousy Empire.]]))
         .. new_objectives(obj_defeat_maletus)
   },
   {
      title = _("Be Vigilant!"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[May Satul warm you, Sidolus. Our ally now trusts us to some extent, but we shouldn’t trust them too much.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina
         _([[So I want you to defeat this Empire, but ensure that we will become and remain strong enough to enforce the respect we deserve.]]))
   },
   {
      title = _("Keep Delivering"),
      body = kalitath(
         -- TRANSLATORS: Kalitath
         _([[Strangers, I almost forgot to mention, but it should have been clear anyway.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Kalitath
         _([[We are in urgent need of your supply, so hurry up with your deliveries.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Kalitath
         _([[If you fail to deliver everything within three and a half hours, we need to assume you are just playing along with us. So keep an eye on the time as we will not give any further warning.]]))
   },
}

alliance_broken = {
   {
      title = _("Traitors"),
      body = kalitath(
         -- TRANSLATORS: Kalitath
         _([[Strangers, we still haven’t received any help. We are deeply insulted by your treachery.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Kalitath
         _([[At least your princess will make a reasonable slave, if we survive the battle with our enemy.]]))
   },
}

alliance_broken_1 = {
   {
      title = _("Traitors"),
      body = kalitath(
         -- TRANSLATORS: Kalitath
         _([[Strangers, you did not deliver the help we needed in time. We are deeply insulted by your treachery.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Kalitath
         _([[At least your princess will make a reasonable slave, if we survive the battle with our enemy.]]))
   },
}

suicide = {
   {
      title = _("Jundlina’s Suicide"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[Barbarian, I am an Atlantean princess and I will never go into slavery. I have prepared myself for this eventuality and will rather die from the poison I carry with me than be your slave. May Satul avenge my sacrifice.]]))
   },
   {
      title = _("Defeated!"),
      body = sidolus(_("Sidolus has bad news…"),
         -- TRANSLATORS: Sidolus
         _([[Aaaargh. I don’t know how this could have happened, but Satul couldn’t protect us from the enemy. We have lost our noble princess. Thus, we have lost this battle and our people is doomed!]]))
         .. objective_text(_("You Have Lost"),
         _([[You have lost and may not continue playing. May you have better luck when you retry this scenario. Click OK to return to the main menu.]])),
   },
}

kalitath_dead = {
   {
      title = _("Kalitath Defeated"),
      body = sidolus(_("Sidolus has bad news…"),
         -- TRANSLATORS: Sidolus
         _([[Aaaargh. I don’t know how this could have happened, but Satul couldn’t protect our ally from the enemy. He lost his last warehouse and our noble princess perished with him. Thus, we have lost this battle and our people is doomed!]]))
         .. objective_text(_("You Have Lost"),
         _([[You have lost and may not continue playing. May you have better luck when you retry this scenario. Click OK to return to the main menu.]])),
   },
}
