#ifndef NATIVE_SOUND_H
#define NATIVE_SOUND_H

namespace native_sound {
	struct sound {
		virtual ~sound() {}
	};
	
	extern int frequency;
	extern int channels;

	void init();
	void play(int channel, sound* s, int volume, int pan);
	bool is_playing(int channel);
	void stop(int channel);
	void set_volume(int channel, int volume);
	std::unique_ptr<sound> load_wav(const void* data, size_t size);

}

#endif
