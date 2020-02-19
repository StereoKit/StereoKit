using StereoKit;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading.Tasks;

class DemoAnim<T>
{
    class TimeCompare : IComparer<(float time, T data)>
        { public int Compare((float time, T data) a, (float time, T data) b) => a.time.CompareTo(b.time); }

    TimeCompare _comparer = new TimeCompare();
    (float time,T data)[] _frames;
    Func<T,T,float,T>     _lerp;
    float                 _startTime;
    float                 _updated;
    T                     _curr;

    public T Current { get {
        float now = Time.Totalf;
        if (now == _updated)
            return _curr;

        float elapsed = now - _startTime;
        _updated = now;
        _curr = Sample(elapsed);

        return _curr;
    } }

    public bool Playing => (Time.Totalf - _startTime) <= _frames[_frames.Length-1].time;

    public DemoAnim(Func<T, T, float, T> lerp, params (float,T)[] frames)
    {
        _frames    = frames;
        _lerp      = lerp;
        _startTime = Time.Totalf;
    }

    public void Play()
    {
        _startTime = Time.Totalf;
    }

    T Sample(float time)
    {
        if (time <= _frames[0].time)
            return  _frames[0].data;
        if (time >= _frames[_frames.Length - 1].time)
            return  _frames[_frames.Length - 1].data;

        int item = Array.BinarySearch(_frames, (time,default(T)), _comparer);
        if (item > 0)
            return _frames[item].data;
        else
        {
            item = ~item;
            var   p1  = _frames[item - 1];
            var   p2  = _frames[item];
            float pct = (time - p1.time) / (p2.time - p1.time);
            return _lerp(p1.data, p2.data, pct);
        }
    }
}

class DebugToolWindow
{
    static List<(float time, Pose        pose)> recordingHead = new List<(float time, Pose        pose)>();
    static List<(float time, HandJoint[] pose)> recordingHand = new List<(float time, HandJoint[] pose)>();
    static bool recordHead   = false;
    static bool recordHand   = false;
    static Pose pose         = new Pose(0, 0.2f, -.5f, Quat.LookAt(new Vec3(0, 0.2f, -.5f), Vec3.Zero));
    static bool screenshots  = false;
    static int  screenshotId = 1;

