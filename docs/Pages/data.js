var documents =
	{
	name : 'Pages',
	pages : [],
	folders : [
		{
		name : 'Guides',
		pages : ['Getting Started','User Interface','Using Hands'],
		},
		{
		name : 'Reference',
		pages : [],
		folders : [
			{
			name : 'Bounds',
			pages : ['center','dimensions','Bounds','Contains','FromCorner','FromCorners','Intersect'],
			},
			{
			name : 'BtnState',
			pages : ['Active','Any','Changed','Inactive','JustActive','JustInactive'],
			},
			{
			name : 'BtnStateExtensions',
			pages : ['IsActive','IsJustActive','IsJustInactive'],
			},
			{
			name : 'Color',
			pages : ['a','b','Black','BlackTransparent','g','r','White','Color','HSV','LAB','ToHSV','ToLAB'],
			},
			{
			name : 'Color32',
			pages : ['a','b','g','r','White'],
			},
			{
			name : 'Cull',
			pages : ['Back','Front','None'],
			},
			{
			name : 'DefaultIds',
			pages : ['cubemap','font','material','materialEquirect','materialFont','quad','shader','shaderEquirect','shaderFont','shaderPbr','shaderUnlit','tex','texBlack','texFlat','texGray','texRough'],
			},
			{
			name : 'Display',
			pages : ['Additive','Opaque','Passthrough'],
			},
			{
			name : 'FingerId',
			pages : ['Index','Little','Middle','Ring','Thumb'],
			},
			{
			name : 'Font',
			pages : ['Find','FromFile'],
			},
			{
			name : 'Gradient',
			pages : ['Count','Gradient','Add','Get','Get32','Remove','Set'],
			},
			{
			name : 'GradientKey',
			pages : ['color','position'],
			},
			{
			name : 'Hand',
			pages : ['grip','handed','IsGripped','IsJustGripped','IsJustPinched','IsJustTracked','IsJustUngripped','IsJustUnpinched','IsJustUntracked','IsPinched','IsTracked','Material','palm','pinch','Solid','tracked','Visible','wrist'],
			},
			{
			name : 'Handed',
			pages : ['Left','Max','Right'],
			},
			{
			name : 'HandJoint',
			pages : ['orientation','Pose','position','radius'],
			},
			{
			name : 'Hierarchy',
			pages : ['Enabled','Pop','Push','ToLocal','ToLocalDirection','ToWorld','ToWorldDirection'],
			},
			{
			name : 'InputSource',
			pages : ['Any','CanPress','Gaze','GazeCursor','GazeEyes','GazeHead','Hand','HandLeft','HandRight'],
			},
			{
			name : 'JointId',
			pages : ['Distal','Intermediate','Metacarpal','Proximal','Tip'],
			},
			{
			name : 'Key',
			pages : ['A','Add','Alt','B','Backspace','C','Ctrl','D','Decimal','Del','Divide','Down','E','End','Esc','F','F1','F10','F11','F12','F2','F3','F4','F5','F6','F7','F8','F9','G','H','Home','I','Insert','J','K','L','LCmd','Left','M','MAX','MouseCenter','MouseLeft','MouseRight','Multiply','N','N0','N1','N2','N3','N4','N5','N6','N7','N8','N9','Num0','Num1','Num2','Num3','Num4','Num5','Num6','Num7','Num8','Num9','O','P','Printscreen','Q','R','RCmd','Return','Right','S','Shift','Space','Subtract','T','Tab','U','Up','V','W','X','Y','Z'],
			},
			{
			name : 'Lines',
			pages : ['Add','AddAxis'],
			},
			{
			name : 'Log',
			pages : ['Filter','Err','Info','Warn','Write'],
			},
			{
			name : 'LogLevel',
			pages : ['Diagnostic','Error','Info','Warning'],
			},
			{
			name : 'Material',
			pages : ['FaceCull','ParamCount','QueueOffset','Shader','Transparency','Material','Copy','Find','SetColor','SetFloat','SetMatrix','SetTexture','SetVector'],
			},
			{
			name : 'MaterialParam',
			pages : ['Color128','Float','Matrix','Texture','Vector'],
			},
			{
			name : 'Matrix',
			pages : ['Identity','Inverse','T','TransformDirection','TransformPoint','TRS','TS'],
			},
			{
			name : 'Mesh',
			pages : ['Bounds','Mesh','Draw','Find','GenerateCube','GenerateCylinder','GeneratePlane','GenerateRoundedCube','GenerateSphere'],
			},
			{
			name : 'Model',
			pages : ['Bounds','SubsetCount','Model','Draw','Find','FromFile','FromMesh','GetMaterial'],
			},
			{
			name : 'Mouse',
			pages : ['available','pos','posChange','scroll','scrollChange'],
			},
			{
			name : 'Plane',
			pages : ['d','normal','Plane','Closest','Intersect'],
			},
			{
			name : 'Pointer',
			pages : ['orientation','Pose','ray','source','state','tracked'],
			},
			{
			name : 'Pose',
			pages : ['Forward','orientation','position','Right','Up','Pose','ToMatrix'],
			},
			{
			name : 'Quat',
			pages : ['Identity','w','x','y','z','Quat','Difference','FromAngles','Inverse','LookAt','LookDir','Normalize','Slerp'],
			},
			{
			name : 'Ray',
			pages : ['direction','position','Ray','Intersect'],
			},
			{
			name : 'Renderer',
			pages : ['EnableSky','SkyLight','SkyTex','Add','Blit','Screenshot','SetClip','SetView'],
			},
			{
			name : 'Runtime',
			pages : ['Flatscreen','MixedReality'],
			},
			{
			name : 'Settings',
			pages : ['assetsFolder','flatscreenHeight','flatscreenPosX','flatscreenPosY','flatscreenWidth'],
			},
			{
			name : 'Shader',
			pages : ['Name','Find','FromFile','FromHLSL'],
			},
			{
			name : 'SHLight',
			pages : ['color','directionTo'],
			},
			{
			name : 'SKMath',
			pages : ['Pi','Tau','Cos','Sin','Sqrt'],
			},
			{
			name : 'Solid',
			pages : ['Enabled','Solid','AddBox','AddCapsule','AddSphere','GetPose','Move','SetAngularVelocity','SetEnabled','SetType','SetVelocity','Teleport'],
			},
			{
			name : 'SolidType',
			pages : ['Immovable','Normal','Unaffected'],
			},
			{
			name : 'Sphere',
			pages : ['center','Diameter','radius','Sphere','Contains','Intersect'],
			},
			{
			name : 'SphericalHarmonics',
			pages : ['FromLights','Sample'],
			},
			{
			name : 'Sprite',
			pages : ['Aspect','Height','NormalizedDimensions','Width','Draw','FromFile','FromTex'],
			},
			{
			name : 'SpriteType',
			pages : ['Atlased','Single'],
			},
			{
			name : 'StereoKitApp',
			pages : ['ActiveRuntime','IsInitialized','settings','System','VersionId','VersionName','Initialize','Quit','Shutdown','Step'],
			},
			{
			name : 'SystemInfo',
			pages : ['displayType'],
			},
			{
			name : 'Tex',
			pages : ['AddressMode','Anisoptropy','Height','Id','SampleMode','Width','Tex','Find','FromCubemapEquirectangular','FromCubemapFile','FromFile','GenCubemap'],
			},
			{
			name : 'TexAddress',
			pages : ['Clamp','Mirror','Wrap'],
			},
			{
			name : 'TexFormat',
			pages : ['Depth16','Depth32','DepthStencil','Rgba128','Rgba32','Rgba32Linear','Rgba64'],
			},
			{
			name : 'TexSample',
			pages : ['Anisotropic','Linear','Point'],
			},
			{
			name : 'Text',
			pages : ['Add','MakeStyle','Size'],
			},
			{
			name : 'TextAlign',
			pages : ['Center','XCenter','XLeft','XRight','YBottom','YCenter','YTop'],
			},
			{
			name : 'TextStyle',
			pages : [],
			},
			{
			name : 'TexType',
			pages : ['Cubemap','Depth','Dynamic','Image','ImageNomips','Mips','Rendertarget'],
			},
			{
			name : 'Time',
			pages : ['Elapsed','Elapsedf','ElapsedUnscaled','ElapsedUnscaledf','Scale','Total','Totalf','TotalUnscaled','TotalUnscaledf'],
			},
			{
			name : 'Transparency',
			pages : ['Blend','Clip','None'],
			},
			{
			name : 'UI',
			pages : ['ColorScheme','LineHeight','Settings','AffordanceBegin','AffordanceEnd','Button','ButtonRound','HSlider','Image','Label','LayoutArea','NextLine','PopId','PushId','Radio','ReserveBox','SameLine','Space','Toggle','WindowBegin','WindowEnd'],
			},
			{
			name : 'UISettings',
			pages : ['backplateBorder','backplateDepth','depth','gutter','padding'],
			},
			{
			name : 'Units',
			pages : ['cm2m','deg2rad','m2cm','m2mm','mm2m','rad2deg'],
			},
			{
			name : 'Vec2',
			pages : ['Magnitude','MagnitudeSq','One','x','y','Zero','Vec2','Normalize','Normalized'],
			},
			{
			name : 'Vec3',
			pages : ['Forward','Magnitude','MagnitudeSq','One','Right','Up','x','y','z','Zero','Vec3','AngleXY','AngleXZ','Cross','Distance','DistanceSq','Dot','Normalize','Normalized','PerpendicularRight'],
			},
			{
			name : 'Vec4',
			pages : ['w','x','y','z','Vec4'],
			},
		]
		},
	]
	};