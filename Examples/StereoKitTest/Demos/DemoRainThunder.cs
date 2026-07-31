// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

using StereoKit;
using System;
using System.Collections.Concurrent;
using System.Threading.Tasks;

/// <summary>The audio system's acceptance scene: a real-time rainstorm
/// built entirely from the revamp's features. Individual drop transients
/// are cheap fire-and-forget voices, the rain wash is a pair of generated
/// ambisonic beds under a shaped-emitter far wash, and thunder is a
/// polyline lightning bolt played as staggered, propagation-delayed
/// instances whose rumble tails widen as they roll.
///
/// The synthesis is a demo-scale sketch of drop-population physics: pools
/// of ground and puddle drop variants are pre-rendered, each from its own
/// sampled diameter (a drop's contact time is ~d/v, so small fast drops
/// tick bright and big drops thud dark, and *declared loudness* follows
/// impact energy), spawned with Poisson timing across a wide ground disk
/// so distance and statistics make the patter irregular. Puddle hits fire
/// delayed bubble voices - deep plinks from big drops, a near-ultrasonic
/// Minnaert fizz from small ones - and big drops shatter into a delayed
/// splash crown of bright micro-droplet taps. The
/// thunder boom is a train of overlapping N-wave pulses put through a
/// steep lowpass - the pulse bodies without the high-frequency rasp,
/// which is what distance leaves of a strike.</summary>
class DemoRainThunder : ITest
{
	string title       = "Rain & Thunder";
	string description = "A real-time storm simulation exercising the audio system: fire-and-forget drop voices spawned by drop-size statistics, procedural ambisonic rain beds under a shaped-emitter far wash, and thunder that arrives along a lightning bolt with real propagation delay.\n\nEverything here is a declared real-world loudness - a drizzle droplet at ~40dB, thunder at 115dB - and the mix falls out of physics.";

	Pose   windowPose = (Demo.contentPose * Matrix.T(0, 0, 0)).Pose;
	Random rand       = new Random(1);

	// Pre-rendered variant pools: each variant is synthesized from its own
	// drop diameter, and *declares* the loudness that size of drop has.
	struct DropVariant { public Sound sound; public float dbOffset; public float d_mm; }
	DropVariant[] drops    = new DropVariant[16];
	DropVariant[] puddles  = new DropVariant[10];
	DropVariant[] plinks   = new DropVariant[5];
	DropVariant[] splashes = new DropVariant[6];
	DropVariant[] fizzes   = new DropVariant[3];

	Sound     washBright;
	Sound     washDark;
	Sound     washFar;
	Sound     boomSound;
	Sound     rumbleSound;
	SoundInst washBrightInst;
	SoundInst washDarkInst;
	SoundInst washFarInst;

	// Rain dials
	float rainIntensity = 0.5f;
	float gustiness     = 0.5f;
	float dropDb        = 55;
	float dropSpread    = 6;     // Radius of the drop disk, meters
	float plinkAmount   = 0.4f;
	float washDb        = 52;    // The far-field sits under the drop field
	float bedBright     = 0.8f;  // Bias on the near bed's bright/dark mix
	bool  showVoices    = false; // Debug spheres at every voice position

	// Thunder dials
	float thunderDist   = 0.25f; // 0-1 -> 80m..1200m
	float thunderEnergy = 0.5f;  // 0-1 -> pitch 1.35..0.65

	// Weather state
	float dropTimer  = 0;
	float gust       = 0.5f;
	float gustTarget = 0.5f;
	float gustTimer  = 0;

	// A rolling thunder event: staggered rumble voices along the bolt.
	SoundInst[] rumbleInsts = new SoundInst[4];
	SoundInst   boomInst;
	float       rumbleTime  = -1;
	int         frame       = 0;

	// Storms strike on their own: a random timer fires Thunder(), and the
	// lightning itself is a sky flash at strike time - light arrives
	// instantly while the boom's PropagationDelay supplies the real
	// flash-to-bang gap.
	//
	// A flash is one continuous brightness *envelope*, not a strobe: a few
	// return strokes down the same channel, each an instant spike decaying
	// over tens of ms (cloud scatter smears it), usually dimmer than the
	// last, 30-120ms apart - and often a low "continuing current" glow
	// simmers between them, wavering, which is what visually ties the
	// strokes together into one event.
	float thunderTimer = 10;
	bool  autoThunder  = true;
	SphericalHarmonics skyBase;
	SphericalHarmonics skyOriginal;
	AudioEnvironment   envOriginal;
	bool  skyFlashOn  = false;
	float flashLevel  = 0;       // Current stroke brightness, decaying
	float glowLevel   = 0;       // Continuing-current floor
	int   strokesLeft = 0;
	float strokeTimer = 0;       // Time until the next stroke
	float strokePeak  = 1;       // Diminishes down the chain
	Vec3  flashDir    = Vec3.Up;

	// Recent fire-and-forget voices, kept only so debug draw can see them.
	SoundInst[] recent   = new SoundInst[128];
	int         recentAt = 0;

	// Visible rain: a static cloud of droplet quads in a unit cube, the
	// shader wraps and falls them so the cube follows the head while the
	// droplets stay world-fixed. Ported from ProcIsland's sky renderer.
	const float rainCubeSize = 20;
	const float rainFloorY   = -1.5f; // Program.cs's floor plane
	Mesh        rainMesh;
	Material    rainMat;
	Mesh        splashMesh;
	Material    splashMat;

	float Gauss() => (float)(rand.NextDouble() + rand.NextDouble() + rand.NextDouble()) * 2 - 3;

