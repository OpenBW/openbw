# Replay command stream (`replay_actions.h`)

Walk the command section of a StarCraft: Brood War replay without creating units or loading tilesets.

The simulator (`load_replay` + `next_frame`) applies each command to the match. This header only reads the stream: per-player counts, APM, chat, and leave events. That is enough for a listing bot, a replay index, or a quick APM table.

## Stream layout

Commands are stored as frame bundles:

```
int32 frame
uint8 size
size bytes: repeating { uint8 player_id, uint8 action_id, payload }
```

Payload sizes match the engine's `read_action_*` handlers in `actions.h` (select lists, build tiles, 81-byte chat, and so on). Unknown action ids throw `bwgame::exception`, same as playback.

## What you get

| Result | Meaning |
| --- | --- |
| `players[i].action_count` | Every recorded command, including selection |
| `players[i].command_count` | Same, excluding Select / Shift Select / Deselect |
| `average_apm` | `action_count * (60000 / 42) / duration_frames` |
| `peak_apm` | Highest rate in any 238-frame window (~10 s at fastest) |
| `chat` | Public chat strings (bytes below 32 stripped) |
| `leaves` | Leave-game commands and their reason byte |
| `events` | Optional full log (`collect_events`) |

Duration defaults to the header `end_frame` when you scan a `.rep` file, or to the last bundle frame when you scan a raw buffer. Pass `duration_frames` to override.

Player ids in the stream are mapped through the twelve slot player-ids from the replay header. `visit_replay_actions` reports the raw player id; `owner` is the slot index (0–11) or -1 if unmapped.

## Example

```cpp
#include "replay_actions.h"
#include <cstdio>

int main(int argc, char** argv) {
	if (argc < 2) return 1;
	auto stats = bwgame::scan_replay_file(argv[1]);
	for (int i = 0; i != 12; ++i) {
		auto& p = stats.players[i];
		if (p.action_count == 0) continue;
		std::printf("slot %d  actions %d  APM %.0f  peak %d\n",
			i, p.action_count, p.average_apm, p.peak_apm);
	}
	for (auto& chat : stats.chat) {
		std::printf("f%d p%d: %s\n", chat.frame, chat.owner, chat.text.c_str());
	}
}
```

To walk commands yourself:

```cpp
bwgame::visit_replay_actions(data, data + size,
	[](int frame, int player_id, int action_id, const uint8_t* payload, size_t n) {
		(void)payload; (void)n;
		std::printf("%d %d %s\n", frame, player_id, bwgame::replay_action_name(action_id));
	});
```

`read_replay_actions_file` returns the decompressed command section only (the map chunk is not opened).

## Tests

```sh
cmake -S tests -B /tmp/openbw-tests -DCMAKE_CXX_COMPILER=g++
cmake --build /tmp/openbw-tests
/tmp/openbw-tests/replay_actions_test
```