    static DemoAnim<Pose>        headAnim = new DemoAnim<Pose>(Pose.Lerp, (0f, new Pose(new Vec3(0.1914565f, 0.3257995f, 0.1829229f), new Quat(-0.2241393f, -0.2395577f, -0.05429352f, 0.9430938f))), (0.355526f, new Pose(new Vec3(0.1951213f, 0.328903f, 0.1627441f), new Quat(-0.2338382f, -0.1637848f, -0.02224868f, 0.9581228f))), (0.4547253f, new Pose(new Vec3(0.1841034f, 0.3249042f, 0.1445852f), new Quat(-0.2377328f, -0.0929592f, 0.002946629f, 0.9668677f))), (0.5115223f, new Pose(new Vec3(0.171472f, 0.3065492f, 0.1305225f), new Quat(-0.2242583f, -0.07368638f, 0.01729225f, 0.9715861f))), (0.5443153f, new Pose(new Vec3(0.1613611f, 0.2875795f, 0.1222149f), new Quat(-0.2115933f, -0.06653329f, 0.01791838f, 0.9749259f))), (0.5775814f, new Pose(new Vec3(0.1505832f, 0.2631982f, 0.1147647f), new Quat(-0.2037545f, -0.06157619f, 0.02242593f, 0.9768263f))), (0.6107597f, new Pose(new Vec3(0.1381644f, 0.2361028f, 0.1103159f), new Quat(-0.2056883f, -0.05901966f, 0.02364002f, 0.9765501f))), (0.632946f, new Pose(new Vec3(0.1300654f, 0.2161919f, 0.1069458f), new Quat(-0.20443f, -0.05756797f, 0.02442121f, 0.9768817f))), (0.6555958f, new Pose(new Vec3(0.1218811f, 0.1946493f, 0.1030865f), new Quat(-0.2018817f, -0.05882844f, 0.02723324f, 0.9772621f))), (0.677536f, new Pose(new Vec3(0.1157516f, 0.1723713f, 0.09849694f), new Quat(-0.1958864f, -0.05855279f, 0.03496263f, 0.9782524f))), (0.6995316f, new Pose(new Vec3(0.1085414f, 0.1504662f, 0.09512299f), new Quat(-0.1901778f, -0.06079718f, 0.0372897f, 0.9791555f))), (0.7222271f, new Pose(new Vec3(0.1005952f, 0.1265764f, 0.0916122f), new Quat(-0.1857565f, -0.06463068f, 0.03455311f, 0.9798589f))), (0.7442322f, new Pose(new Vec3(0.0944031f, 0.1058932f, 0.09000421f), new Quat(-0.1821094f, -0.06970615f, 0.03242926f, 0.9802681f))), (0.7673626f, new Pose(new Vec3(0.09019547f, 0.08502308f, 0.08613905f), new Quat(-0.1685613f, -0.07320351f, 0.029854f, 0.9825156f))), (0.7887478f, new Pose(new Vec3(0.08532361f, 0.06391568f, 0.08250527f), new Quat(-0.1563897f, -0.07618076f, 0.02721744f, 0.9843769f))), (0.8218098f, new Pose(new Vec3(0.07799675f, 0.03521464f, 0.0809193f), new Quat(-0.1484175f, -0.08632713f, 0.01951507f, 0.9849563f))), (0.8669243f, new Pose(new Vec3(0.07449702f, 0.01215063f, 0.08222283f), new Quat(-0.1617943f, -0.09106453f, 0.02187518f, 0.9823703f))), (0.9110508f, new Pose(new Vec3(0.07458723f, -0.008113771f, 0.0754652f), new Quat(-0.1504796f, -0.1033066f, 0.02681769f, 0.9828349f))), (1.001028f, new Pose(new Vec3(0.08712305f, 0.01294851f, 0.08201814f), new Quat(-0.1616616f, -0.1179734f, 0.04097734f, 0.9789119f))), (1.033171f, new Pose(new Vec3(0.09467448f, 0.03808746f, 0.08594169f), new Quat(-0.1592139f, -0.1160819f, 0.03955282f, 0.9795976f))), (1.055285f, new Pose(new Vec3(0.09907428f, 0.05768698f, 0.08745639f), new Quat(-0.1524404f, -0.114254f, 0.02889907f, 0.9812608f))), (1.077074f, new Pose(new Vec3(0.1044894f, 0.08029418f, 0.08964168f), new Quat(-0.1510048f, -0.1144186f, 0.02425575f, 0.9815893f))), (1.100456f, new Pose(new Vec3(0.1094245f, 0.1065148f, 0.09214954f), new Quat(-0.1546448f, -0.1161268f, 0.01929696f, 0.9809318f))), (1.122089f, new Pose(new Vec3(0.1147043f, 0.1336032f, 0.09557317f), new Quat(-0.1600719f, -0.1179494f, 0.01567554f, 0.9799078f))), (1.144974f, new Pose(new Vec3(0.1193766f, 0.1599542f, 0.09714724f), new Quat(-0.161073f, -0.1195368f, 0.01285851f, 0.9795923f))), (1.166304f, new Pose(new Vec3(0.1238931f, 0.1867671f, 0.09802413f), new Quat(-0.164518f, -0.1188974f, 0.01158335f, 0.9791135f))), (1.18829f, new Pose(new Vec3(0.1300743f, 0.2132208f, 0.1001236f), new Quat(-0.1699284f, -0.1164234f, 0.0116271f, 0.9784859f))), (1.210852f, new Pose(new Vec3(0.1348352f, 0.2368987f, 0.1012218f), new Quat(-0.1783565f, -0.1136027f, 0.01148794f, 0.9773185f))), (1.246321f, new Pose(new Vec3(0.1391893f, 0.2564777f, 0.1027609f), new Quat(-0.1893898f, -0.1104968f, 0.008905901f, 0.9756242f))), (1.299856f, new Pose(new Vec3(0.1458896f, 0.2766979f, 0.1047388f), new Quat(-0.2015288f, -0.1091828f, 0.006220843f, 0.9733584f))), (1.310583f, new Pose(new Vec3(0.1590459f, 0.3100202f, 0.1080005f), new Quat(-0.2139706f, -0.1061873f, 0.003414538f, 0.9710454f))), (1.376709f, new Pose(new Vec3(0.1782563f, 0.3232526f, 0.1134057f), new Quat(-0.2225705f, -0.10899f, -0.01236079f, 0.9687263f))), (1.433495f, new Pose(new Vec3(0.1987221f, 0.3109389f, 0.1170774f), new Quat(-0.2219496f, -0.1145556f, -0.0179747f, 0.9681386f))), (1.477024f, new Pose(new Vec3(0.2182039f, 0.2944488f, 0.1206159f), new Quat(-0.2234124f, -0.1112119f, -0.01724984f, 0.9682052f))), (1.510145f, new Pose(new Vec3(0.2321605f, 0.273108f, 0.1228303f), new Quat(-0.2220339f, -0.1059825f, -0.02201792f, 0.9690118f))), (1.544067f, new Pose(new Vec3(0.2499947f, 0.2516218f, 0.1266512f), new Quat(-0.2255727f, -0.09766546f, -0.01598587f, 0.9691867f))), (1.566183f, new Pose(new Vec3(0.2615541f, 0.2349443f, 0.12774f), new Quat(-0.2213042f, -0.09250855f, -0.01345812f, 0.970714f))), (1.588099f, new Pose(new Vec3(0.2720206f, 0.2160992f, 0.12779f), new Quat(-0.20952f, -0.08997208f, -0.01477908f, 0.973544f))), (1.610682f, new Pose(new Vec3(0.2837929f, 0.1926532f, 0.1274144f), new Quat(-0.1928724f, -0.08570652f, -0.009500269f, 0.9774275f))), (1.633192f, new Pose(new Vec3(0.2955699f, 0.1724532f, 0.1292937f), new Quat(-0.1905081f, -0.0785557f, -0.002885593f, 0.9785333f))), (1.654739f, new Pose(new Vec3(0.305541f, 0.1524989f, 0.1316793f), new Quat(-0.1886067f, -0.07122602f, -0.002923859f, 0.979462f))), (1.677031f, new Pose(new Vec3(0.314874f, 0.1322908f, 0.1315839f), new Quat(-0.1784309f, -0.06578997f, -0.002472234f, 0.9817474f))), (1.711016f, new Pose(new Vec3(0.3258101f, 0.1062102f, 0.1348584f), new Quat(-0.1686634f, -0.06326717f, -0.004889883f, 0.983629f))), (1.743893f, new Pose(new Vec3(0.3364647f, 0.07852073f, 0.1340623f), new Quat(-0.1434395f, -0.05811973f, -0.00685063f, 0.9879273f))), (1.778324f, new Pose(new Vec3(0.3428913f, 0.05552439f, 0.1372361f), new Quat(-0.1354274f, -0.05252423f, -0.009920545f, 0.9893443f))), (1.810915f, new Pose(new Vec3(0.348583f, 0.03486582f, 0.1409953f), new Quat(-0.1300032f, -0.05171874f, -0.009578167f, 0.9901174f))), (1.85495f, new Pose(new Vec3(0.3510286f, 0.01338358f, 0.1420258f), new Quat(-0.1159455f, -0.05165787f, -0.01924526f, 0.9917246f))), (1.998804f, new Pose(new Vec3(0.3393222f, 0.0321097f, 0.1416358f), new Quat(-0.1088893f, -0.04406923f, -0.01915836f, 0.9928917f))), (2.03289f, new Pose(new Vec3(0.3312312f, 0.0559527f, 0.1377733f), new Quat(-0.1087753f, -0.04692283f, -0.01541439f, 0.9928387f))), (2.066179f, new Pose(new Vec3(0.3209557f, 0.08533747f, 0.1343096f), new Quat(-0.1153403f, -0.0476366f, -0.0107859f, 0.9921245f))), (2.088507f, new Pose(new Vec3(0.3135069f, 0.1062079f, 0.1317586f), new Quat(-0.1204726f, -0.0471329f, -0.005073559f, 0.9915842f))), (2.110525f, new Pose(new Vec3(0.3048471f, 0.1310803f, 0.1297936f), new Quat(-0.130384f, -0.04508919f, 0.001660329f, 0.9904364f))), (2.132381f, new Pose(new Vec3(0.2953553f, 0.1561729f, 0.1287273f), new Quat(-0.1431189f, -0.04425873f, 0.002626077f, 0.9887119f))), (2.154272f, new Pose(new Vec3(0.2846318f, 0.182828f, 0.1265375f), new Quat(-0.1535698f, -0.04808998f, 0.003572408f, 0.9869604f))), (2.178198f, new Pose(new Vec3(0.2745971f, 0.2083782f, 0.1236088f), new Quat(-0.1604307f, -0.05558069f, 0.005678043f, 0.9854646f))), (2.199215f, new Pose(new Vec3(0.2640784f, 0.2325628f, 0.1206467f), new Quat(-0.1692149f, -0.06337897f, 0.003154742f, 0.9835342f))), (2.22193f, new Pose(new Vec3(0.2520559f, 0.2568325f, 0.1171423f), new Quat(-0.179703f, -0.07491983f, -0.003657353f, 0.980857f))), (2.243835f, new Pose(new Vec3(0.241139f, 0.2769028f, 0.1140805f), new Quat(-0.1888072f, -0.08646848f, -0.002415723f, 0.9781969f))), (2.26622f, new Pose(new Vec3(0.2298432f, 0.2943158f, 0.1110424f), new Quat(-0.2000788f, -0.1003556f, -0.00104852f, 0.9746261f))), (2.28797f, new Pose(new Vec3(0.2166988f, 0.3120079f, 0.1065167f), new Quat(-0.2093188f, -0.1180281f, -0.007649587f, 0.9706681f))), (2.321581f, new Pose(new Vec3(0.1983479f, 0.327944f, 0.09893578f), new Quat(-0.2139649f, -0.1459688f, -0.01324211f, 0.9657829f))), (2.354502f, new Pose(new Vec3(0.1794701f, 0.3373234f, 0.09136558f), new Quat(-0.2188825f, -0.1763353f, -0.0207048f, 0.9594622f))), (2.399265f, new Pose(new Vec3(0.1566968f, 0.3360512f, 0.08272652f), new Quat(-0.2242082f, -0.2167887f, -0.02229376f, 0.9498612f))), (2.432257f, new Pose(new Vec3(0.138608f, 0.3281055f, 0.07437485f), new Quat(-0.2229175f, -0.2461943f, -0.03014647f, 0.9427552f))), (2.46587f, new Pose(new Vec3(0.1231344f, 0.3166706f, 0.06652384f), new Quat(-0.2232418f, -0.2705563f, -0.02991332f, 0.9359848f))), (2.498781f, new Pose(new Vec3(0.1090417f, 0.2974134f, 0.05685246f), new Quat(-0.2119131f, -0.2947804f, -0.03096888f, 0.9312562f))), (2.532478f, new Pose(new Vec3(0.09685286f, 0.2731085f, 0.04914221f), new Quat(-0.2021628f, -0.3156936f, -0.02841586f, 0.9266393f))), (2.554588f, new Pose(new Vec3(0.08829539f, 0.2551641f, 0.04420257f), new Quat(-0.1929042f, -0.3277875f, -0.02785191f, 0.9244283f))), (2.576843f, new Pose(new Vec3(0.08218027f, 0.2334609f, 0.04010531f), new Quat(-0.1797353f, -0.339547f, -0.02747413f, 0.9228479f))), (2.599455f, new Pose(new Vec3(0.07624562f, 0.2126211f, 0.03694845f), new Quat(-0.1690516f, -0.351681f, -0.02745168f, 0.9203197f))), (2.621849f, new Pose(new Vec3(0.07029328f, 0.1908721f, 0.03480389f), new Quat(-0.1591599f, -0.364313f, -0.02798318f, 0.9171484f))), (2.643599f, new Pose(new Vec3(0.06706518f, 0.1701311f, 0.03279181f), new Quat(-0.1483813f, -0.3758664f, -0.02500528f, 0.9143752f))), (2.67662f, new Pose(new Vec3(0.0618786f, 0.1427489f, 0.03203499f), new Quat(-0.1385235f, -0.3908329f, -0.02760774f, 0.9095596f))), (2.710606f, new Pose(new Vec3(0.05811398f, 0.1198651f, 0.03236936f), new Quat(-0.1296381f, -0.4008496f, -0.02714471f, 0.906519f))), (2.743765f, new Pose(new Vec3(0.05594352f, 0.09949272f, 0.03072697f), new Quat(-0.1119999f, -0.4086015f, -0.02691768f, 0.905415f))), (2.788795f, new Pose(new Vec3(0.05572342f, 0.07891964f, 0.02907514f), new Quat(-0.09557652f, -0.4147836f, -0.02125086f, 0.9046369f))), (2.954998f, new Pose(new Vec3(0.06329652f, 0.09998041f, 0.03018029f), new Quat(-0.100432f, -0.4027752f, -0.02438218f, 0.9094455f))), (2.987925f, new Pose(new Vec3(0.06704329f, 0.1230562f, 0.0332166f), new Quat(-0.1085575f, -0.4017455f, -0.03116423f, 0.90876f))), (3.02162f, new Pose(new Vec3(0.07114621f, 0.1490365f, 0.03674053f), new Quat(-0.1141912f, -0.4046409f, -0.03941366f, 0.9064617f))), (3.054815f, new Pose(new Vec3(0.07664807f, 0.1774579f, 0.04027779f), new Quat(-0.1165117f, -0.405455f, -0.04479388f, 0.9055522f))), (3.076736f, new Pose(new Vec3(0.07962295f, 0.1979589f, 0.04208566f), new Quat(-0.1191587f, -0.4058847f, -0.05369468f, 0.9045307f))), (3.098715f, new Pose(new Vec3(0.0829284f, 0.2179809f, 0.04453659f), new Quat(-0.1247261f, -0.406494f, -0.05933537f, 0.903153f))), (3.121796f, new Pose(new Vec3(0.08804367f, 0.2384599f, 0.04835221f), new Quat(-0.1318124f, -0.4060394f, -0.05952118f, 0.9023384f))), (3.154699f, new Pose(new Vec3(0.09548124f, 0.2668979f, 0.05315951f), new Quat(-0.1424887f, -0.4019839f, -0.06316017f, 0.9022842f))), (3.188238f, new Pose(new Vec3(0.1042578f, 0.2922443f, 0.05887536f), new Quat(-0.1558524f, -0.3955116f, -0.0623401f, 0.9029919f))), (3.221741f, new Pose(new Vec3(0.1142632f, 0.313045f, 0.06538795f), new Quat(-0.1716025f, -0.3856848f, -0.05654731f, 0.9047664f))), (3.25465f, new Pose(new Vec3(0.1237892f, 0.3305988f, 0.07338968f), new Quat(-0.1949344f, -0.3750962f, -0.05496205f, 0.9045897f))), (3.310022f, new Pose(new Vec3(0.1404564f, 0.3442206f, 0.08332066f), new Quat(-0.2096288f, -0.3569611f, -0.05327704f, 0.9087332f))), (3.377254f, new Pose(new Vec3(0.1604695f, 0.3439125f, 0.09292511f), new Quat(-0.2197163f, -0.3290672f, -0.05202066f, 0.9169151f))), (3.443933f, new Pose(new Vec3(0.1778544f, 0.3338479f, 0.1004301f), new Quat(-0.2334978f, -0.2998171f, -0.05463454f, 0.9233654f))), (3.543186f, new Pose(new Vec3(0.1974547f, 0.3263714f, 0.1044159f), new Quat(-0.2497671f, -0.2762135f, -0.06424015f, 0.9258487f))), (3.798946f, new Pose(new Vec3(0.2136394f, 0.3213064f, 0.1150421f), new Quat(-0.2500122f, -0.2394643f, -0.06813665f, 0.9356859f))), (3.876904f, new Pose(new Vec3(0.2135198f, 0.3208998f, 0.118064f), new Quat(-0.2500775f, -0.2391606f, -0.06901909f, 0.9356815f))));
    static DemoAnim<HandJoint[]> handAnim = null;

