using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit {

    /// <summary>The default StereoKit assets all have ids, and this class is a collection
    /// of all of them in case you want to copy or use them!</summary>
    public static class DefaultIds
    {
        /// <summary>The default material! This is used by any model or mesh rendered from
        /// within StereoKit. Its shader may change based on system performance characteristics,
        /// so it can be great to copy this one when creating your own materials!</summary>
        public const string material         = "default/material";
        /// <summary>This material is used for projecting equirectangular textures into cubemap
        /// faces. It's probably not a great idea to change this one much!</summary>
        public const string materialEquirect = "default/equirect_convert";
        /// <summary>This material is used as the default for rendering text! By default, it uses
        /// the 'default/shader_font' shader, which is a two-sided alpha-clip shader. This also 
        /// turns off backface culling.</summary>
        public const string materialFont     = "default/material_font";
        /// <summary>This is the default material for rendering the hand! It's a copy of the
        /// default material, but set to transparent, and using a generated texture.</summary>
        public const string materialHand     = "default/material_hand";
        /// <summary>The material used by the UI! By default, it uses a shader that creates
        /// a 'finger shadow' that shows how close the finger is to the UI.</summary>
        public const string materialUI       = "default/material_ui";

        /// <summary>Default 2x2 white opaque texture, this is the texture referred to as 'white'
        /// in the shader texture defaults.</summary>
        public const string tex      = "default/tex";
        /// <summary>Default 2x2 black opaque texture, this is the texture referred to as 'black'
        /// in the shader texture defaults.</summary>
        public const string texBlack = "default/tex_black";
        /// <summary>Default 2x2 middle gray (0.5,0.5,0.5) opaque texture, this is the texture 
        /// referred to as 'gray' in the shader texture defaults.</summary>
	    public const string texGray  = "default/tex_gray";
        /// <summary>Default 2x2 flat normal texture, this is a normal that faces out from the,
        /// face, and has a color value of (0.5,0.5,1). this is the texture referred to as 'flat'
        /// in the shader texture defaults.</summary>
	    public const string texFlat  = "default/tex_flat";
        /// <summary>Default 2x2 roughness color (0,0,1) texture, this is the texture referred to as 'rough'
        /// in the shader texture defaults.</summary>
	    public const string texRough = "default/tex_rough";

        /// <summary>The default font asset used by the UI and text. This is currently Segoe!</summary>
        public const string font = "default/font";

        /// <summary>The default cubemap that StereoKit generates, this is the cubemap that's visible
        /// as the background and initial scene lighting.</summary>
        public const string cubemap = "default/cubemap";

        /// <summary>A default quad mesh, 2 triangles, 4 verts, from (-1,-1,0) to (1,1,0) and facing 
        /// forward on the Z axis (0,0,-1). White vertex colors, and UVs from (0,0) at 
        /// vertex (-1,-1,0) to (1,1) at vertex (1,1,0).</summary>
        public const string quad = "default/quad";

        /// <summary>This is a fast, general purpose shader. It uses a texture for 'diffuse', a 'color'
        /// property for tinting the material, and a 'tex_scale' for scaling the UV coordinates. For 
        /// lighting, it just uses a lookup from the current cubemap.</summary>
        public const string shader         = "default/shader";
        /// <summary>A physically based shader.</summary>
        public const string shaderPbr      = "default/shader_pbr";
        /// <summary>Sometimes lighting just gets in the way! This is an extremely simple and fast shader
        /// that uses a 'diffuse' texture and a 'color' tint property to draw a model without any lighting
        /// at all!</summary>
        public const string shaderUnlit    = "default/shader_unlit";
        /// <summary>A shader for text! Right now, this will render a font atlas texture, and perform
        /// alpha-testing for transparency. It also flips normals of the back-face of the surface, so 
        /// backfaces get lit properly when backface culling is turned off, as it is by default for text.</summary>
        public const string shaderFont     = "default/shader_font";
        /// <summary>A shader for projecting equirectangular textures onto cube faces! This is for 
        /// equirectangular texture loading.</summary>
        public const string shaderEquirect = "default/shader_equirect";
        public const string shaderUI       = "default/shader_ui";
        public const string soundClick     = "default/sound_click";
        public const string soundUnclick   = "default/sound_unclick";
    }
}