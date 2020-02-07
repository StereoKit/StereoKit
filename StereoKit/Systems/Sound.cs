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

        public static Sound FromFile(string filename)
        {
            IntPtr inst = NativeAPI.sound_create(filename);
            return inst == IntPtr.Zero ? null : new Sound(inst);
        }

        public static Sound Generate(AudioGenerator generator, float duration)
        {
            AudioGenerator tmpGen = generator;
            IntPtr inst = NativeAPI.sound_generate(tmpGen, duration);
            return inst == IntPtr.Zero ? null : new Sound(inst);
        }
    }
}