	// Each droplet is 4 identical verts the shader expands into a streak
	// diamond: uv.x carries the corner index, uv.y a per-droplet random
	// used for both intensity culling and variation.
	static Mesh MakeRainMesh(int dropletCount)
	{
		var      r     = new Random(77);
		Vertex[] verts = new Vertex[dropletCount * 4];
		uint[]   inds  = new uint  [dropletCount * 6];
		for (int i = 0; i < dropletCount; i++)
		{
			Vec3  pos       = new Vec3((float)r.NextDouble(), (float)r.NextDouble(), (float)r.NextDouble());
			float variation = (float)r.NextDouble();
			int   vi        = i * 4;
			for (int v = 0; v < 4; v++)
				verts[vi + v] = new Vertex(pos, Vec3.Up, new Vec2(v, variation), Color32.White);

			int ii = i * 6;
			inds[ii + 0] = (uint)(vi + 0);
			inds[ii + 1] = (uint)(vi + 1);
			inds[ii + 2] = (uint)(vi + 2);
			inds[ii + 3] = (uint)(vi + 2);
			inds[ii + 4] = (uint)(vi + 1);
			inds[ii + 5] = (uint)(vi + 3);
		}
		Mesh mesh = new Mesh();
		mesh.SetData(verts, inds, calculateBounds: false);
		// The shader scatters verts across the whole cube, unit-cube
		// bounds would cull it.
		mesh.Bounds = new Bounds(Vec3.Zero, Vec3.One * rainCubeSize * 1.5f);
		return mesh;
	}

	// Splash quads share the droplet vertex layout, plus a per-splash
	// phase in the color channel so each loops its own little life.
	static Mesh MakeSplashMesh(int splashCount)
	{
		var      r     = new Random(78);
		Vertex[] verts = new Vertex[splashCount * 4];
		uint[]   inds  = new uint  [splashCount * 6];
		for (int i = 0; i < splashCount; i++)
		{
			Vec3    pos       = new Vec3((float)r.NextDouble(), 0, (float)r.NextDouble());
			float   variation = (float)r.NextDouble();
			Color32 phase     = new Color32((byte)r.Next(256), 0, 0, 255);
			int     vi        = i * 4;
			for (int v = 0; v < 4; v++)
				verts[vi + v] = new Vertex(pos, Vec3.Up, new Vec2(v, variation), phase);

			int ii = i * 6;
			inds[ii + 0] = (uint)(vi + 0);
			inds[ii + 1] = (uint)(vi + 1);
			inds[ii + 2] = (uint)(vi + 2);
			inds[ii + 3] = (uint)(vi + 2);
			inds[ii + 4] = (uint)(vi + 1);
			inds[ii + 5] = (uint)(vi + 3);
		}
		Mesh mesh = new Mesh();
		mesh.SetData(verts, inds, calculateBounds: false);
		mesh.Bounds = new Bounds(Vec3.Zero, Vec3.One * rainCubeSize * 1.5f);
		return mesh;
	}

	static float TapTau(float d_mm, bool water)
		=> water ? 0.0012f + 0.0004f * d_mm : 0.002f + 0.0008f * d_mm;

	// One impact tap: a noise burst through a 4-pole lowpass, rendered into
	// `into`, returning its length. This is the *single* synthesis path
	// behind the foreground pools and the beds - close patter and distant
	// wash drawn from one voice is what lets them read as the same rain.
	//
	// The steep skirt matters - hundreds of overlapping 2-pole tails sum
	// into an audible synthetic hiss, and a real impact's contact-time
	// smoothing rolls off much harder than 12dB/oct. The 0.55 keeps the
	// knee at `cut` while the skirt falls ~24dB/oct past it. Brightness
	// follows the contact time ~d/v: small fast drops tick bright, big
	// drops thud dark. Water is stiff with no give, so puddle taps are
	// brighter and shorter than the same drop on ground, and keep less of
	// the low thud - only soil takes the whole body blow.
	//
	// The attack is *instant*: the noise starts at full amplitude, and the
	// only onset smoothing is the filter's own rise - the contact time
	// itself. A shaped attack on top is how a surface with give sounds,
	// and it turns a ground smack into a leaf pat.
	static int RenderTap(Random r, float d_mm, bool water, float v_ms, float cutScale, float[] into)
	{
		float bright = Math.Clamp(v_ms / (3.3f * d_mm), 0.5f, 4);
		float tau    = TapTau(d_mm, water);
		float cut    = (water ? 2000 + 900 * bright : 500 + 600 * bright) * cutScale;
		float a      = 1.0f - MathF.Exp(-6.2831853f * MathF.Min(cut / 0.55f, 20000) / 48000.0f);
		float hpK    = water ? 0.0448f : 0.013f; // ~350Hz / ~100Hz strip
		int   len    = Math.Min((int)(tau * 6 * 48000), into.Length);
		float lp0 = 0, lp1 = 0, lp2 = 0, lp3 = 0, hp = 0;
		// The envelope is a geometric series and the noise is a xorshift -
		// an exp and a Random.NextDouble per sample are what made scene
		// startup slow, and white noise doesn't care where it comes from.
		float envK = MathF.Exp(-1.0f / (tau * 48000.0f));
		float env  = 1;
		uint  rng  = (uint)r.Next() | 1;
		for (int s = 0; s < len; s++) {
			rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
			float x = rng * (1.0f / 2147483648.0f) - 1.0f;
			lp0 += a * (x   - lp0);
			lp1 += a * (lp0 - lp1);
			lp2 += a * (lp1 - lp2);
			lp3 += a * (lp2 - lp3);
			hp  += hpK * (lp3 - hp);
			into[s] = (lp3 - hp) * env * 6;
			env *= envK;
		}
		return len;
	}

	// Pre-rendering all this audio synchronously stalls startup by over a
	// second. Buffers fill on background threads as pure math, and Step
	// turns finished pieces into Sounds on the main thread. Rain starts
	// with the first variants, and gains variety as the pools fill in.
	ConcurrentQueue<Action> genReady = new ConcurrentQueue<Action>();
	void GenAsync(int sampleCount, Action<float[]> fill, Action<float[]> finish)
	{
		Task.Run(() => {
			float[] buf = new float[sampleCount];
			fill(buf);
			genReady.Enqueue(() => finish(buf));
		});
	}