    public static void Step()
    {
        UI.WindowBegin("Helper", ref pose, new Vec2(20, 0)*Units.cm2m);
        if (UI.Button("Print Screenshot Pose")) HeadshotPose();
        if (UI.Button("Print Hand Pose")) HandshotPose();
        if (UI.Toggle("Record Head", ref recordHead)) ToggleRecordHead();
        if (headAnim != null) {
            UI.SameLine();
            if (UI.Button("Play1")) headAnim.Play();
        }
        if (UI.Toggle("Record Hand", ref recordHand)) ToggleRecordHand();
        if (handAnim != null) {
            UI.SameLine();
            if (UI.Button("Play2")) handAnim.Play();
        }
        UI.Toggle("Enable Screenshots", ref screenshots);
        UI.WindowEnd();

        if (screenshots) TakeScreenshots();
        if (recordHead)  RecordHead();
        if (recordHand)  RecordHand();
        if (headAnim != null && headAnim.Playing)
            Renderer.SetView(headAnim.Current.ToMatrix());
        if (handAnim != null && handAnim.Playing)
            Input.HandOverride(Handed.Right, handAnim.Current);
        else
            Input.HandClearOverride(Handed.Right);
        
        ScreenshotPreview();
    }

    static void TakeScreenshots()
    {
        // Take a screenshot on the first frame both hands are gripped
        bool valid =
            Input.Hand(Handed.Left ).IsTracked &&
            Input.Hand(Handed.Right).IsTracked;
        BtnState right = Input.Hand(Handed.Right).grip;
        BtnState left  = Input.Hand(Handed.Left ).grip;
        if (valid && left.IsActive() && right.IsActive() && (left.IsJustActive() || right.IsJustActive()))
        {
            Renderer.Screenshot(Input.Head.position, Input.Head.Forward, 1920 * 2, 1080 * 2, "Screenshot" + screenshotId + ".jpg");
            screenshotId += 1;
        }
    }

