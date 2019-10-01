using System;

namespace StereoKit
{
    public class Sprite
    {
        internal IntPtr _spriteInst;

        public float Aspect { get{ return NativeAPI.sprite_get_aspect(_spriteInst); } }
        
        public Sprite(Tex2D image, SpriteType type = SpriteType.Atlased, string atlasId = "default")
        {
            _spriteInst = NativeAPI.sprite_create(image._texInst, type, atlasId);
            if (_spriteInst == IntPtr.Zero)
                Log.Write(LogLevel.Warning, "Couldn't create sprite!");
        }
        public Sprite(string file, SpriteType type = SpriteType.Atlased, string atlasId = "default")
        {
            _spriteInst = NativeAPI.sprite_create_file(file, type, atlasId);
            if (_spriteInst == IntPtr.Zero)
                Log.Write(LogLevel.Warning, "Couldn't create sprite! {0}", file);
        }
        ~Sprite()
        {
            if (_spriteInst != IntPtr.Zero)
                NativeAPI.sprite_release(_spriteInst);
        }

        public void Draw(ref Matrix transform, Color32 color)
        {
            NativeAPI.sprite_draw(_spriteInst, ref transform, color);
        }
    }
}
