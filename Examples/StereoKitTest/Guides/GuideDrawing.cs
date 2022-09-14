using StereoKit;
using System;

/// :CodeDoc: Guides 5 Drawing
/// # Drawing content with StereoKit
/// 
/// Generally, the first thing you want to do is get content on-screen! Or
/// in-visor? However it's said, in this guide we're going to explore the
/// various ways to display some holograms!
/// 
/// :End:

class GuideDrawing : ITest
{
	Material wireframeMaterial;
	Model model;

	public void Initialize()
	{
		model = Model.FromFile("DamagedHelmet.gltf");

		wireframeMaterial = Material.Default.Copy();
		wireframeMaterial.Wireframe = true;
	}

	public void Shutdown()
	{
	}

	public void Update()
	{
		Hierarchy.Push(Matrix.T(0, 2, 0));
		Mesh.Sphere.Draw(wireframeMaterial, Matrix.TS(-0.07f, 0, 0, 0.1f));
		Mesh.Sphere.Draw(Material.Default,  Matrix.TS( 0.07f, 0, 0, 0.1f));
		Tests.Screenshot("Drawing_MeshLooksLike.jpg", 600, 400, Hierarchy.ToWorld(V.XYZ(0, 0, 0.12f)), Hierarchy.ToWorld(Vec3.Zero));
		Hierarchy.Pop();

		/// :CodeDoc: Guides Drawing
		/// At its core, drawing things in 3D is done through a combination of
		/// [`Mesh`]({{site.url}}/Pages/Reference/Mesh.html)es and 
		/// [`Material`]({{site.url}}/Pages/Reference/Material.html)s. A Mesh
		/// is a collection of triangles in 3D space that describe where the
		/// surface of that 3D object is. And a Material is then a collection
		/// of parameters, [`Tex`]({{site.url}}/Pages/Reference/Tex.html)tures
		/// (2d images), and Shader code that are combined to describe the
		/// visual properties of the Mesh's surface!
		/// 
		/// ![Meshes are made from triangles!]({{site.screen_url}}/Drawing_MeshLooksLike.jpg)
		/// _Meshes are made from triangles!_
		/// 
		/// And in addition to that, you'll need to know a little bit about
		/// matrices, which are a math construct used to describe the location,
		/// orientation and scale of geometry within the 3D space! A [`Matrix`]({{site.url}}/Pages/Reference/Matrix.html)
		/// isn't difficult the way we're using it, so don't worry if math 
		/// isn't your thing.
		/// 
		/// And then StereoKit also has a [`Model`]({{site.url}}/Pages/Reference/Model.html),
		/// which is a high level combination of Meshes, Material, Matrices,
		/// and a few more things! Most of the time, you'll probably be drawing
		/// Models loaded from file, but it's important to have options.
		/// 
		/// Then lastly, StereoKit has easy systems for drawing [`Line`]({{site.url}}/Pages/Reference/Lines.html)s,
		/// [`Text`]({{site.url}}/Pages/Reference/Text.html), [`Sprite`]({{site.url}}/Pages/Reference/Sprite.html)s
		/// and various other things! These are still based on Meshes and
		/// Materials under the hood, but have some complex features that can
		/// make them difficult to build from scratch.
		/// 
		/// ## Meshes and Materials
		/// 
		/// To simplify things here, we're going to use the built-in assets,
		/// [`Mesh.Sphere`]({{site.url}}/Pages/Reference/Mesh/Sphere.html) 
		/// and [`Material.Default`]({{site.url}}/Pages/Reference/Material/Default.html).
		/// Mesh.Sphere is a built-in mesh generated using math when StereoKit
		/// starts up, and Material.Default is a high performance simple
		/// Material that serves as StereoKit's default Material. (For more
		/// built-in assets, see the [`Default`]({{site.url}}/Pages/Reference/Default.html)s)
		/// 
		Mesh.Sphere.Draw(Material.Default, Matrix.Identity);
		/// 
		/// ![Default sphere and material]({{site.screen_url}}/Drawing_Defaults.jpg)
		/// _Drawing the default sphere Mesh with the default Material._
		/// 
		/// [`Matrix.Identity`]({{site.url}}/Pages/Reference/Matrix/Identity.html)
		/// can be though of as a 'No transform' Matrix, so this is drawing the
		/// sphere at the origin of the 3D space.
		/// 
		/// That's pretty straightforward! StereoKit will get this Mesh/Material
		/// pair onto the screen this frame. Remember that StereoKit is
		/// generally an immediate mode API, so this won't show up for more
		/// than the current frame. If you want it to draw every frame, you'll
		/// have to call Draw every frame!
		/// 
		/// So how do you get a Mesh to begin with? In most cases you'll just
		/// be working with Models, but you can get a Mesh directly from a few
		/// places:
		///  - [`Mesh.Sphere`]({{site.url}}/Pages/Reference/Mesh/Sphere.html), [`Mesh.Cube`]({{site.url}}/Pages/Reference/Mesh/Cube.html), and [`Mesh.Quad`]({{site.url}}/Pages/Reference/Mesh/Quad.html) are built-in mesh assets that are handy to have around.
		///  - [`Mesh`]({{site.url}}/Pages/Reference/Mesh.html) has a number of static methods for generating procedural shapes, such as [`Mesh.GenerateRoundedCube`]({{site.url}}/Pages/Reference/Mesh/GenerateRoundedCube.html) or [`Mesh.GeneratePlane`]({{site.url}}/Pages/Reference/Mesh/GeneratePlane.html).
		///  - A Mesh can be extracted from one of a [Model's nodes]({{site.url}}/Pages/Reference/ModelNode/Mesh.html).
		///  - You can create a Mesh from a list of vertices and indices. This is more advanced, but [check the sample here]({{site.url}}/Pages/Reference/Mesh/SetVerts.html).
		///  
		/// And where do you get a Material? Well, 
		///  - See built-in Materials like [`Material.PBR`]({{site.url}}/Pages/Reference/Default/MaterialPBR.html) for high-quality surface or [`Material.Unlit`]({{site.url}}/Pages/Reference/Default/MaterialUnlit.html) for fast/stylistic surfaces.
		///  - A Material [constructor]({{site.url}}/Pages/Reference/Material/Material.html) can be called with a Shader. Check out [the Material guide]({{site.url}}/Pages/Guides/Working-with-Materials.html) for in-depth usage (Materials and Shaders are a lot of fun!).
		///  - You can call [`Material.Copy`]({{site.url}}/Pages/Reference/Material/Copy.html) to create a duplicate of an existing Material.
		/// 
		/// ## Matrix basics
		/// 
		/// If you like math, this explanation is not really for you! But if
		/// you like results, this will get you going where you need to go. The
		/// important thing to know about a [`Matrix`]({{site.url}}/Pages/Reference/Matrix.html),
		/// is that it's a good way to represent an object's transform (Translation,
		/// Rotation, and Scale).
		/// 
		/// StereoKit provides a number of Matrix creation methods that allow
		/// you to easily create Translation/Rotation/Scale matrices.

		// The identity matrix is the matrix equivalent of '1'. You can also
		// think of it as a 'no-transform' matrix.
		Matrix transform = Matrix.Identity;

		// Translates points 1 meter up the Y axis
		transform = Matrix.T(0, 1, 0);

		// Scales a point by (2,2,2), rotates it 180 on the X axis, and
		// then translates it up 1 meter up the Y axis.
		transform = Matrix.TRS(
			new Vec3(0,1,0),
			Quat.FromAngles(180, 0, 0),
			new Vec3(2,2,2));

		// To draw a cube at (0,-10,0) that's rotated 45 degrees around its Y
		// axis:
		Mesh.Cube.Draw(Material.Default, Matrix.TR(0,-10,0, Quat.FromAngles(0,45,0)));
		/// 
		/// The TRS methods have a lot of permutations that can help make your
		/// matrix creation code a bit shorter. Like, if you don't need to add
		/// scale to your TRS matrix, there's the TR variant! No rotation? Try
		/// TS! Etc. etc.
		/// 
		/// Now. Even _more_ interesting, is that many Matrices can be combined
		/// into a single Matrix representing multiple transforms! This is done
		/// via multiplication, and an important note here is that matrix
		/// multiplication is not commutative, that is: `A*B != B*A`, so the
		/// order in which you combine your matrices is important.
		/// 
		/// This can let you do things like, rotate around a pivot point, or
		/// build a hierarchy of transforms! A parent/child position hierarchy
		/// can be described pretty easily this way:

		Matrix parentTransform = Matrix.TR(10, 0, 0, Quat.FromAngles(0, 45, 0));
		Matrix childTransform  = Matrix.TS( 1, 1, 0, 0.2f);

		Mesh.Cube.Draw(Material.Default, parentTransform);
		Mesh.Cube.Draw(Material.Default, childTransform * parentTransform);
		/// 
		/// ![Combining matrices]({{site.screen_url}}/Drawing_MatrixCombine.jpg)
		/// _The smaller `childTransform` is relative to `parentTransform` via multiplication._
		/// 
		/// ## Models
		/// 
		/// The easiest way to draw complex content is through a Model! A Model
		/// is basically a small scene of Mesh/Material pairs at positions with
		/// hierarchical relationships to each other. If you're creating art in
		/// a 3D modeling tool such as Blender, then this is basically a full
		/// representation of the scene you've created there.
		/// 
		/// Since a model already has all its information within it, all you
		/// need to do is provide it with a location!
		model.Draw(Matrix.T(10, 10, 0));
		/// ![Drawing a model]({{site.screen_url}}/Drawing_Model.jpg)
		/// _StereoKit's main format is the .gltf file._
		/// 
		/// So... that was also pretty simple! The only real trick with Models
		/// is getting one in the first place, but even that's not too hard.
		/// There's a lot you can do with a Model beyond just drawing it, so
		/// for more details on that, check out [the Model guide](https://github.com/StereoKit/StereoKit/blob/master/Examples/StereoKitTest/Demos/DemoNodes.cs) (coming soon)!
		/// 
		/// But here's the quick list of where you can get a Model to begin
		/// with:
		///  - [`Model.FromFile`]({{site.url}}/Pages/Reference/Model/FromFile.html) is the easiest, most common way to get a Model!
		///  - [`Model.FromMesh`]({{site.url}}/Pages/Reference/Model/FromMesh.html) will let you create a very simple Model with a single function call.
		///  - The [Model constructor]({{site.url}}/Pages/Reference/Model/Model.html) lets you create an empty Model, which you can then fill with ModelNodes via [`Model.AddNode`]({{site.url}}/Pages/Reference/Model/AddNode.html)
		///  - You can call [`Model.Copy`]({{site.url}}/Pages/Reference/Model/Copy.html) to create a duplicate of an existing Model.
		///  
		/// ## Lines
		/// 
		/// Being able to easily draw a line is incredibly useful for
		/// debugging, and generally quite practical for many other purposes as
		/// well! StereoKit has the [`Lines`]({{site.url}}/Pages/Reference/Lines.html)
		/// class to assist with this, and is pretty straightforward to use. 
		/// There's a few variations, but at it's simplest, it's a few points,
		/// a color, and a thickness.
		Lines.Add(
			new Vec3(2, 2, 0),
			new Vec3(3, 2.5f, 0),
			Color.Black, 1*U.cm);
		/// ![Drawing a line]({{site.screen_url}}/Drawing_Lines.jpg)
		/// _You can also draw Rays, Poses, and multicolored lists of lines!_
		/// 
		/// ## Text
		/// 
		/// Text is drawn with a collection of rectangular quads, each mapped
		/// to a character glyph on a texture. StereoKit supports rendering any
		/// Unicode glyphs you throw at it, as long as the active Font has
		/// that glyph defined in it! This means you can work with all sorts of
		/// different languages right away, without any baking or preparation.
		/// 
		/// To draw text with the default Font, you can do this!
		Text.Add("こんにちは", Matrix.T(-10, 10,0));
		/// 
		/// ![Drawing text]({{site.screen_url}}/Drawing_Text.jpg)
		/// _'Hello' in Japanese, I'm pretty sure._
		/// 
		/// You can create additional font styles and fonts to use with text
		/// drawing, and there are a number of overloads for [`Text.Add`]({{site.url}}/Pages/Reference/Text/Add.html)
		/// that allow you to change the layout or constrain to a particular
		/// area. Check the docs for the method for more information about that!
		/// 
		/// ## Cool!
		/// 
		/// So that's the highlights! There's plenty more to draw and more
		/// tricks to be learned, but this is a great start! There's treasures
		/// in the documentation, so hunt around in there for more samples. You
		/// may also be interested in the [Materials guide]({{site.url}}/Pages/Guides/Working-with-Materials.html)
		/// for advanced rendering code, or the [Model guide](https://github.com/StereoKit/StereoKit/blob/master/Examples/StereoKitTest/Demos/DemoNodes.cs) 
		/// (coming soon), for managing your visible content!
		/// 
		/// If you'd like to see all the code for this document,
		/// [check it out here!](https://github.com/StereoKit/StereoKit/blob/master/Examples/StereoKitTest/Guides/GuideDrawing.cs)
		/// :End:
		Tests.Screenshot("Drawing_Defaults.jpg",      600, 600,     V.XYZ(   0,     0,  1.2f), Vec3.Zero);
		Tests.Screenshot("Drawing_MatrixCombine.jpg", 600, 600,     V.XYZ(  10,     0,  2   ), V.XYZ(10, 0,0));
		Tests.Screenshot("Drawing_Model.jpg",         600, 600, 60, V.XYZ(  10,    10, -3f  ), V.XYZ(10,10,0));
		Tests.Screenshot("Drawing_Lines.jpg",         600, 600,     V.XYZ(2.5f, 2.25f,  0.8f), V.XYZ(2.5f, 2.25f, 0));
		Tests.Screenshot("Drawing_Text.jpg",          600, 600,     V.XYZ(-10,     10, -0.1f), V.XYZ(-10, 10, 0));
	}
}
