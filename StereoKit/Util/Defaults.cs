namespace StereoKit
{
    public static class Default
    {
        public static Material Material         { get; private set; }
        public static Material MaterialEquirect { get; private set; }
        public static Material MaterialFont     { get; private set; }
        public static Material MaterialHand     { get; private set; }
        public static Material MaterialUI       { get; private set; }

        public static Tex Tex      { get; private set; }
        public static Tex TexBlack { get; private set; }
        public static Tex TexGray  { get; private set; }
        public static Tex TexFlat  { get; private set; }
        public static Tex TexRough { get; private set; }
        public static Tex Cubemap  { get; private set; }

        public static Mesh Quad { get; private set; }

        public static Shader Shader         { get; private set; }
        public static Shader ShaderPbr      { get; private set; }
        public static Shader ShaderUnlit    { get; private set; }
        public static Shader ShaderFont     { get; private set; }
        public static Shader ShaderEquirect { get; private set; }
        public static Shader ShaderUI       { get; private set; }

        internal static void Initialize()
        {
            Material         = Material.Find(DefaultIds.material);
            MaterialEquirect = Material.Find(DefaultIds.materialEquirect);
            MaterialFont     = Material.Find(DefaultIds.materialFont);
            MaterialHand     = Material.Find(DefaultIds.materialHand);
            MaterialUI       = Material.Find(DefaultIds.materialUI);

            Tex      = Tex.Find(DefaultIds.tex);
            TexBlack = Tex.Find(DefaultIds.texBlack);
            TexGray  = Tex.Find(DefaultIds.texGray);
            TexFlat  = Tex.Find(DefaultIds.texFlat);
            TexRough = Tex.Find(DefaultIds.texRough);
            Cubemap  = Tex.Find(DefaultIds.cubemap);

            Quad = Mesh.Find(DefaultIds.quad);

            Shader         = Shader.Find(DefaultIds.shader);
            ShaderPbr      = Shader.Find(DefaultIds.shaderPbr);
            ShaderUnlit    = Shader.Find(DefaultIds.shaderUnlit);
            ShaderFont     = Shader.Find(DefaultIds.shaderFont);
            ShaderEquirect = Shader.Find(DefaultIds.shaderEquirect);
            ShaderUI       = Shader.Find(DefaultIds.shaderUI);
        }

        internal static void Shutdown()
        {
            Material         = null;
            MaterialEquirect = null;
            MaterialFont     = null;
            MaterialHand     = null;
            MaterialUI       = null;

            Tex      = null;
            TexBlack = null;
            TexGray  = null;
            TexFlat  = null;
            TexRough = null;
            Cubemap  = null;

            Quad = null;

            Shader         = null;
            ShaderPbr      = null;
            ShaderUnlit    = null;
            ShaderFont     = null;
            ShaderEquirect = null;
            ShaderUI       = null;
        }
    }
}
