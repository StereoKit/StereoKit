using StereoKit;

class TestBounds : ITest
{
	bool TestBoundsScaledScalar()
	{
		var inputBounds = new Bounds(Vec3.Zero, Vec3.One);

		const float scale = 0.1f;
		var scaledScalarByOperator = inputBounds * scale;
		var scaledScalarByFunction = inputBounds.Scaled(scale);

		if (
			scaledScalarByFunction.center.v != scaledScalarByOperator.center.v
			|| scaledScalarByFunction.dimensions.v != scaledScalarByOperator.dimensions.v
		)
		{
			return false;
		}

		if (!scaledScalarByOperator.center.Equals(Vec3.Zero))
			return false;
		if (!scaledScalarByOperator.dimensions.Equals(new Vec3(0.1f, 0.1f, 0.1f)))
			return false;

		return true;
	}

	bool TestBoundsScaledScalarIsPure()
	{
		// Scaled is expected to return a new Bounds without modifying the 'this' Bounds.
		var inputBounds = new Bounds(Vec3.Zero, Vec3.One);

		const float scale = 0.1f;
		var scaledBounds1 = inputBounds.Scaled(scale);
		var scaledBounds2 = inputBounds.Scaled(scale);

		// Repeating the operation should be idempotent
		if (!scaledBounds1.Equals(scaledBounds2))
		{
			return false;
		}

		if (inputBounds.dimensions.v != Vec3.One) // Ensure input is not scaled.
		{
			return false;
		}

		return true;
	}

	bool TestBoundsScale()
	{
		// Scale is expected to modify in-place.
		var inputBounds = new Bounds(Vec3.Zero, Vec3.One);

		const float scale = 0.1f;
		inputBounds.Scale(scale);

		if (
			inputBounds.center.v != Vec3.Zero
			&& inputBounds.dimensions.v != new Vec3(0.1f, 0.1f, 0.1f)
		)
		{
			return false;
		}

		return true;
	}

	bool TestBoundsScaleByVec3()
	{
		// Scale is expected to modify in-place.
		var inputBounds = new Bounds(Vec3.One, Vec3.One);

		var scale = new Vec3(0.1f, 0.2f, 0.3f);
		inputBounds.Scale(scale);

		if (
			inputBounds.center.v != new Vec3(0.1f, 0.2f, 0.3f)
			&& inputBounds.dimensions.v != new Vec3(0.1f, 0.2f, 0.3f)
		)
		{
			return false;
		}

		return true;
	}

	bool TestBoundsScaledByVec3()
	{
		var inputBounds = new Bounds(Vec3.One, Vec3.One);

		var scale = new Vec3(0.1f, 0.2f, 0.3f);
		var scaledBounds = inputBounds.Scaled(scale);

		if (
			scaledBounds.center.v != new Vec3(0.1f, 0.2f, 0.3f)
			&& scaledBounds.dimensions.v != new Vec3(0.1f, 0.2f, 0.3f)
		)
		{
			return false;
		}

		return true;
	}

	public void Initialize()
	{
		Tests.Test(TestBoundsScaledScalar);
		Tests.Test(TestBoundsScaledByVec3);
		Tests.Test(TestBoundsScaledScalarIsPure);
		Tests.Test(TestBoundsScale);
		Tests.Test(TestBoundsScaleByVec3);
	}

	public void Shutdown() { }

	public void Update() { }
}
