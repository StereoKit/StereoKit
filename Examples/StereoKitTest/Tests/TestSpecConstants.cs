using StereoKit;
using System;

// Verifies specialization constants routed through the ordinary scalar
// setters/getters: default values, overrides, persistence across pipeline
// rebuilds, Material.Copy, shader swaps, and compute dispatch.
class TestSpecConstants : ITest
{
	Material             _matDefault;
	Material             _matOverride;
	Compute              _compute;
	ComputeBuffer<float> _computeBuf;
	int                  _frame;

	static bool Near(float a, float b) => Math.Abs(a - b) < 0.001f;

	public void Initialize()
	{
		Shader shader = Shader.FromFile("spec_constant_test.hlsl");

		// Defaults come straight from the HLSL declarations.
		_matDefault = new Material(shader);
		Tests.Test(() => _matDefault.GetInt  ("COLOR_STEPS") == 2);
		Tests.Test(() => Near(_matDefault.GetFloat("BRIGHTNESS"), 0.25f));
		Tests.Test(() => _matDefault.GetBool ("USE_TINT")    == true);
		Tests.Test(() => _matDefault.GetUInt ("BLUE_SCALE")  == 4);

		// Overriding a spec constant via the normal setters bakes a new
		// pipeline; the getter reports the effective value.
		_matOverride = new Material(shader);
		_matOverride.SetInt  ("COLOR_STEPS", 4);
		_matOverride.SetFloat("BRIGHTNESS",  0.6f);
		_matOverride.SetBool ("USE_TINT",    false);
		_matOverride.SetUInt ("BLUE_SCALE",  12);
		Tests.Test(() => _matOverride.GetInt  ("COLOR_STEPS") == 4);
		Tests.Test(() => Near(_matOverride.GetFloat("BRIGHTNESS"), 0.6f));
		Tests.Test(() => _matOverride.GetBool ("USE_TINT")    == false);
		Tests.Test(() => _matOverride.GetUInt ("BLUE_SCALE")  == 12);

		// Regression: overrides must survive an unrelated pipeline-state change
		// (cull/transparency both rebuild the pipeline via material_build_info).
		_matOverride.FaceCull     = Cull.Front;
		_matOverride.Transparency = Transparency.Blend;
		Tests.Test(() => _matOverride.GetInt ("COLOR_STEPS") == 4);
		Tests.Test(() => _matOverride.GetUInt("BLUE_SCALE")  == 12);

		// Material.Copy carries spec overrides.
		Material copy = _matOverride.Copy();
		Tests.Test(() => copy.GetInt  ("COLOR_STEPS") == 4);
		Tests.Test(() => copy.GetBool ("USE_TINT")    == false);

		// Swapping to a shader without the constant drops the override; swapping
		// back yields the HLSL default rather than the stale override.
		Material swap = new Material(shader);
		swap.SetInt("COLOR_STEPS", 5);
		swap.Shader = Shader.Unlit;
		swap.Shader = shader;
		Tests.Test(() => swap.GetInt("COLOR_STEPS") == 2);

		// Compute getters already prove reflection + override storage + resolve;
		// the dispatch output is checked across frames in Step (see below).
		_compute    = new Compute(Shader.FromFile("Shaders/spec_constant_compute.hlsl"));
		_computeBuf = new ComputeBuffer<float>(ComputeBufferType.ReadWrite, 1);
		_compute.SetStorage("output", _computeBuf);
		Tests.Test(() => Near(_compute.GetFloat("SCALE"),    2.0f));
		Tests.Test(() => _compute.GetInt("ADD_COUNT")     == 3);

		// Give the compute dispatch/readback state machine room to run — reads
		// happen several frames after each dispatch so the GPU work completes
		// (skr_buffer_get does no GPU sync, DispatchNow submits at frame end).
		Tests.RunForFrames(11);
	}

	public void Shutdown() { }

	public void Step()
	{
		// Compute dispatch needs an active frame; read back with a gap so the
		// dispatched frame has been submitted and completed.
		switch (_frame)
		{
			// Default: ADD_COUNT(3) * SCALE(2) + OFFSET(10) = 16
			case 0: _compute.DispatchNow(1, 1, 1); break;
			case 3:
				Tests.Test(() => Near(_computeBuf.GetData()[0], 16.0f));
				// Override SCALE; the already-bound buffer must persist across
				// the pipeline rebuild (no re-bind here). 3 * 3 + 10 = 19
				_compute.SetFloat("SCALE", 3.0f);
				_compute.DispatchNow(1, 1, 1);
				break;
			case 6:
				Tests.Test(() => Near(_computeBuf.GetData()[0], 19.0f));
				// Toggling the bool spec constant negates: -(3 * 3 + 10) = -19
				_compute.SetBool("NEGATE", true);
				_compute.DispatchNow(1, 1, 1);
				break;
			case 9:
				Tests.Test(() => Near(_computeBuf.GetData()[0], -19.0f));
				Tests.Test(() => Near(_compute.GetFloat("SCALE"), 3.0f));
				Tests.Test(() => _compute.GetBool("NEGATE") == true);
				break;
		}
		_frame++;

		// Two materials sharing one shader, distinct spec-constant pipeline
		// variants, rendered side by side.
		Mesh.Sphere.Draw(_matDefault,  Matrix.TS(new Vec3(-0.1f, 0, 0), 0.16f));
		Mesh.Sphere.Draw(_matOverride, Matrix.TS(new Vec3( 0.1f, 0, 0), 0.16f));

		Tests.Screenshot("Tests/SpecConstants.jpg", 400, 200, 55, new Vec3(0, 0, 0.35f), Vec3.Zero);
	}
}