    static void RecordHead()
    {
        Pose prev = recordingHead[recordingHead.Count - 1].pose;
        Quat diff = Quat.Difference(Input.Head.orientation, prev.orientation);
        if (Vec3.DistanceSq(Input.Head.position, prev.position) > (2 * Units.cm2m * 2 * Units.cm2m) ||
            (diff.w) * (diff.w) < (0.8f * 0.8f))
            recordingHead.Add((Time.Totalf, Input.Head));
        
    }
    static void ToggleRecordHead()
    {
        recordingHead.Add((Time.Totalf, Input.Head));
        if (!recordHead)
        {
            float  rootTime = recordingHead[0].time;
            string result = "DemoAnim<Pose> anim = new DemoAnim<Pose>(Pose.Lerp,";
            for (int i = 0; i < recordingHead.Count; i++)
            {
                recordingHead[i] = (recordingHead[i].time - rootTime, recordingHead[i].pose);
                var f = recordingHead[i];
                result += $"({f.time}f, new Pose(new Vec3({f.pose.position.x:0.000}f,{f.pose.position.y:0.000}f,{f.pose.position.z:0.000}f), new Quat({f.pose.orientation.x:0.000}f,{f.pose.orientation.y:0.000}f,{f.pose.orientation.z:0.000}f,{f.pose.orientation.w:0.000}f)))";
                if (i < recordingHead.Count-1)
                    result += ",";
            }
            result += ");";
            Log.Info(result);

            headAnim = new DemoAnim<Pose>(Pose.Lerp, recordingHead.ToArray());
            recordingHead.Clear();
        }
    }

