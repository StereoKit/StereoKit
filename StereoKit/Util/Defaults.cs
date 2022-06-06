namespace StereoKit
{
	/// <summary>This is a collection of StereoKit default assets that are
	/// created or loaded by StereoKit during its initialization phase! Feel
	/// free to use them or Copy them, but be wary about modifying them, 
	/// since it could affect many things throughout the system.</summary>
	public static class Default
	{
		/// <summary>The default material! This is used by many models and
		/// meshes rendered from within StereoKit. Its shader is tuned for 
		/// high performance, and may change based on system performance
		/// characteristics, so it can be great to copy this one when
		/// creating your own materials! Or if you want to override
		/// StereoKit's default material, here's where you do it!</summary>
		public static Material Material         { get; private set; }
		/// <summary>The default Physically Based Rendering material! This is
		/// used by StereoKit anytime a mesh or model has metallic or 
		/// roughness properties, or needs to look more realistic. Its shader
		/// may change based on system performance characteristics, so it can
		/// be great to copy this one when creating your own materials! Or if
		/// you want to override StereoKit's default PBR behavior, here's
		/// where you do it! Note that the shader used by default here is
		/// much more costly than Default.Material.</summary>
		public static Material MaterialPBR      { get; private set; }
		/// <summary>Same as MaterialPBR, but it uses a discard clip for
		/// transparency.</summary>
		public static Material MaterialPBRClip  { get; private set; }
		/// <summary>The default unlit material! This is used by StereoKit 
		/// any time a mesh or model needs to be rendered with an unlit 
		/// surface. Its shader may change based on system performance 
		/// characteristics, so it can be great to copy this one when 
		/// creating your own materials! Or if you want to override 
		/// StereoKit's default unlit behavior, here's where you do it!
		/// </summary>
		public static Material MaterialUnlit    { get; private set; }
		/// <summary>The default unlit material with alpha clipping! This is
		/// used by StereoKit for unlit content with transparency, where
		/// completely transparent pixels are discarded. This means less
		/// alpha blending, and fewer visible alpha blending issues! In
		/// particular, this is how Sprites are drawn. Its shader may change
		/// based on system performance characteristics, so it can be great
		/// to copy this one when creating your own materials! Or if you want
		/// to override StereoKit's default unlit clipped behavior, here's
		/// where you do it! </summary>
		public static Material MaterialUnlitClip{ get; private set; }
		/// <summary>This material is used for projecting equirectangular 
		/// textures into cubemap faces. It's probably not a great idea to 
		/// change this one much!</summary>
		public static Material MaterialEquirect { get; private set; }
		/// <summary>This material is used as the default for rendering text!
		/// By default, it uses the 'default/shader_font' shader, which is a 
		/// two-sided alpha-clip shader. This also turns off backface culling.
		/// </summary>
		public static Material MaterialFont     { get; private set; }
		/// <summary>This is the default material for rendering the hand! 
		/// It's a copy of the default material, but set to transparent, and 
		/// using a generated texture.</summary>
		public static Material MaterialHand     { get; private set; }
		/// <summary>The material used by the UI! By default, it uses a shader 
		/// that creates a 'finger shadow' that shows how close the finger is 
		/// to the UI.</summary>
		public static Material MaterialUI       { get; private set; }
		/// <summary> A material for indicating interaction volumes! It
		/// renders a border around the edges of the UV coordinates that will
		/// 'grow' on proximity to the user's finger. It will discard pixels
		/// outside of that border, but will also show the finger shadow.
		/// This is meant to be an opaque material, so it works well for
		/// depth LSR.
		/// 
		/// This material works best on cube-like meshes where each face has
		/// UV coordinates from 0-1.
		/// 
		/// Shader Parameters:
		/// ```color                - color
		/// border_size          - meters
		/// border_size_grow     - meters
		/// border_affect_radius - meters```
		/// </summary>
		public static Material MaterialUIBox { get; private set; }
		/// <summary>The material used by the UI for Quadrant Sized UI
		/// elements. See UI.QuadrantSizeMesh for additional details. By
		/// default, it uses a shader that creates a 'finger shadow' that shows
		/// how close the finger is to the UI.</summary>
		public static Material MaterialUIQuadrant { get; private set; }

		/// <summary>Default 2x2 white opaque texture, this is the texture
		/// referred to as 'white' in the shader texture defaults.</summary>
		public static Tex Tex      { get; private set; }
		/// <summary>Default 2x2 black opaque texture, this is the texture
		/// referred to as 'black' in the shader texture defaults.</summary>
		public static Tex TexBlack { get; private set; }
		/// <summary>Default 2x2 middle gray (0.5,0.5,0.5) opaque texture,
		/// this is the texture referred to as 'gray' in the shader texture
		/// defaults.</summary>
		public static Tex TexGray  { get; private set; }
		/// <summary>Default 2x2 flat normal texture, this is a normal that
		/// faces out from the, face, and has a color value of (0.5,0.5,1).
		/// This is the texture referred to as 'flat' in the shader texture
		/// defaults.</summary>
		public static Tex TexFlat  { get; private set; }
		/// <summary>Default 2x2 roughness color (1,1,0,1) texture, this is the
		/// texture referred to as 'rough' in the shader texture defaults.
		/// </summary>
		public static Tex TexRough { get; private set; }
		/// <summary>This is a white checkered grid texture used to easily add
		/// visual features to materials. By default, this is used for the
		/// loading fallback texture for all Tex objects.</summary>
		public static Tex TexDevTex { get; private set; }
		/// <summary>This is a red checkered grid texture used to indicate some
		/// sort of error has occurred. By default, this is used for the error
		/// fallback texture for all Tex objects.</summary>
		public static Tex TexError { get; private set; }
		/// <summary>The default cubemap that StereoKit generates, this is
		/// the cubemap that's visible as the background and initial scene
		/// lighting.</summary>
		public static Tex Cubemap  { get; private set; }

		/// <summary>A default quad mesh, 2 triangles, 4 verts, from
		/// (-0.5,-0.5,0) to (0.5,0.5,0) and facing forward on the Z axis
		/// (0,0,-1). White vertex colors, and UVs from (1,1) at vertex
		/// (-0.5,-0.5,0) to (0,0) at vertex (0.5,0.5,0).</summary>
		public static Mesh MeshQuad   { get; private set; }
		/// <summary>A default quad mesh designed for full-screen rendering.
		/// 2 triangles, 4 verts, from (-1,-1,0) to (1,1,0) and facing
		/// backwards on the Z axis (0,0,1). White vertex colors, and UVs
		/// from (0,0) at vertex (-1,-1,0) to (1,1) at vertex (1,1,0).
		/// </summary>
		public static Mesh MeshScreenQuad { get; private set; }
		/// <summary>A cube with dimensions of (1,1,1), this is equivalent to
		/// Mesh.GenerateCube(Vec3.One).</summary>
		public static Mesh MeshCube   { get; private set; }
		/// <summary>A sphere mesh with a diameter of 1. This is equivalent
		/// to Mesh.GenerateSphere(1,4).</summary>
		public static Mesh MeshSphere { get; private set; }

		/// <summary>This is a fast, general purpose shader. It uses a
		/// texture for 'diffuse', a 'color' property for tinting the
		/// material, and a 'tex_scale' for scaling the UV coordinates. For
		/// lighting, it just uses a lookup from the current cubemap.
		/// </summary>
		public static Shader Shader         { get; private set; }
		/// <summary>A physically based shader.</summary>
		public static Shader ShaderPbr      { get; private set; }
		/// <summary>Same as ShaderPBR, but with a discard clip for
		/// transparency.</summary>
		public static Shader ShaderPbrClip  { get; private set; }
		/// <summary>Sometimes lighting just gets in the way! This is an
		/// extremely simple and fast shader that uses a 'diffuse' texture
		/// and a 'color' tint property to draw a model without any lighting
		/// at all!</summary>
		public static Shader ShaderUnlit    { get; private set; }
		/// <summary>Sometimes lighting just gets in the way! This is an
		/// extremely simple and fast shader that uses a 'diffuse' texture
		/// and a 'color' tint property to draw a model without any lighting
		/// at all! This shader will also discard pixels with an alpha of
		/// zero.</summary>
		public static Shader ShaderUnlitClip{ get; private set; }
		/// <summary>A shader for text! Right now, this will render a font
		/// atlas texture, and perform alpha-testing for transparency, and
		/// super-sampling for better readability. It also flips normals of
		/// the back-face of the surface, so  backfaces get lit properly when
		/// backface culling is turned off, as it is by default for text.
		/// </summary>
		public static Shader ShaderFont     { get; private set; }
		/// <summary>A shader for projecting equirectangular textures onto
		/// cube faces! This is for equirectangular texture loading.</summary>
		public static Shader ShaderEquirect { get; private set; }
		/// <summary>A shader for UI or interactable elements, this'll be the
		/// same as the Shader, but with an additional finger 'shadow' and
		/// distance circle effect that helps indicate finger distance from
		/// the surface of the object.</summary>
		public static Shader ShaderUI       { get; private set; }
		/// <summary>A shader for indicating interaction volumes! It renders
		/// a border around the edges of the UV coordinates that will 'grow'
		/// on proximity to the user's finger. It will discard pixels outside
		/// of that border, but will also show the finger shadow. This is
		/// meant to be an opaque shader, so it works well for depth LSR.
		/// 
		/// This shader works best on cube-like meshes where each face has
		/// UV coordinates from 0-1.
		/// 
		/// Shader Parameters:
		/// ```color                - color
		/// border_size          - meters
		/// border_size_grow     - meters
		/// border_affect_radius - meters```</summary>
		public static Shader ShaderUIBox { get; private set; }

		/// <summary>A default click sound that lasts for 300ms. It's a
		/// procedurally generated sound based on a mouse press, with extra
		/// low frequencies in it.</summary>
		public static Sound SoundClick   { get; private set; }
		/// <summary>A default click sound that lasts for 300ms. It's a
		/// procedurally generated sound based on a mouse release, with extra
		/// low frequencies in it.</summary>
		public static Sound SoundUnclick { get; private set; }

		/// <summary>The default font used by StereoKit's text. This varies
		/// from platform to platform, but is typically a sans-serif general
		/// purpose font, such as Segoe UI.</summary>
		public static Font Font { get; private set; }

		internal static void Initialize()
		{
			Material           = Material.Find(DefaultIds.material);
			MaterialPBR        = Material.Find(DefaultIds.materialPBR);
			MaterialPBRClip    = Material.Find(DefaultIds.materialPBRClip);
			MaterialUnlit      = Material.Find(DefaultIds.materialUnlit);
			MaterialUnlitClip  = Material.Find(DefaultIds.materialUnlitClip);
			MaterialEquirect   = Material.Find(DefaultIds.materialEquirect);
			MaterialFont       = Material.Find(DefaultIds.materialFont);
			MaterialHand       = Material.Find(DefaultIds.materialHand);
			MaterialUI         = Material.Find(DefaultIds.materialUI);
			MaterialUIBox      = Material.Find(DefaultIds.materialUIBox);
			MaterialUIQuadrant = Material.Find(DefaultIds.materialUIQuadrant);

			Tex      = Tex.Find(DefaultIds.tex);
			TexBlack = Tex.Find(DefaultIds.texBlack);
			TexGray  = Tex.Find(DefaultIds.texGray);
			TexFlat  = Tex.Find(DefaultIds.texFlat);
			TexRough = Tex.Find(DefaultIds.texRough);
			TexDevTex= Tex.Find(DefaultIds.texDevTex);
			TexError = Tex.Find(DefaultIds.texError);
			Cubemap  = Tex.Find(DefaultIds.cubemap);

			Font = Font.Find(DefaultIds.font);

			MeshQuad       = Mesh.Find(DefaultIds.meshQuad);
			MeshScreenQuad = Mesh.Find(DefaultIds.meshScreenQuad);
			MeshCube       = Mesh.Find(DefaultIds.meshCube);
			MeshSphere     = Mesh.Find(DefaultIds.meshSphere);

			Shader         = Shader.Find(DefaultIds.shader);
			ShaderPbr      = Shader.Find(DefaultIds.shaderPbr);
			ShaderPbrClip  = Shader.Find(DefaultIds.shaderPbrClip);
			ShaderUnlit    = Shader.Find(DefaultIds.shaderUnlit);
			ShaderUnlitClip= Shader.Find(DefaultIds.shaderUnlitClip);
			ShaderFont     = Shader.Find(DefaultIds.shaderFont);
			ShaderEquirect = Shader.Find(DefaultIds.shaderEquirect);
			ShaderUI       = Shader.Find(DefaultIds.shaderUI);
			ShaderUIBox    = Shader.Find(DefaultIds.shaderUIBox);

			SoundClick   = Sound.Find(DefaultIds.soundClick);
			SoundUnclick = Sound.Find(DefaultIds.soundUnclick);
		}

		internal static void Shutdown()
		{
			Material           = null;
			MaterialPBR        = null;
			MaterialPBRClip    = null;
			MaterialUnlit      = null;
			MaterialUnlitClip  = null;
			MaterialEquirect   = null;
			MaterialFont       = null;
			MaterialHand       = null;
			MaterialUI         = null;
			MaterialUIBox      = null;
			MaterialUIQuadrant = null;

			Tex      = null;
			TexBlack = null;
			TexGray  = null;
			TexFlat  = null;
			TexRough = null;
			Cubemap  = null;

			Font = null;

			MeshQuad   = null;
			MeshCube   = null;
			MeshSphere = null;

			Shader         = null;
			ShaderPbr      = null;
			ShaderPbrClip  = null;
			ShaderUnlit    = null;
			ShaderUnlitClip= null;
			ShaderFont     = null;
			ShaderEquirect = null;
			ShaderUI       = null;
			ShaderUIBox    = null;

			SoundClick   = null;
			SoundUnclick = null;
		}
	}
}
