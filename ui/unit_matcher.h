#pragma once
#include <emscripten.h>
#include <emscripten/val.h>

#include "ui.h"
#include "common.h"

struct main_t;
class unit_matcher {
public:
  unit_matcher(main_t* main): m(main) {}

	struct unit_match_infos {
		int32_t cherrypi_id;
		int32_t unit_type;
		int32_t posx;
		int32_t posy;
	};

	void add_unit(
			int32_t frame,
			int32_t cherrypi_id,
			int32_t unit_type,
			int32_t posx,
			int32_t posy);

	size_t do_match_unit(
			unit_match_infos const& info,
			std::unordered_set<size_t> const& ignore_units
	);

	void do_set_matching(std::vector<unit_match_infos> const& new_units);

	emscripten::val do_matching(int max_frames_this_time);
  emscripten::val get_matching();

  void on_frame();

private:
	std::unordered_map<int32_t, std::vector<unit_match_infos> > frames_new_units;
	std::unordered_set<size_t> matched_bw_ids;
	std::unordered_set<size_t> matched_cp_ids;
  emscripten::val internal2cp = emscripten::val::object();
  emscripten::val cp2internal = emscripten::val::object();
  main_t* m;
  int highest_frame_done = -1;
  bool updated = true;
};