    static void RecordHand()
    {
        if (Time.Totalf - recordingHand[recordingHand.Count-1].time > 0.2f) {
            Hand        h      = Input.Hand(Handed.Right);
            HandJoint[] joints = new HandJoint[27];
            Array.Copy(h.fingers,0,joints,0,25);
            joints[25] = new HandJoint(h.palm.position,  h.palm.orientation,  0);
            joints[26] = new HandJoint(h.wrist.position, h.wrist.orientation, 0);
            recordingHand.Add((Time.Totalf, joints));
        }
    }
    static void ToggleRecordHand()
    {
        Hand        h      = Input.Hand(Handed.Right);
        HandJoint[] joints = new HandJoint[27];
        Array.Copy(h.fingers,0,joints,0,25);
        joints[25] = new HandJoint(h.palm.position,  h.palm.orientation,  0);
        joints[26] = new HandJoint(h.wrist.position, h.wrist.orientation, 0);
        recordingHand.Add((Time.Totalf, joints));
        if (!recordHand)
        {
            float  rootTime = recordingHand[0].time;
            StringBuilder result = new StringBuilder();
            result.Append("DemoAnim<HandJoint[]> anim = new DemoAnim<HandJoint[]>(JointsLerp,");
            for (int i = 0; i < recordingHand.Count; i++)
            {
                recordingHand[i] = (recordingHand[i].time - rootTime, recordingHand[i].pose);
                var f = recordingHand[i];
                result.Append( $"({f.time}f,new HandJoint[]{{" );
                for (int j = 0; j < f.pose.Length; j++)
                {
                    result.Append($"new HandJoint(new Vec3({f.pose[j].position.x:0.000}f,{f.pose[j].position.y:0.000}f,{f.pose[j].position.z:0.000}f), new Quat({f.pose[j].orientation.x:0.000}f,{f.pose[j].orientation.y:0.000}f,{f.pose[j].orientation.z:0.000}f,{f.pose[j].orientation.w:0.000}f), {f.pose[j].radius:0.000}f)");
                    if (j < f.pose.Length - 1)
                        result.Append(",");
                }
                result.Append("})");
                if (i < recordingHand.Count - 1)
                    result.Append(",");
            }
            result.Append(");");
            Log.Info(result.ToString());

            handAnim = new DemoAnim<HandJoint[]>(JointsLerp, recordingHand.ToArray());
            recordingHand.Clear();
        }
    }