	// A pre-rendered foreground variant, filled into its pool slot when
	// generation lands. v_ms overrides terminal speed for drops that
	// didn't fall far - splash droplets.
	void MakeDrop(DropVariant[] pool, int index, int seed, float d_mm, bool water, float v_ms = 0, float dbBonus = 0)
	{
		if (v_ms == 0) v_ms = 9.65f - 10.3f * MathF.Exp(-0.6f * d_mm);
		float fall = v_ms;
		// Loudness follows impact energy: ~d^1.5*v in amplitude terms.
		float dbOffset = 20 * MathF.Log10(MathF.Pow(d_mm, 1.5f) * v_ms) - 26 + dbBonus;

		GenAsync((int)(TapTau(d_mm, water) * 6 * 48000),
			samples => RenderTap(new Random(seed), d_mm, water, fall, 1, samples),
			buf => pool[index] = new DropVariant {
				sound    = Sound.FromSamples(buf),
				dbOffset = dbOffset,
				d_mm     = d_mm });
	}

	// The pools fill in from background generation, so a pick lands on the
	// nearest finished variant - or nothing at all, in the first frames.
	static DropVariant PickReady(DropVariant[] pool, int pick)
	{
		for (int o = 0; o < pool.Length; o++)
		{
			if (pick - o >= 0          && pool[pick - o].sound != null) return pool[pick - o];
			if (pick + o < pool.Length && pool[pick + o].sound != null) return pool[pick + o];
		}
		return default;
	}

	// The near bed's content: far-field taps from the same ground+puddle
	// impact family as the foreground pools, each drawn from the drop
	// population, placed on the 6..28m annulus at amplitude 1/r - starting
	// where the foreground disk ends, so the bed only carries the field the
	// live drops don't already render - and encoded into first order
	// ambisonics from its true direction. Wrap at the loop boundary keeps
	// the seam silent, and cutScale bakes one bed bright and one dark for
	// the intensity crossfade.
	static void BedFill(float[] samples, int seed, float cutScale)
	{
		var     r   = new Random(seed);
		float[] tap = new float[2048];
		int     n   = samples.Length / 4; // Frames of interleaved W,Y,Z,X
		for (int e = 0; e < 6000; e++) {
			int   f0    = r.Next(n);
			float d_mm  = 0.6f + 4.4f * (float)(r.NextDouble() * r.NextDouble());
			float v_ms  = 9.65f - 10.3f * MathF.Exp(-0.6f * d_mm);
			bool  water = r.NextDouble() < 0.3;
			float rad   = MathF.Sqrt(36 + (float)r.NextDouble() * (28 * 28 - 36));
			float amp   = MathF.Pow(d_mm, 1.5f) * v_ms * (0.02f / rad)
			            * MathF.Exp(((float)(r.NextDouble() + r.NextDouble() + r.NextDouble()) * 2 - 3) * 0.3f);
			// ambiX encode gains from the tap's direction, ACN order W,Y,Z,X.
			float az = (float)r.NextDouble() * 6.2831853f;
			Vec3  u  = new Vec3(MathF.Cos(az) * rad, -1.5f, MathF.Sin(az) * rad).Normalized;
			float gy = -u.x, gz = u.y, gx = -u.z;

			// One SIMD multiply-add per frame sums the tap in from its
			// direction, with a wrapping index instead of a modulo.
			var frames = System.Runtime.InteropServices.MemoryMarshal.Cast<float, System.Numerics.Vector4>(samples.AsSpan());
			var enc    = new System.Numerics.Vector4(1, gy, gz, gx);
			int len    = RenderTap(r, d_mm, water, v_ms, cutScale, tap);
			int at     = f0;
			for (int i = 0; i < len; i++) {
				frames[at] += (tap[i] * amp) * enc;
				at += 1; if (at == n) at = 0;
			}
		}
	}

	// Everything falling past ~28m: a whole neighborhood of rain arriving
	// near-grazing, dragged over ground and obstacles - the highs are long
	// gone, so a fixed dark 4-pole knee is what's left of the population.
	static void FarWashFill(float[] samples)
	{
		var   r = new Random(45);
		int   n = samples.Length;
		float a = 1.0f - MathF.Exp(-6.2831853f * (700 / 0.55f) / 48000.0f);
		for (int e = 0; e < 14000; e++) {
			int   s0   = r.Next(n);
			float d_mm = 0.6f + 4.4f * (float)(r.NextDouble() * r.NextDouble());
			float v_ms = 9.65f - 10.3f * MathF.Exp(-0.6f * d_mm);
			float tau  = 0.002f + 0.0008f * d_mm;
			float amp  = MathF.Pow(d_mm, 1.5f) * v_ms * 0.0016f
			           * MathF.Exp(((float)(r.NextDouble() + r.NextDouble() + r.NextDouble()) * 2 - 3) * 0.3f);
			// Attack-decay envelope as two geometric series, xorshift noise,
			// wrapping index - the same trims that keep RenderTap fast.
			float lp0 = 0, lp1 = 0, lp2 = 0, lp3 = 0;
			float k1  = MathF.Exp(-1.0f / (tau * 48000.0f));
			float k2  = MathF.Exp(-1.0f / (tau * 0.18f * 48000.0f));
			float e1  = 1, e2 = 1;
			uint  rng = (uint)r.Next() | 1;
			int   len = (int)(tau * 6 * 48000);
			int   at  = s0;
			for (int i = 0; i < len; i++) {
				rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
				float x = rng * (1.0f / 2147483648.0f) - 1.0f;
				lp0 += a * (x   - lp0);
				lp1 += a * (lp0 - lp1);
				lp2 += a * (lp1 - lp2);
				lp3 += a * (lp2 - lp3);
				samples[at] += lp3 * (e1 - e2) * amp;
				e1 *= k1; e2 *= k2;
				at += 1; if (at == n) at = 0;
			}
		}
	}

