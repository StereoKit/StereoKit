
namespace StereoKit
{
	/// <summary>A collection of unit conversion constants! Multiply things by these 
	/// to convert them into different units.</summary>
	public static class Units
	{
		/// <summary>Degrees to radians, multiply degree values by this, and you get 
		/// radians! Like so: `180 * Units.deg2rad == 3.1415926536`, which is pi,
		/// or 1 radian.</summary>
		public const float deg2rad = 0.01745329252f;
		/// <summary>Radians to degrees, multiply radian values by this, and you get 
		/// degrees! Like so: `PI * Units.rad2deg == 180`</summary>
		public const float rad2deg = 57.295779513f;
		/// <summary> Converts centimeters to meters. There are 100cm in 1m. In StereoKit 
		/// 1 unit is also 1 meter, so `25 * Units.cm2m == 0.25`, 25 centimeters is .25 
		/// meters/units. </summary>
		public const float cm2m = 0.01f;
		/// <summary> Converts millimeters to meters. There are 1000mm in 1m. In StereoKit 
		/// 1 unit is 1 meter, so `250 * Units.mm2m == 0.25`, 250 millimeters is .25 
		/// meters/units. </summary>
		public const float mm2m = 0.001f;
		/// <summary>Converts meters to centimeters. There are 100cm in 1m, so this just
		/// multiplies by 100.</summary>
		public const float m2cm = 100f;
		/// <summary>Converts meters to millimeters. There are 1000mm in 1m, so this just
		/// multiplies by 1000.</summary>
		public const float m2mm = 1000;
	}

	/// <summary>A shorthand class with unit multipliers. Helps make code a 
	/// little more terse on occasions!.</summary>
	public static class U
	{
		/// <summary> Converts centimeters to meters. There are 100cm in 1m. In StereoKit 
		/// 1 unit is also 1 meter, so `25 * U.cm == 0.25`, 25 centimeters is .25 
		/// meters/units. </summary>
		public const float cm = 0.01f;
		/// <summary> Converts millimeters to meters. There are 1000mm in 1m. In StereoKit 
		/// 1 unit is 1 meter, so `250 * Units.mm2m == 0.25`, 250 millimeters is .25 
		/// meters/units. </summary>
		public const float mm = 0.001f;
		/// <summary> StereoKit's default unit is meters, but sometimes it's 
		/// nice to be explicit!</summary>
		public const float m = 1.0f;
		/// <summary>Converts meters to kilometers. There are 1000m in 1km, 
		/// so this just multiplies by 1000.</summary>
		public const float km = 1000f;
	}
}
