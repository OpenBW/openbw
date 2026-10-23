# Replay metadata (`replay_info.h`)

Read the header of a StarCraft: Brood War replay (`.rep`) without loading tilesets, sprites, or running the simulator.

`load_replay` still creates a full match from the embedded map. Use this header when you only need names, slots, duration, and game settings — for example a bot that lists who played, or a tool that indexes a replay folder.

## Contents

| Field | Meaning |
| --- | --- |
| `map_name`, `game_name` | Scenario and lobby titles (Korean 949 text is converted to UTF-8 when possible) |
| `end_frame` | Last logical frame; `replay_duration_ms(end_frame)` is game time at fastest (42 ms/frame) |
| `game_type` | Numeric type; `replay_game_type_name` labels Melee, FFA, Use Map Settings, Top vs Bottom, and the other lobby types |
| `tileset`, `game_speed` | `replay_tileset_name` / `replay_game_speed_name` |
| `players[12]` | Slot, player id, controller, race, force, color, name |
| `starting_minerals`, `starting_gas` | Lobby resource settings |
| `actions_data` | Optional command stream (same bytes `load_replay` would execute) |
| `map_data` | Optional embedded `.chk` |

## Options

```cpp
bwgame::replay_info_options opt;
opt.load_actions = false; // header only; do not decompress the command stream
opt.load_map = false;     // default: skip the embedded map
auto info = bwgame::read_replay_info_file("game.rep", opt);
```

- Default: load the command stream, skip the map.
- `load_map` also decompresses the command stream so the reader can reach the map section.
- If both flags are false, `map_data_size` stays 0 (the map section is not opened).

## Example

```cpp
#include "replay_info.h"
#include <cstdio>

int main(int argc, char** argv) {
	if (argc < 2) return 1;
	auto info = bwgame::read_replay_info_file(argv[1]);
	std::printf("%s  %s  %d frames (%d ms)\n",
		info.map_name.c_str(),
		bwgame::replay_game_type_name(info.game_type),
		info.end_frame,
		bwgame::replay_duration_ms(info.end_frame));
	for (auto& p : info.players) {
		if (!bwgame::replay_player_is_human(p.controller)
			&& !bwgame::replay_player_is_computer(p.controller)) continue;
		std::printf("  %s  %s  %s\n",
			p.name.c_str(),
			bwgame::replay_race_name(p.race),
			bwgame::replay_controller_name(p.controller));
	}
}
```

`parse_replay_game_info` reads an already-decompressed 633-byte game-info block. That is the same layout `load_replay` uses after the `reRS` identifier.

## Tests

```sh
cmake -S tests -B /tmp/openbw-tests -DCMAKE_CXX_COMPILER=g++
cmake --build /tmp/openbw-tests
/tmp/openbw-tests/replay_info_test
```
