using System;

namespace StereoKit
{
    /// <summary>The class represents a sound effect! Excellent for blips and bloops
    /// and little clips that you might play around your scene. Not great for long
    /// streams of audio like you might see in a podcast. Right now, this only supports
    /// .wav files, and procedurally generated noises!</summary>
    public class Sound
    {
        internal IntPtr _inst;

        public string Id { set { NativeAPI.sound_set_id(_inst, value); } }

        private Sound(IntPtr sound)
        {
            _inst = sound;
            if (_inst == IntPtr.Zero)
                Log.Err("Received an empty sound!");
        }
        ~Sound()
        {
            if (_inst != IntPtr.Zero)
                NativeAPI.sound_release(_inst);
        }

        /// <summary>Plays the sound at the 3D location specified, using the volume
        /// parameter as an additional volume control option! Sound volume falls off
        /// from 3D location, and can also indicate direction and location through
        /// spatial audio cues. So make sure the position is where you want people to
        /// think it's from! Currently, if this sound is playing somewhere else, it'll
        /// be cancelled, and moved to this location.</summary>
        /// <param name="at">World space location for the audio to play at.</param>
        /// <param name="volume">Volume modifier for the effect! 1 means full volume,
        /// and 0 means completely silent.</param>
        public void Play(Vec3 at, float volume = 1)
            => NativeAPI.sound_play(_inst, at, volume);

        /// <summary>Looks for a Sound asset that's already loaded, matching the given id!</summary>
        /// <param name="modelId">Which Sound are you looking for?</param>
        /// <returns>A link to the sound matching 'soundId', null if none is found.</returns>
        public static Sound Find(string modelId)
        {
            IntPtr sound = NativeAPI.sound_find(modelId);
            return sound == IntPtr.Zero ? null : new Sound(sound);
        }

        /// <summary>Loads a sound effect from file! Currently, StereoKit only supports
        /// .wav files. Audio is converted to mono.</summary>
        /// <param name="filename">Name of the audio file! Supports .wav files.</param>
        /// <returns>A sound object, or null if something went wrong.</returns>
        public static Sound FromFile(string filename)
        {
            IntPtr inst = NativeAPI.sound_create(filename);
            return inst == IntPtr.Zero ? null : new Sound(inst);
        }

        /// <summary>This function will generate a sound from a function you provide! The
        /// function is called once for each sample in the duration. As an example, it 
        /// may be called 48,000 times for each second of duration.</summary>
        /// <param name="generator">This function takes a time value as an argument, which
        /// will range from 0-duration, and should return a value from -1 - +1 representing
        /// the audio wave at that point in time.</param>
        /// <param name="duration">In seconds, how long should the sound be?</param>
        /// <returns>Returns a generated sound effect! Or null if something went wrong.</returns>
        public static Sound Generate(AudioGenerator generator, float duration)
        {
            AudioGenerator tmpGen = generator;
            IntPtr inst = NativeAPI.sound_generate(tmpGen, duration);
            return inst == IntPtr.Zero ? null : new Sound(inst);
        }
    }
}
