#include "unit_matcher.h"
#include "gfxtest.h"

#include <emscripten/bind.h>
#include <emscripten/val.h>
using namespace emscripten;
using namespace bwgame;

void unit_matcher::add_unit(
		int32_t frame,
		int32_t cherrypi_id,
		int32_t unit_type,
		int32_t posx,
		int32_t posy) {
	unit_match_infos i;
	i.cherrypi_id = cherrypi_id;
	i.unit_type = unit_type;
	i.posx = posx;
	i.posy = posy;
	frames_new_units[frame].push_back(i);
}

size_t unit_matcher::do_match_unit(
		unit_match_infos const& info,
		std::unordered_set<size_t> const& ignore_units
) {
	state_functions f(m->ui.st);
	int32_t best_dist = 0;
	unit_t* best_unit = nullptr;
	int32_t howmany_within_20 = 0;
	int32_t howmany_within_30 = 0;
	for (unit_t* u : ptr(m->ui.st.visible_units)) {
		if ((int)u->unit_type->id != info.unit_type) {
			continue;
		}
		if (ignore_units.find(f.get_unit_id(u).raw_value) != ignore_units.end()){
			continue;
		}
		int32_t d = (u->position.x - info.posx) * (u->position.x - info.posx);
		d += (u->position.y - info.posy) * (u->position.y - info.posy);
		if (d < 20*20) {
			++howmany_within_20;
		}
		if (d < 30*30) {
			++howmany_within_30;
		}
		if (best_unit == nullptr || d < best_dist) {
			best_unit = u;
			best_dist = d;
		}
	}
	// HACK: This accounts for some of the non-reproducibility of OpenBW
	// This is too far
	if (best_dist > 25*25) {
		return 0;
	}
	else if (best_dist > 18*18) {
		if (howmany_within_30 > 1)
			return 0;
	}
	else if (best_dist > 10*10) {
		if (howmany_within_20 > 1)
			return 0;
	}
	return best_unit ? f.get_unit_id(best_unit).raw_value : 0;
}

void unit_matcher::do_set_matching(std::vector<unit_match_infos> const& new_units) {
	for (auto& unit: new_units) {
		if (matched_cp_ids.find(unit.cherrypi_id) != matched_cp_ids.end()) {
			continue;
		}
		size_t internal_index = do_match_unit(unit, matched_bw_ids);
		if (internal_index > 0) {
      updated = true;
			cp2internal.set(
				std::to_string(unit.cherrypi_id),
				internal_index
			);
			internal2cp.set(
				std::to_string(internal_index),
				unit.cherrypi_id
			);
			matched_bw_ids.insert(internal_index);
			matched_cp_ids.insert(unit.cherrypi_id);
		}
	}
}

val unit_matcher::do_matching(int max_frames_this_time) {
	auto& st = m->ui.st;
  int max_frame = std::min(
    st.current_frame + max_frames_this_time,
    m->ui.replay_st.end_frame
  );
	do {
		m->next_frame(); // Will call 'on_frame'
	} while (st.current_frame < max_frame);

	val progress = val::object();
	progress.set("current_frame", st.current_frame);
	progress.set("end_frame", m->ui.replay_st.end_frame);
  progress.set("done", st.current_frame >= m->ui.replay_st.end_frame);
	return progress;
}

val unit_matcher::get_matching() {
  val matching = val::object();
  matching.set("this", int(this));
  matching.set("updated", updated);
  matching.set("highest_frame_done", highest_frame_done);
  if (updated) {
  	matching.set("cp2internal", cp2internal);
  	matching.set("internal2cp", internal2cp);
    matching.set("matched_cp_count", matched_cp_ids.size());
    updated = false;
  }
	return matching;
}

void unit_matcher::on_frame() {
  auto& st = m->ui.st;
  if (st.current_frame <= highest_frame_done) {
    return;
  }
  highest_frame_done = st.current_frame;

  auto it = frames_new_units.find(m->ui.st.current_frame);
  if (it != frames_new_units.end()) {
    do_set_matching(it->second);
  }
}