    static void HeadshotPose()
    {
        Vec3 pos = Input.Head.position + Input.Head.Forward * 10 * Units.cm2m;
        Vec3 fwd = pos + Input.Head.Forward;
        Log.Info($"Demos.Screenshot(600, 600, \"image.jpg\", new Vec3({pos.x:0.000}f, {pos.y:0.000}f, {pos.z:0.000}f), new Vec3({fwd.x:0.000}f, {fwd.y:0.000}f, {fwd.z:0.000}f));");
        PreviewScreenshot(pos, fwd);
    }
    static void HandshotPose()
    {
        Hand h = Input.Hand(Handed.Right);
        HandJoint[] joints = new HandJoint[27];
        Array.Copy(h.fingers, 0, joints, 0, 25);
        joints[25] = new HandJoint(h.palm.position, h.palm.orientation, 0);
        joints[26] = new HandJoint(h.wrist.position, h.wrist.orientation, 0);
        recordingHand.Add((Time.Totalf, joints));

        string result = ($"Demos.Hand(new HandJoint[]{{");
        for (int j = 0; j < joints.Length; j++)
        {
            result += $"new HandJoint(new Vec3({joints[j].position.x:0.000}f,{joints[j].position.y:0.000}f,{joints[j].position.z:0.000}f), new Quat({joints[j].orientation.x:0.000}f,{joints[j].orientation.y:0.000}f,{joints[j].orientation.z:0.000}f,{joints[j].orientation.w:0.000}f), {joints[j].radius:0.000}f)";
            if (j < joints.Length - 1)
                result += ",";
        }
        result+="});";
        Log.Info(result);
    }

