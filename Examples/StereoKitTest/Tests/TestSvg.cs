using StereoKit;
using System;

// Checks the SVG loader's geometry numerically. Every image here is 100
// units tall, so it normalizes to one meter and each unit is a centimeter.
class TestSvg : ITest
{
	const string head = "<svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' width='100' height='100' viewBox='0 0 100 100'>";

	static Mesh Load(string body, string root = head)
	{
		Mesh mesh = Mesh.FromMemory("test.svg", System.Text.Encoding.UTF8.GetBytes(root + body + "</svg>"));
		Assets.BlockUntil(mesh, AssetState.Loaded);
		return mesh;
	}
	static bool Near(float a, float b, float tolerance = 0.002f) => Math.Abs(a - b) <= tolerance;
	static bool IsColor(Color32 c, byte r, byte g, byte b) => c.r == r && c.g == g && c.b == b;

	// Summed triangle area in meters squared. Triangles are expected to wind
	// clockwise seen from +Z, so a positive cross product is a failure.
	static float Area(Mesh mesh, out bool allClockwise)
	{
		Vertex[] v    = mesh.GetVerts();
		uint[]   i    = mesh.GetInds();
		float    area = 0;
		allClockwise  = true;
		for (int t = 0; t + 2 < i.Length; t += 3)
		{
			Vec3  a = v[i[t]].pos, b = v[i[t+1]].pos, c = v[i[t+2]].pos;
			float cross = (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
			if (cross > 0) allClockwise = false;
			area += Math.Abs(cross) * 0.5f;
		}
		return area;
	}

	bool TestRect()
	{
		Mesh m = Load("<rect x='10' y='10' width='50' height='30' fill='#ff0000'/>");
		Vertex[] v = m.GetVerts();
		bool ok = m.VertCount == 4 && m.IndCount == 6;
		ok = ok && Near(Area(m, out bool cw), 0.5f*0.3f) && cw;
		// Centered on the image, SVG +x runs along -X, +y along +Y, faces -Z.
		ok = ok && Near(m.Bounds.center.x, 0.15f) && Near(m.Bounds.center.y, 0.25f);
		ok = ok && Near(m.Bounds.dimensions.x, 0.5f) && Near(m.Bounds.dimensions.y, 0.3f);
		foreach (Vertex vert in v)
			ok = ok && Near(vert.norm.z, -1) && IsColor(vert.col, 255, 0, 0);
		return ok;
	}

	bool TestPhysicalUnits()
	{
		string mm = "<svg xmlns='http://www.w3.org/2000/svg' width='200mm' height='100mm' viewBox='0 0 200 100'>";
		string inch = "<svg xmlns='http://www.w3.org/2000/svg' width='2in' height='1in' viewBox='0 0 200 100'>";
		string px = "<svg xmlns='http://www.w3.org/2000/svg' width='200px' height='100px' viewBox='0 0 200 100'>";
		string rect = "<rect width='200' height='100'/>";
		Mesh a = Load(rect, mm), b = Load(rect, inch), c = Load(rect, px);
		return Near(a.Bounds.dimensions.x, 0.2f)    && Near(a.Bounds.dimensions.y, 0.1f)
			&& Near(b.Bounds.dimensions.x, 0.0508f) && Near(b.Bounds.dimensions.y, 0.0254f)
			&& Near(c.Bounds.dimensions.x, 2)       && Near(c.Bounds.dimensions.y, 1);
	}

	bool TestFillRules()
	{
		string outer = "M0 0 H100 V100 H0 Z";
		string same  = "M25 25 H75 V75 H25 Z"; // same winding as outer
		string flip  = "M25 25 V75 H75 V25 Z"; // opposite winding
		Mesh evenOdd     = Load($"<path d='{outer} {same}' fill-rule='evenodd'/>");
		Mesh nonzeroSame = Load($"<path d='{outer} {same}'/>");
		Mesh nonzeroFlip = Load($"<path d='{outer} {flip}'/>");
		return Near(Area(evenOdd,     out _), 0.75f)
			&& Near(Area(nonzeroSame, out _), 1.0f)
			&& Near(Area(nonzeroFlip, out _), 0.75f);
	}

	// Two rings sharing an edge, a vertex of one sits on the other's boundary.
	bool TestTouchingRings()
	{
		Mesh m = Load("<path d='M0 0 H50 V100 H0 Z M50 0 H100 V100 H50 Z'/>");
		return Near(Area(m, out bool cw), 1.0f) && cw;
	}

	// Later shapes come first in the index buffer, so the default depth test
	// keeps SVG's painter's order.
	bool TestPaintOrder()
	{
		Mesh m = Load("<rect width='100' height='100' fill='#ff0000'/><rect x='25' y='25' width='50' height='50' fill='#0000ff'/>");
		Vertex[] v = m.GetVerts();
		uint[]   i = m.GetInds();
		return m.VertCount == 8 && IsColor(v[i[0]].col, 0, 0, 255) && IsColor(v[i[i.Length-1]].col, 255, 0, 0);
	}

	// A four vertex rect can't carry a middle stop, so the fill gets split
	// along it.
	bool TestLinearGradient()
	{
		Mesh m = Load("<defs><linearGradient id='g' x1='0' y1='0' x2='1' y2='0'><stop offset='0' stop-color='#ff0000'/><stop offset='0.5' stop-color='#00ff00'/><stop offset='1' stop-color='#0000ff'/></linearGradient></defs><rect width='100' height='100' fill='url(#g)'/>");
		bool red = false, green = false, blue = false;
		foreach (Vertex v in m.GetVerts())
		{
			red   |= IsColor(v.col, 255, 0, 0) && Near(v.pos.x,  0.5f);
			green |= IsColor(v.col, 0, 255, 0) && Near(v.pos.x,  0.0f);
			blue  |= IsColor(v.col, 0, 0, 255) && Near(v.pos.x, -0.5f);
		}
		return red && green && blue && Near(Area(m, out _), 1.0f);
	}

	// A radial gradient needs interior vertices, a plain triangulated disc
	// only has its rim.
	bool TestRadialGradient()
	{
		Mesh m = Load("<defs><radialGradient id='g'><stop offset='0' stop-color='#ffffff'/><stop offset='1' stop-color='#ff0000'/></radialGradient></defs><circle cx='50' cy='50' r='40' fill='url(#g)'/>");
		bool white = false, red = false;
		foreach (Vertex v in m.GetVerts())
		{
			white |= v.col.g > 200 && v.col.b > 200;
			red   |= v.col.g < 20  && v.col.b < 20;
		}
		return white && red;
	}

	bool TestStrokeCaps()
	{
		string line(string cap) => $"<line x1='20' y1='50' x2='80' y2='50' stroke='#000' stroke-width='10' stroke-linecap='{cap}'/>";
		Mesh butt = Load(line("butt")), square = Load(line("square")), round = Load(line("round"));
		return Near(Area(butt,   out bool cw1), 0.06f) && cw1
			&& Near(Area(square, out bool cw2), 0.07f) && cw2
			&& Near(Area(round,  out bool cw3), 0.06f + (float)Math.PI * 0.05f*0.05f, 0.0005f) && cw3;
	}

	// An L shape, the join style decides how the outer corner is filled.
	bool TestStrokeJoins()
	{
		string path(string join) => $"<polyline points='10,10 60,10 60,60' fill='none' stroke='#000' stroke-width='10' stroke-linejoin='{join}'/>";
		Mesh miter = Load(path("miter")), bevel = Load(path("bevel")), round = Load(path("round"));
		float aMiter = Area(miter, out bool cw1), aBevel = Area(bevel, out bool cw2), aRound = Area(round, out bool cw3);
		return Near(aMiter, 0.1f) && cw1
			&& Near(aBevel, 0.1f - 0.00125f) && cw2
			&& aRound > aBevel && cw3;
	}

	// nanosvg was taught both of these, CSS inside CDATA and use references.
	bool TestUseAndCData()
	{
		Mesh m = Load("<style><![CDATA[ .box { fill: #00ff00; } ]]></style><defs><rect id='r' width='20' height='20'/></defs><use xlink:href='#r' class='box' x='40' y='40'/>");
		bool ok = m.VertCount == 4 && Near(m.Bounds.center.x, 0) && Near(m.Bounds.center.y, 0);
		foreach (Vertex v in m.GetVerts()) ok = ok && IsColor(v.col, 0, 255, 0);
		return ok;
	}

	bool TestErrors()
	{
		Log.Warn("Expected errors:");
		Mesh garbage = Mesh.FromMemory("bad.svg", System.Text.Encoding.UTF8.GetBytes("this is not an svg"));
		Mesh empty   = Load("");
		Mesh missing = Mesh.FromFile("does_not_exist.svg");
		Assets.BlockUntil(garbage, AssetState.Loaded);
		Assets.BlockUntil(missing, AssetState.Loaded);
		return garbage.AssetState < AssetState.None
			&& empty  .AssetState < AssetState.None
			&& missing.AssetState == AssetState.ErrorNotFound
			&& Mesh.FromFile("nope.txt") == null;
	}

	// Model picks a blend material only when something in the file is
	// translucent.
	bool TestModelTranslucency()
	{
		byte[] opaque = System.Text.Encoding.UTF8.GetBytes(head + "<rect width='100' height='100'/></svg>");
		byte[] faded  = System.Text.Encoding.UTF8.GetBytes(head + "<rect width='100' height='100' opacity='0.5'/></svg>");
		Model a = Model.FromMemory("opaque.svg", opaque);
		Model b = Model.FromMemory("faded.svg",  faded);
		Assets.BlockUntil(a, AssetState.Loaded);
		Assets.BlockUntil(b, AssetState.Loaded);
		return a.Visuals[0].Material.Transparency == Transparency.None
			&& b.Visuals[0].Material.Transparency == Transparency.Blend;
	}

	Mesh composite;
	public void Initialize()
	{
		Tests.Test(TestRect);
		Tests.Test(TestPhysicalUnits);
		Tests.Test(TestFillRules);
		Tests.Test(TestTouchingRings);
		Tests.Test(TestPaintOrder);
		Tests.Test(TestLinearGradient);
		Tests.Test(TestRadialGradient);
		Tests.Test(TestStrokeCaps);
		Tests.Test(TestStrokeJoins);
		Tests.Test(TestUseAndCData);
		Tests.Test(TestErrors);
		Tests.Test(TestModelTranslucency);

		composite = Load(
			"<defs><linearGradient id='g' x1='0' y1='0' x2='1' y2='0'><stop offset='0' stop-color='#ff0000'/><stop offset='0.5' stop-color='#00ff00'/><stop offset='1' stop-color='#0000ff'/></linearGradient>" +
			"<radialGradient id='r'><stop offset='0' stop-color='#ffffff'/><stop offset='1' stop-color='#ff6600'/></radialGradient>" +
			"<g id='star'><polygon points='0,-10 3,-3 10,-3 4.5,1.5 6.5,9 0,4.5 -6.5,9 -4.5,1.5 -10,-3 -3,-3' fill='#ffcc00'/></g></defs>" +
			"<rect x='5' y='5' width='40' height='20' fill='url(#g)'/>" +
			"<circle cx='70' cy='15' r='10' fill='url(#r)' stroke='#00aaff' stroke-width='3'/>" +
			"<path d='M10 35 H40 V55 H10 Z M17 40 H33 V50 H17 Z' fill='#8844cc' fill-rule='evenodd'/>" +
			"<path d='M50 40 Q60 30 70 40 T90 40' fill='none' stroke='#ff44aa' stroke-width='2.5' stroke-linecap='round' stroke-linejoin='round'/>" +
			"<polyline points='48,55 53,47 58,55 63,47' fill='none' stroke='#333333' stroke-width='1.5'/>" +
			"<rect x='10' y='65' width='80' height='30' fill='#ff0000'/><rect x='30' y='70' width='40' height='20' fill='#0000ff'/>" +
			"<use xlink:href='#star' x='75' y='60'/><use xlink:href='#star' x='90' y='58' transform='scale(0.8)'/>");
	}

	public void Shutdown() { }

	public void Step()
	{
		// In front of the default camera, turned to face it.
		composite.Draw(Material.Unlit, Matrix.TRS(V.XYZ(0, 0, -0.5f), Quat.FromAngles(0, 180, 0), 0.5f));
		Tests.Screenshot("Tests/Svg.jpg", 400, 400, 60, V.XYZ(0, 0, 0), V.XYZ(0, 0, -0.5f));
	}
}