	public void Initialize()
	{
		// -- Storm light --
		// Half the default skylight: overcast mood, and the lightning
		// flashes read far better against it. Restored at shutdown, as is
		// whatever acoustic environment the scene arrived with.
		envOriginal = Audio.Environment;
		skyOriginal = Renderer.SkyLight;
		SphericalHarmonics sky = skyOriginal;
		sky.coefficient1 *= 0.5f; sky.coefficient2 *= 0.5f; sky.coefficient3 *= 0.5f;
		sky.coefficient4 *= 0.5f; sky.coefficient5 *= 0.5f; sky.coefficient6 *= 0.5f;
		sky.coefficient7 *= 0.5f; sky.coefficient8 *= 0.5f; sky.coefficient9 *= 0.5f;
		Renderer.SkyLight = sky;

		// -- Drop variants --
		// Sample the population: mostly small drops, a fat tail of big
		// ones, on two surfaces. ~30% of the ground is puddle - the same
		// size population with the stiffer, brighter water voicing, and
		// the only drops that can ring a bubble. Energy goes with d^3*v^2,
		// which is why the declared loudness swings ~20dB across the pool.
		for (int i = 0; i < drops.Length; i++)
		{
			float t01 = i / (drops.Length - 1.0f);
			MakeDrop(drops, i, 100 + i, 0.6f + t01 * t01 * 4.4f, water: false);
		}
		for (int i = 0; i < puddles.Length; i++)
		{
			float t01 = i / (puddles.Length - 1.0f);
			MakeDrop(puddles, i, 200 + i, 0.6f + t01 * t01 * 4.4f, water: true);
		}

		// -- Splash crowns --
		// Drops past ~2.5mm shatter on landing into a crown of sub-mm
		// droplets that patter down around the impact a few ms later at
		// half the parent's speed - fast for their size, so they're the
		// brightest thing the rain does, and most of its top octave. One
		// voice stands in for the whole crown, declared a dozen dB over
		// a single droplet's energy to cover its brood.
		for (int i = 0; i < splashes.Length; i++)
		{
			float d_mm = 0.4f + 0.5f * i / (splashes.Length - 1.0f);
			MakeDrop(splashes, i, 300 + i, d_mm, water: true, v_ms: 4.6f, dbBonus: 12);
		}

		// -- Bubble fizz --
		// Pumphrey & Crum: drops under ~1.4mm entrain a ~0.2mm bubble on
		// nearly every water impact, ringing near 15kHz - the glassy fizz
		// of rain on a pond. Barely any of it crosses the water-air
		// interface, so it's a whisper, but in a band nothing else
		// touches. No range gate needed: the air-absorption model's
		// cutoff falls below these frequencies within ~10m, so only
		// nearby puddles fizz, exactly as they should.
		for (int i = 0; i < fizzes.Length; i++)
		{
			int   fi   = i; // The loop variable is shared across captures
			float r_mm = 0.20f + i * 0.05f;       // Bubble radius
			float freq = 3.26f / (r_mm * 0.001f); // True Minnaert, no cheat
			float q    = 12 + freq / 1500.0f;

			GenAsync((int)(0.015f * 48000), samples => {
				float decay = 6.2831853f * freq / (2 * q);
				for (int s = 0; s < samples.Length; s++) {
					float time = s / 48000.0f;
					samples[s] = MathF.Sin(time * freq * 6.2831853f) * MathF.Exp(time * -decay);
				}
			}, buf => fizzes[fi] = new DropVariant { sound = Sound.FromSamples(buf), dbOffset = -4 });
		}

		// -- Bubble plinks --
		// The entrained-bubble ring is its own quiet event after the tap:
		// low Q so it dies fast (high Q is a wine glass, not a puddle),
		// frequency scattered per bubble radius, pitch creeping up as the
		// bubble rises.
		for (int i = 0; i < plinks.Length; i++)
		{
			int   pi   = i; // The loop variable is shared across captures
			float r_mm = 0.35f + i * 0.24f;              // Bubble radius
			float freq = 3.26f / (r_mm * 0.001f) * 0.5f; // Minnaert-ish, kept audible
			float q    = 12 + freq / 1200.0f;

			GenAsync((int)(0.09f * 48000), samples => {
				float decay = 6.2831853f * freq / (2 * q);
				for (int s = 0; s < samples.Length; s++) {
					float time = s / 48000.0f;
					float f    = freq * (1 + time * 0.3f);
					samples[s] = MathF.Sin(time * f * 6.2831853f) * MathF.Exp(time * -decay);
				}
			// The audible sparkle rides above its tap
			}, buf => plinks[pi] = new DropVariant { sound = Sound.FromSamples(buf), dbOffset = 2 });
		}

		// -- Visible rain --
		rainMesh = MakeRainMesh(30000);
		rainMat  = new Material(Shader.FromFile("rain.hlsl"));
		rainMat.Transparency = Transparency.Blend;
		rainMat.FaceCull     = Cull.None;
		rainMat.DepthWrite   = false;
		rainMat.QueueOffset  = 11;

		splashMesh = MakeSplashMesh(1500);
		splashMat  = new Material(Shader.FromFile("rain_splash.hlsl"));
		splashMat.Transparency = Transparency.Blend;
		splashMat.FaceCull     = Cull.None;
		splashMat.DepthWrite   = false;
		splashMat.QueueOffset  = 11;
		splashMat[MatParamName.DiffuseTex] = Tex.GenParticle(64, 64, 1);

		// -- The far wash --
		// A sphere the listener stands inside: a shaped emitter inside its
		// own volume goes fully diffuse, surrounding the listener the same
		// way the drop field does. Its content is low frequency, where a
		// real diffuse field *is* interaurally coherent - the one wash
		// layer a mono loop renders honestly.
		GenAsync(4 * 48000, samples => FarWashFill(samples), buf => {
			washFar = Sound.FromSamples(buf);
			/// :CodeSample: SoundPlay SoundPlay.shape SoundInst.SetShape
			/// ### A shaped rain emitter
			/// Shapes turn a looping sound into an extended source: the
			/// emitter follows the listener along the shape, widens as it
			/// fills more of the view, and goes fully diffuse inside it - a
			/// rain bed the listener stands inside surrounds them completely.
			washFarInst = washFar.Play(Vec3.Zero, new SoundPlay {
				flags       = SoundFlags.Loop,
				shape       = new Vec3[] { new Vec3(0, 0, 0) },
				shapeRadius = 12,
			});
			/// :End:
		});

		// -- Thunder --
		// The boom: a channel radiates thousands of little N-waves. Up
		// close their edges read as a ripping crack, but at any distance
		// the air has eaten the highs and what arrives is the pulse
		// *bodies* - so the train goes through a steep lowpass and lands
		// as a deep front-loaded boom instead of a "blap".
		GenAsync((int)(1.2f * 48000), samples => {
			var r = new Random(46);
			int n = samples.Length;
			for (int e = 0; e < 700; e++) {
				float at  = -MathF.Log(1 - (float)r.NextDouble() * 0.985f) * 0.08f;
				int   s0  = (int)(at * 48000);
				float amp = MathF.Exp(at * -5) * (0.35f + 0.65f * (float)r.NextDouble());
				int   len = (int)(0.004f * MathF.Exp((float)r.NextDouble() * 1.1f - 0.55f) * 48000);
				for (int i = 0; i < len && s0 + i < n; i++)
					samples[s0 + i] += (1 - 2 * (i / (float)len)) * amp;
			}
			float lp1 = 0, lp2 = 0, lp3 = 0, lp4 = 0;
			for (int i = 0; i < n; i++) {
				lp1 += 0.045f * (samples[i] - lp1);
				lp2 += 0.045f * (lp1 - lp2);
				lp3 += 0.06f  * (lp2 - lp3);
				lp4 += 0.06f  * (lp3 - lp4);
				samples[i] = lp4 * 8;
			}
		}, buf => {
			boomSound = Sound.FromSamples(buf);
			boomSound.Decibels = 125;
		});

		// The rumble: deep 4-pole noise with a slow, uneven "roll" - the
		// turbulence-scattered swelling of far thunder, not a static tone.
		GenAsync((int)(4.5f * 48000), samples => {
			var   r = new Random(47);
			float lp1 = 0, lp2 = 0, lp3 = 0, lp4 = 0;
			float roll = 0.6f, rollTarget = 0.6f;
			int   rollAt = 0;
			for (int i = 0; i < samples.Length; i++) {
				float t = i / 48000.0f;
				if (i >= rollAt) {
					rollTarget = 0.25f + 0.75f * (float)r.NextDouble();
					rollAt     = i + (int)((0.25f + (float)r.NextDouble() * 0.8f) * 48000);
				}
				roll += (rollTarget - roll) * 0.00006f;

				float env   = MathF.Min(t * 3, 1) * MathF.Exp(t * -0.8f);
				float noise = (float)(r.NextDouble() * 2 - 1);
				lp1 += 0.035f * (noise - lp1);
				lp2 += 0.035f * (lp1   - lp2);
				lp3 += 0.045f * (lp2   - lp3);
				lp4 += 0.045f * (lp3   - lp4);
				samples[i] = lp4 * env * roll * 55;
			}
		}, buf => {
			rumbleSound = Sound.FromSamples(buf);
			rumbleSound.Decibels = 115;
		});

		// The near bed is the Campbell wash made literal: all the drops too
		// far to individuate, summed from the same impact family as the
		// foreground so bed and patter read as one rain. A mono loop played
		// diffuse reaches both ears identical - coherent, narrow, in the
		// head - where a real diffuse field is decorrelated at the ears.
		// Encoding each tap from its own direction makes the bed a *true*
		// diffuse field, and the decode's ear paths do the rest.
		GenAsync(4 * 48000 * 4, samples => BedFill(samples, 48, 1.5f), buf => {
			washBright     = Sound.FromSamples(buf, SoundChannels.Ambisonic1);
			washBrightInst = washBright.Play(Vec3.Zero, new SoundPlay { flags = SoundFlags.Loop });
		});
		GenAsync(4 * 48000 * 4, samples => BedFill(samples, 49, 0.6f), buf => {
			washDark     = Sound.FromSamples(buf, SoundChannels.Ambisonic1);
			washDarkInst = washDark.Play(Vec3.Zero, new SoundPlay { flags = SoundFlags.Loop });
		});
	}