    static HandJoint[] JointsLerp(HandJoint[] a, HandJoint[] b, float t)
    {
        HandJoint[] result = new HandJoint[a.Length];
        for (int i = 0; i < a.Length; i++) { 
            result[i].position    = Vec3  .Lerp (a[i].position, b[i].position, t);
            result[i].orientation = Quat  .Slerp(a[i].orientation, b[i].orientation, t);
            result[i].radius      = SKMath.Lerp (a[i].radius, b[i].radius, t);
        }
        return result;
    }

    static Sprite screenshot;
    static Pose   screenshotPose;
    static bool   screenshotVisible;
    static void PreviewScreenshot(Vec3 from, Vec3 at)
    {
        string path = Path.GetTempFileName();
        path = Path.ChangeExtension(path, "jpg");
        Renderer.Screenshot(from, at, 600, 600, path);
        Task.Run(()=> { 
            Task.Delay(1000).Wait();
            screenshot = Sprite.FromTex(Tex.FromFile(path));
        });

        if (!screenshotVisible) { 
            screenshotPose.position    = Input.Head.position + Input.Head.Forward * 0.3f;
            screenshotPose.orientation = Quat.LookAt(screenshotPose.position, Input.Head.position);
        }
        screenshotVisible = true;
    }
    static void ScreenshotPreview()
    {
        if (!screenshotVisible)
            return;

        UI.WindowBegin("Screenshot", ref screenshotPose, new Vec2(20,0)*Units.cm2m);
        if (screenshot != null)
            UI.Image(screenshot, new Vec2(18,0) * Units.cm2m);
        if (UI.Button("Close"))
        { 
            screenshotVisible = false;
            screenshot = null;
        }
        UI.WindowEnd();
    }
}