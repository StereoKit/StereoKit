using StereoKit;

class DocModel : ITest
{
    Model testModel;

    bool TestModel()
    {
        /// :CodeSample: Model.Model
        Model model = new Model();
        model.AddSubset(
            Mesh.GenerateCube(Vec3.One),
            Default.Material,
            Matrix.Identity);
        /// :End:
        /// :CodeSample: Model.AddSubset
        model.AddSubset(
            Mesh.GenerateSphere(1),
            Default.Material,
            Matrix.T(0,1,0));
        /// :End:
        
        int lastSubset = model.AddSubset(
            Mesh.GenerateCube(new Vec3(0.5f,1,0.5f)),
            Default.Material,
            Matrix.T(0, 1, 0));

        testModel = model;

        // Check that model bounds are as expected
        Log.Info("Expected bounds: center:<0,0.5,0> dimensions:<1,2,1> Got: {0}", model.Bounds);
        if (model.Bounds.center.x != 0 ||
            model.Bounds.center.y != 0.5f ||
            model.Bounds.center.z != 0 ||
            model.Bounds.dimensions.x != 1 ||
            model.Bounds.dimensions.y != 2 ||
            model.Bounds.dimensions.z != 1)
            return false;

        model.RemoveSubset(lastSubset);

        /// :CodeSample: Model.SubsetCount Model.GetMaterial Model.SetMaterial
        for (int i = 0; i < model.SubsetCount; i++)
        {
            Material mat = model.GetMaterial(i).Copy();
            mat[MatParamName.ColorTint] = Color.HSV(0, 1, 1);
            model.SetMaterial(i, mat);
        }
        /// :End:
        
        return true;
    }

    public void Initialize()
    {
        Tester.Test(TestModel);
    }
    public void Shutdown  (){}
    public void Update    ()
    {
        testModel.Draw(Matrix.Identity);
        Tests.Screenshot(600,600,"Tests/ModelSubsets.jpg", new Vec3(0,0.5f,-2), new Vec3(0, 0.5f, 0));
    }
}
