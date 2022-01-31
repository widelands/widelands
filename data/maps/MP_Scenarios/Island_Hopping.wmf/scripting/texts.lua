-- =======================================================================
--                          Texts for this scenario
-- =======================================================================
welcome_msg = {
   heading = _("Welcome to Island Hopping"),
   body =
      h2(_("Rules")) ..
      p(_(
   [[Island Hopping is a traditional tournament in Atlantean culture. ]] ..
   [[The rules of the game are simple: you start with a headquarters on an island. ]] ..
   [[When you finish a castle at the end of the first island, you are granted a ]] ..
   [[second headquarters on a second island, which will contain all wares from your ]] ..
   [[first headquarters.]]
   )) ..
    p(_(
   [[If you finish a castle in the target area on the second island, you will get ]] ..
   [[a third headquarters on the third island. You must build a castle at ]] ..
   [[the center of the third island and hold it for 20 minutes to win the game.]]
   )) ..
    p(_(
   [[Whenever you finish an island, you will get bonus wares, depending on how many players ]] ..
   [[have finished the island before you. The earlier you finish, the fewer wares you will get. ]] ..
   [[See below for the details.]])) .. p(_(
   [[The point is that the first island only provides rocks and trees, the second only meadows and resources. The economies you leave behind will continue to work for you, but you will only reap the benefits at the moment you reach a new island.]])
   ) ..
   p(_
   [[Finally, be careful not to waste your quartz and diamonds.]]
   ) ..
   h2(_("Finish Rewards")) ..
   h3(_("First Island")) ..
   h4(_("1st to finish")) .. p(format_rewards(_finish_rewards[1][1])) ..
   h4(_("2nd to finish")) .. p(format_rewards(_finish_rewards[1][2])) ..
   h4(_("3rd to finish")) .. p(format_rewards(_finish_rewards[1][3])) ..
   h4(_("4th to finish")) .. p(format_rewards(_finish_rewards[1][4])) ..
   h3(_("Second Island")) ..
   h4(_("1st to finish")) .. p(format_rewards(_finish_rewards[2][1])) ..
   h4(_("2nd to finish")) .. p(format_rewards(_finish_rewards[2][2])) ..
   h4(_("3rd to finish")) .. p(format_rewards(_finish_rewards[2][3])) ..
   h4(_("4th to finish")) .. p(format_rewards(_finish_rewards[2][4]))
}

msgs_finished_island = {
   li(_("%1$s was the first to reach Island number %2$i.")),
   li(_("%1$s was the second to reach Island number %2$i.")),
   li(_("%1$s was the third to reach Island number %2$i.")),
   li(_("%1$s was the fourth to reach Island number %2$i."))
}
finished_island_continues = p(_("The reward for this feat amounts to:")) .. p("%s")

player_claims_hill = p(_
[[%s is now King of the Hill and will win the game in 20 minutes, if nobody takes over the hill before then.]]
)
lost_control = p(_([[%s lost control of the hill.]]))
had_control_for = p(_([[%1$s has been King of the Hill for %2$s!]]))
player_won = p(_([[%s has won the game. Congratulations!]]))
