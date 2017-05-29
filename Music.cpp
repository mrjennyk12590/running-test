#include "ev3api.h"
#include "Music.h"

// コンストラクタ
Music::Music(uint8_t volume)
{
    clock_ = new ev3api::Clock();
    ev3_speaker_set_volume(volume);
}

// デストラクタ
Music::~Music()
{
    delete clock_;
}

void Music::playMusic()
{
    playTone(NOTE_C5,400);
    playTone(NOTE_D5,400);
    playTone(NOTE_E5,400);
    playTone(NOTE_F5,400);
    playTone(NOTE_E5,400);
    playTone(NOTE_D5,400);
    playTone(NOTE_C5,800);

    playTone(NOTE_E5,400);
    playTone(NOTE_F5,400);
    playTone(NOTE_G5,400);
    playTone(NOTE_A5,400);
    playTone(NOTE_G5,400);
    playTone(NOTE_F5,400);
    playTone(NOTE_E5,800);

    playTone(NOTE_C5,400);
    clock_->sleep(400);
    playTone(NOTE_C5,400);
    clock_->sleep(400);
    playTone(NOTE_C5,400);
    clock_->sleep(400);
    playTone(NOTE_C5,400);
    clock_->sleep(400);

    playTone(NOTE_C5,200);
    playTone(NOTE_C5,200);
    playTone(NOTE_D5,200);
    playTone(NOTE_D5,200);
    playTone(NOTE_E5,200);
    playTone(NOTE_E5,200);
    playTone(NOTE_F5,200);
    playTone(NOTE_F5,200);

    playTone(NOTE_E5,400);
    playTone(NOTE_D5,400);
    playTone(NOTE_C5,800);
}

void Music::playTone(uint16_t frequency, int32_t duration)
{
    ev3_speaker_play_tone(frequency,duration);
    clock_->sleep(duration);
}