	public void Shutdown()
	{
		Audio.Environment = envOriginal;
		Renderer.SkyLight = skyOriginal;
		skyFlashOn        = false;
		washBrightInst.Stop();
		washDarkInst  .Stop();
		washFarInst   .Stop();
		for (int i = 0; i < rumbleInsts.Length; i++) rumbleInsts[i].Stop();
	}

	void SpawnDrop(float effIntensity)
	{
		// Heavier rain has disproportionately more big drops (the fat
		// tail), so intensity skews the variant pick toward the deep end.
		float shape = 2.2f - effIntensity * 1.3f;
		bool  water = rand.NextDouble() < 0.3;
		DropVariant[] pool = water ? puddles : drops;
		int   pick  = (int)(MathF.Pow((float)rand.NextDouble(), shape) * pool.Length);
		pick = Math.Min(pick, pool.Length - 1);
		DropVariant v = PickReady(pool, pick);
		if (v.sound == null) return;

		// Thunder's foreground split: a drop is worth a voice only inside
		// its own audibility radius, so each size spawns uniformly over
		// *that* disk - small drops tick right around your feet where they
		// can still be heard, big smacks land anywhere, and no voice is
		// spent where the bed already covers it. Half the dB slope keeps
		// the mid sizes off the floor. The 0.7m floor stays fixed as
		// Spread grows: drops landing right at your feet are the
		// individually audible "character" drops, and pushing the minimum
		// out kills the foreground entirely.
		float rc01 = Math.Clamp(MathF.Pow(10, v.dbOffset / 40) * 0.45f, 0.12f, 1);
		float ang  = (float)rand.NextDouble() * 6.2831853f;
		float rad  = 0.7f + dropSpread * rc01 * MathF.Sqrt((float)rand.NextDouble());
		Vec3  at   = new Vec3(MathF.Cos(ang) * rad, -1.3f, MathF.Sin(ang) * rad);

		// Dense rain spawns several drops per frame; scattering their
		// onsets across the frame keeps them off the frame-rate grid.
		float jitter = (float)rand.NextDouble() * 0.012f;

		// Per-drop lognormal loudness jitter on top of the size's dB.
		recent[recentAt++ % recent.Length] = v.sound.Play(at, new SoundPlay {
			delay  = jitter,
			pitch  = 0.85f + (float)rand.NextDouble() * 0.3f,
			volume = MathF.Exp(Gauss() * 0.45f) * 0.85f,
		});

		// Big drops shatter into a splash crown: 1-2 bright micro-droplet
		// taps pattering down around the impact. Puddles - a film of
		// standing water - crown more readily than soaked ground.
		if (v.d_mm > 2.5f && (float)rand.NextDouble() < (water ? 0.5f : 0.25f))
		{
			int children = rand.NextDouble() < 0.4 ? 2 : 1;
			for (int c = 0; c < children; c++)
			{
				DropVariant s   = PickReady(splashes, rand.Next(splashes.Length));
				if (s.sound == null) break;
				Vec3        sAt = at + new Vec3(Gauss() * 0.05f, 0, Gauss() * 0.05f);
				recent[recentAt++ % recent.Length] = s.sound.Play(sAt, new SoundPlay {
					delay  = jitter + 0.004f + 0.026f * (float)rand.NextDouble(),
					pitch  = 0.9f + (float)rand.NextDouble() * 0.3f,
					volume = MathF.Exp(Gauss() * 0.4f) * 0.7f,
				});
			}
		}

		// A puddle impact entrains a bubble that rings once the crater
		// collapses: small drops pinch off a near-certain fleck of high
		// fizz, big ones the occasional deeper audible plink. Plinks are
		// a *near-field* sparkle - only drops landing close are worth
		// one - while the fizz thins with distance on its own.
		if (water && v.d_mm <= 1.4f && (float)rand.NextDouble() < plinkAmount * 1.75f)
		{
			DropVariant f = PickReady(fizzes, rand.Next(fizzes.Length));
			if (f.sound == null) return;
			recent[recentAt++ % recent.Length] = f.sound.Play(at, new SoundPlay {
				delay  = jitter + 0.003f + 0.003f * (float)rand.NextDouble(),
				pitch  = 0.9f + (float)rand.NextDouble() * 0.25f,
				volume = MathF.Exp(Gauss() * 0.3f),
			});
		}
		else if (water && v.d_mm > 1.4f && rad < dropSpread * 0.75f && (float)rand.NextDouble() < plinkAmount)
		{
			DropVariant p = PickReady(plinks, rand.Next(plinks.Length));
			if (p.sound == null) return;
			recent[recentAt++ % recent.Length] = p.sound.Play(at, new SoundPlay {
				delay  = jitter + 0.004f + 0.004f * (float)rand.NextDouble(),
				pitch  = 0.9f + (float)rand.NextDouble() * 0.3f,
				volume = MathF.Exp(Gauss() * 0.3f) * 0.8f,
			});
		}
	}

