-- =======================================================================
--                          Texts for this scenario
-- =======================================================================
welcome_msg = rt(
   h1(_"Welcome to Island Hopping") ..
   h2(_"Rules") ..
   p(_
[[Island Hopping is a traditional tournament in the Atlantean culture. The
rules of the game are simple: You start with a headquarters one one island.
When you finish a castle at the end of the first island, you are granted a
second headquarters on a second island which will contain all wares from your
first headquarters.]]) .. p(_
[[If you finish a castle at the target area on the second island, you will get
a third headquarters on the third island. You must build and hold a castle at
the center of the third island for 20 minutes to win the game]]) .. p(_
[[Whenever you finish an island, you get bonus wares depending how many players
have finished the island before you. The earlier you finish, the less wares you
get. See below for the details.]]) .. p(_
[[The clue is that the first island only offers stones and wood, the second
only meadows and resources. The economies you leave behind will continue to
work for you, but you only reap the benefits when you reach a new island.]]
) ..
h2(_"Finish rewards") ..
h3(_"First Island") ..
h4(_"1st to finish") .. p(format_rewards(_finish_rewards[1][1])) ..
h4(_"2nd to finish") .. p(format_rewards(_finish_rewards[1][2])) ..
h4(_"3rd to finish") .. p(format_rewards(_finish_rewards[1][3])) ..
h4(_"4th to finish") .. p(format_rewards(_finish_rewards[1][4])) ..
h3(_"Second Island") ..
h4(_"1st to finish") .. p(format_rewards(_finish_rewards[2][1])) ..
h4(_"2nd to finish") .. p(format_rewards(_finish_rewards[2][2])) ..
h4(_"3rd to finish") .. p(format_rewards(_finish_rewards[2][3])) ..
h4(_"4th to finish") .. p(format_rewards(_finish_rewards[2][4]))
)


msgs_finished_island = {
   _"%s was the first to reach the Island number %i.",
   _"%s was the second to reach the Island number %i.",
   _"%s was the third to reach the Island number %i.",
   _"%s was the fourth to reach the Island number %i."
}
finished_island_continues = _ "The reward for this feat amounts to<br><br>%s"

player_claims_hill = rt(p(_
[[%s is now King of the Hill and will win the game in 20 minutes, if nobody takes over the hill until than.]]
))
lost_control = rt(p(_
[[%s lost control of the hill.]]
))
had_control_for = rt(p(_[[%s has been King of the Hill since %s!]]))
player_won = rt(p(_[[%s has won the game. Congratulations!]]))




