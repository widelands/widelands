#!/usr/bin/env python -tt
# encoding: utf-8
#

"""
Use ++/-- operators instead of += 1 or -= 1;
"""

error_msg="Use ++var / --var instead of var += 1 / var -= 1."

regexp = r"""(\+|\-)= 1(((?=\D)(?=\S))|$)"""

forbidden = [
    "inside_ifdefs += 1",
    "inside_ifdefs -= 1",
    "i += 1;",
    "i -= 1;",
    "enemy_sites[best_target].attack_counter += 1;",
    "msites_per_size[bo.desc->get_size()].finished -= 1;",
    "for (uint8_t i = 0; i < jobs_to_run_count; i += 1) {",
    "for (uint8_t i = foo.size() -1; i >=0; i -= 1) {",
]

allowed = [
    "prio -= 10;",
    "prio += 10;",
    "year -= 1980;",
    "percent -= 100 * animations[i].second;",
    "persistent_data->least_military_score -= 10 / divider;",
    "for (uint8_t i = 0; i < jobs_to_run_count; i += 11) {",
    "for (uint8_t i = foo.size() -1; i >=0; i -= 11) {",
    "prio += 1 - bf->trees_nearby / 3;",
    "prio += 1 - bf->rangers_nearby *",
    "current_song_ += 1 + random % (songs_.size() - 1);"
]