	void Thunder()
	{
		// A strike before the thunder sounds finish generating still gets
		// its flash - light doesn't wait on the audio pipeline either.
		if (boomSound == null || rumbleSound == null)
		{
			float ang = (float)(rand.NextDouble() * Math.PI * 2);
			Flash(new Vec3(MathF.Cos(ang), 0.25f, MathF.Sin(ang)));
			return;
		}

		/// :CodeSample: Sound.Play SoundPlay SoundFlags SoundInst.Spread
		/// ### Thunder along a lightning bolt
		/// Sounds declare real-world loudness in decibels, and
		/// SoundFlags.PropagationDelay delays each voice's onset by its
		/// distance at the speed of sound - so a thunder boom and its
		/// rumble arrive along the bolt just like the real thing. Energy
		/// sets the pitch here: bigger strikes boom deeper.
		// A jagged bolt across the sky: the boom fires from its nearest
		// point, then rumble instances arrive progressively later from
		// farther along it - each duller (distance is a low-pass filter)
		// and wider than the last.
		float dist  = 80 + thunderDist * 1120;
		float dir   = (float)(rand.NextDouble() * Math.PI * 2);
		Vec3  near  = new Vec3(MathF.Cos(dir), 0, MathF.Sin(dir)) * dist + Vec3.Up * 90;
		Vec3  far   = near + new Vec3(MathF.Sin(dir), 0.5f, -MathF.Cos(dir)) * (100 + (float)rand.NextDouble() * 300);
		float pitch = 1.35f - thunderEnergy * 0.7f;

		boomInst = boomSound.Play(near, new SoundPlay {
			flags = SoundFlags.PropagationDelay,
			pitch = pitch,
		});

		for (int i = 0; i < rumbleInsts.Length; i++)
		{
			float t  = (i + 1) / (float)rumbleInsts.Length;
			Vec3  at = Vec3.Lerp(near, far, t);
			rumbleInsts[i] = rumbleSound.Play(at, new SoundPlay {
				flags  = SoundFlags.PropagationDelay,
				delay  = t * 0.5f,
				pitch  = pitch * (1.0f - t * 0.25f),
				volume = 1.0f - t * 0.3f,
				spread = 0.2f,
				cutoff = 900 - t * 550, // Air absorption along the bolt
			});
		}
		/// :End:
		rumbleTime = 0;

		// The flash: light outruns sound, so the sky lights up now and the
		// boom arrives dist/343 seconds later on its own. The first stroke
		// fires immediately, the rest follow down the same channel.
		Flash(near);
	}

	void Flash(Vec3 towards)
	{
		flashDir    = towards.Normalized;
		strokesLeft = 1 + rand.Next(4);
		strokeTimer = 0;
		strokePeak  = 1;
	}

	// Debug spheres at every live voice: drops small and white, plinks
	// ride along, the far wash at its emit point (your head when inside
	// its sphere; the ambisonic beds have no position), thunder large so
	// it reads at bolt distance. Intensity drives a glow so you can also
	// *see* what's currently sounding.
	void DrawVoices()
	{
		for (int i = 0; i < recent.Length; i++)
		{
			if (!recent[i].IsPlaying) continue;
			float glow = 0.3f + recent[i].Intensity * 3;
			Mesh.Sphere.Draw(Material.Default, Matrix.TS(recent[i].Position, 0.08f), new Color(glow, glow, 1));
		}
		if (washFarInst.IsPlaying)
			Mesh.Sphere.Draw(Material.Default, Matrix.TS(washFarInst.Position, 0.25f), new Color(0.2f, 0.8f, 0.4f));
		if (boomInst.IsPlaying)
			Mesh.Sphere.Draw(Material.Default, Matrix.TS(boomInst.Position, 8), new Color(1, 0.9f, 0.3f));
		for (int i = 0; i < rumbleInsts.Length; i++)
		{
			if (!rumbleInsts[i].IsPlaying) continue;
			float glow = 0.4f + rumbleInsts[i].Intensity * 2;
			Mesh.Sphere.Draw(Material.Default, Matrix.TS(rumbleInsts[i].Position, 6), new Color(glow, glow * 0.6f, 0.2f));
		}
	}

	void StepLightning()
	{
		// Fire on our own now and again. The timer leans shorter when the
		// storm is closer, the way strike rates feel as a cell approaches.
		if (autoThunder && rumbleTime < 0)
		{
			thunderTimer -= Time.Stepf;
			if (thunderTimer < 0)
			{
				Thunder();
				thunderTimer = SKMath.Lerp(6, 20, thunderDist) * (0.6f + (float)rand.NextDouble());
			}
		}

		// Fire pending strokes: an instant spike, usually dimmer than the
		// last, and about half of them leave a continuing-current simmer.
		if (strokesLeft > 0)
		{
			strokeTimer -= Time.Stepf;
			if (strokeTimer <= 0)
			{
				strokesLeft -= 1;
				flashLevel   = MathF.Max(flashLevel, strokePeak * (0.75f + 0.5f * (float)rand.NextDouble()));
				strokePeak  *= 0.65f;
				strokeTimer  = 0.03f + (float)rand.NextDouble() * 0.09f;
				if (rand.NextDouble() < 0.5)
					glowLevel = MathF.Max(glowLevel, flashLevel * 0.12f);
			}
		}

		// The envelope: strokes decay fast (~40ms visual tau through the
		// cloud), the simmer decays slow and wavers, and the sky light is
		// the base with the bolt's light *added* at the current level.
		float wobble = 0.75f + 0.25f * MathF.Sin(Time.Totalf * 37) * MathF.Sin(Time.Totalf * 23);
		flashLevel   = MathF.Max(flashLevel * MathF.Exp(-Time.Stepf / 0.04f), glowLevel * wobble);
		glowLevel   *= MathF.Exp(-Time.Stepf / 0.22f);
		if (glowLevel  < 0.005f) glowLevel = 0;

		bool lit = flashLevel > 0.008f || strokesLeft > 0;
		if (lit)
		{
			if (!skyFlashOn) { skyBase = Renderer.SkyLight; skyFlashOn = true; }
			SphericalHarmonics sky = skyBase;
			sky.Add(flashDir, new Color(1, 0.96f, 1)     * 6.0f * flashLevel);
			sky.Add(Vec3.Up,  new Color(0.8f, 0.8f, 1)   * 2.4f * flashLevel);
			Renderer.SkyLight = sky;
		}
		else if (skyFlashOn)
		{
			Renderer.SkyLight = skyBase;
			skyFlashOn        = false;
		}
	}

	public void Step()
	{
		// Turn finished background buffers into Sounds, a couple per frame
		// so a big piece landing never hitches a frame.
		for (int i = 0; i < 2 && genReady.TryDequeue(out Action finish); i++)
			finish();

		StepLightning();

		// Gusts: a slow random walk that swells everything coherently -
		// drop rate, big-drop share, and the wash together.
		gustTimer -= Time.Stepf;
		if (gustTimer < 0) {
			gustTarget = (float)rand.NextDouble();
			gustTimer  = 2 + (float)rand.NextDouble() * 3;
		}
		gust += (gustTarget - gust) * Math.Min(1, Time.Stepf / 1.5f);
		float effIntensity = rainIntensity * (1 - gustiness * 0.6f + gustiness * 1.2f * gust);

		// The dB dial shifts the whole population together, each variant
		// keeping its size-derived offset. Nulls are still generating.
		for (int i = 0; i < drops.Length;    i++) if (drops[i].sound    != null) drops[i].sound.Decibels    = dropDb + drops[i].dbOffset;
		for (int i = 0; i < puddles.Length;  i++) if (puddles[i].sound  != null) puddles[i].sound.Decibels  = dropDb + puddles[i].dbOffset;
		for (int i = 0; i < plinks.Length;   i++) if (plinks[i].sound   != null) plinks[i].sound.Decibels   = dropDb + plinks[i].dbOffset;
		for (int i = 0; i < splashes.Length; i++) if (splashes[i].sound != null) splashes[i].sound.Decibels = dropDb + splashes[i].dbOffset;
		for (int i = 0; i < fizzes.Length;   i++) if (fizzes[i].sound   != null) fizzes[i].sound.Decibels   = dropDb + fizzes[i].dbOffset;

		// Poisson drop spawning: exponential gaps, so arrivals cluster
		// and lull instead of ticking like a metronome. Foreground density
		// is the grain that makes rain read as rain instead of hiss, so a
		// full pour runs ~2000 drops a second - sized to fill the mixer's
		// whole voice budget, since a drop tap only lives ~20ms. Gust peaks
		// push past it, and the audibility ranking sheds the quietest taps.
		dropTimer -= Time.Stepf;
		int safety = 0;
		while (dropTimer < 0 && safety++ < 200)
		{
			SpawnDrop(effIntensity);
			float mean = 0.0005f / Math.Max(0.05f, effIntensity);
			dropTimer += -MathF.Log(1 - (float)rand.NextDouble() * 0.9995f) * mean;
		}

		// The beds track the weather: gusts swell them, and heavier rain
		// shifts its energy into fatter, duller drops, sliding the near
		// bed from bright drizzle-patter toward a dark downpour roar. The
		// Bright dial biases that balance. Campbell says bed power tracks
		// the drop rate, so the whole wash scales with rain amount - down
		// to silence when the rain stops.
		float eff01    = Math.Min(1, effIntensity);
		float brightAt = Math.Clamp((1 - 0.65f * eff01) * bedBright * 2, 0, 1);
		float swell    = MathF.Pow(eff01, 0.8f);
		if (washBright != null) washBright.Decibels = washDb;
		if (washDark   != null) washDark  .Decibels = washDb;
		if (washFar    != null) washFar   .Decibels = washDb - 3;
		washBrightInst.Volume = MathF.Sqrt(brightAt)     * swell;
		washDarkInst  .Volume = MathF.Sqrt(1 - brightAt) * swell;
		washFarInst   .Volume = swell * (0.85f + 0.15f * eff01);

		// Rumble tails widen as they roll - spread is animatable per voice.
		if (rumbleTime >= 0)
		{
			rumbleTime += Time.Stepf;
			bool rolling = boomInst.IsPlaying;
			for (int i = 0; i < rumbleInsts.Length; i++)
			{
				if (!rumbleInsts[i].IsPlaying) continue;
				rolling = true;
				float t = (i + 1) / (float)rumbleInsts.Length;
				rumbleInsts[i].Spread = Math.Min(0.9f, 0.2f + rumbleTime * 0.25f * (0.5f + t));
			}
			// The strike is over once the boom and every rumble have
			// finished - rearm so the auto timer can count toward the next.
			if (!rolling) rumbleTime = -1;
		}

		UI.WindowBegin("Rain & Thunder", ref windowPose, new Vec2(0.3f, 0));

		UI.Text("Rain", Align.TopCenter);
		UI.PanelBegin();
		UI.Label("Amount", new Vec2(0.07f, 0)); UI.SameLine(); UI.HSlider("amt",   ref rainIntensity, 0, 1, 0);
		UI.Label("Gusts",  new Vec2(0.07f, 0)); UI.SameLine(); UI.HSlider("gust",  ref gustiness,     0, 1, 0);
		UI.Label("Spread", new Vec2(0.07f, 0)); UI.SameLine(); UI.HSlider("sprd",  ref dropSpread,    2, 16, 0);
		UI.Label("Plinks", new Vec2(0.07f, 0)); UI.SameLine(); UI.HSlider("plink", ref plinkAmount,   0, 0.8f, 0);
		UI.Label("Drop dB",new Vec2(0.07f, 0)); UI.SameLine(); UI.HSlider("ddb",   ref dropDb,   40, 65, 0);
		UI.Label("Wash dB",new Vec2(0.07f, 0)); UI.SameLine(); UI.HSlider("wdb",   ref washDb,   50, 80, 0);
		UI.Label("Bright", new Vec2(0.07f, 0)); UI.SameLine(); UI.HSlider("brt",   ref bedBright, 0, 1, 0);
		UI.PanelEnd();

		UI.Text("Thunder", Align.TopCenter);
		UI.PanelBegin();
		UI.Label("Distance", new Vec2(0.07f, 0)); UI.SameLine(); UI.HSlider("tdist", ref thunderDist,   0, 1, 0);
		UI.Label("Energy",   new Vec2(0.07f, 0)); UI.SameLine(); UI.HSlider("tengy", ref thunderEnergy, 0, 1, 0);
		UI.PushEnabled(boomSound != null && rumbleSound != null);
		if (UI.Button("Thunder!")) Thunder();
		UI.PopEnabled();
		UI.SameLine();
		UI.Toggle("Auto", ref autoThunder);
		UI.SameLine();
		UI.Label($"{Audio.OutputDecibels,6:F1} dBFS");
		UI.SameLine();
		UI.Toggle("Show", ref showVoices);
		UI.PanelEnd();

		UI.WindowEnd();

		if (showVoices) DrawVoices();

		frame += 1;
		if (Tests.IsTesting && frame == 2)
			Thunder();

		// The visible rain: thickness rides the same gust-modulated
		// intensity as the audio, drops fatten slightly in heavy rain,
		// and droplets stop at the floor where the splash layer takes
		// over. Both are lit per-vertex by the skylight SH in-shader, so
		// the sky's color, brightness, and lightning flashes all land on
		// the water - rain_color is just the albedo, tuned in the shader
		// defaults.
		Vec3 head     = Input.Head.position;
		Vec4 rParams  = new Vec4(Time.Totalf, rainCubeSize, 9, MathF.Max(9 * Time.Stepf, 0.02f));
		Vec4 rSetting = new Vec4(eff01, 0.8f + 0.5f * eff01, rainFloorY, 0);
		Vec4 rHead    = new Vec4(head.x, head.y, head.z, 0);
		rainMat["rain_head_pos"]   = rHead;
		rainMat["rain_params"]     = rParams;
		rainMat["rain_settings"]   = rSetting;
		splashMat["rain_head_pos"] = rHead;
		splashMat["rain_params"]   = rParams;
		splashMat["rain_settings"] = rSetting;
		rainMesh  .Draw(rainMat,   Matrix.T(head));
		splashMesh.Draw(splashMat, Matrix.T(head));

		Demo.ShowSummary(title, description, new Bounds(V.XY0(0, -0.12f), V.XYZ(.34f, .34f, 0.1f)));
		Tests.Screenshot("Demos/RainThunder.jpg", 600, 600, windowPose.position + V.XYZ(0, -0.12f, 0.42f), windowPose.position + V.XYZ(0, -0.12f, 0));
	}
}
