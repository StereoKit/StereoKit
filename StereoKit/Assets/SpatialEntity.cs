using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>A SpatialEntity is something the device has discovered or is
	/// tracking in the user's physical environment: a wall, a table, a QR
	/// code, or an anchor the app has placed. This is StereoKit's surface
	/// for OpenXR's spatial entity extensions.
	///
	/// Entities are composed of components: chunks of data like a bounding
	/// rectangle, a semantic label, or a mesh. Which components an entity
	/// has depends on the capability that discovered it and what the device
	/// supports, so data is accessed through TryGet methods, and entities
	/// can be filtered by the components you need via `With`.
	///
	/// Enable the capabilities you're interested in with `Enable`, and
	/// StereoKit will keep an up-to-date list of entities that you can poll
	/// each frame. Component data reflects the entity's last known state:
	/// check `Tracked` to know if it's currently live.
	///
	/// SpatialEntity is a lightweight identifier, not a reference: the
	/// device owns these entities and controls their lifetimes. Identifiers
	/// are never reused within a session, so a stale one simply stops
	/// resolving once its entity is gone: `Valid` becomes false, and
	/// accessors return no data.</summary>
	public readonly struct SpatialEntity : IEquatable<SpatialEntity>
	{
		internal readonly uint _id;

		internal SpatialEntity(uint id) => _id = id;

		/// <summary>Does this identifier currently resolve to an entity?
		/// This becomes false once the entity permanently leaves the entity
		/// list, and a `default` SpatialEntity is never valid.</summary>
		public bool Valid => _id != 0 && NativeAPI.spatial_entity_is_valid(_id);

		/// <summary>The raw identifier value, unique within the current
		/// session and never reused. 0 is never a valid entity, and this
		/// value works well as a dictionary key.</summary>
		public uint Id => _id;

		/// <summary>Is the system actively tracking this entity? Component
		/// data is always the entity's last known state, so when this is
		/// inactive, that data may be stale. Entities that are permanently
		/// lost leave the entity list entirely.</summary>
		public BtnState Tracked => NativeAPI.spatial_entity_get_tracked(_id);
		/// <summary>The set of components this entity has valid data for.
		/// Each component has a matching TryGet accessor.</summary>
		public SpatialComponent Components => NativeAPI.spatial_entity_get_components(_id);
		/// <summary>Components whose data meaningfully changed this frame!
		/// Continuously updating poses are only flagged when they first
		/// arrive, while mesh/polygon/marker data is flagged whenever the
		/// system provides new data. Handy for skipping expensive work like
		/// mesh extraction when nothing changed.</summary>
		public SpatialComponent Changed => NativeAPI.spatial_entity_get_changed(_id);
		/// <summary>The entity this entity is attached to. This is an
		/// invalid entity if there's no parent, so check `Valid`.</summary>
		public SpatialEntity Parent => new SpatialEntity(NativeAPI.spatial_entity_get_parent(_id));

		/// <summary>The pose of this entity's anchor: a point the system
		/// actively keeps aligned with the physical world as tracking
		/// improves or drifts.</summary>
		/// <param name="pose">The anchor's pose in world space.</param>
		/// <returns>False if this entity has no anchor component.</returns>
		public bool TryGetAnchor(out Pose pose)
			=> NativeAPI.spatial_entity_get_anchor(_id, out pose);

		/// <summary>The 2D rectangular bounds of this entity, such as the
		/// extents of a detected plane, or the shape of a marker. The pose
		/// faces out of the surface: its Forward is the surface normal, the
		/// same way quads and text face in StereoKit. A floor's pose faces
		/// up, and a wall's pose faces into the room.</summary>
		/// <param name="center">The pose at the center of the rectangle.</param>
		/// <param name="size">The rectangle's total size in meters, along
		/// the center pose's X and Y axes.</param>
		/// <returns>False if this entity has no bounds2d component.</returns>
		public bool TryGetBounds2D(out Pose center, out Vec2 size)
			=> NativeAPI.spatial_entity_get_bounds2d(_id, out center, out size);

		/// <summary>The oriented 3D bounding volume of this entity.</summary>
		/// <param name="center">The pose at the center of the volume.</param>
		/// <param name="size">The volume's total size in meters, along the
		/// center pose's axes.</param>
		/// <returns>False if this entity has no bounds3d component.</returns>
		public bool TryGetBounds3D(out Pose center, out Vec3 size)
			=> NativeAPI.spatial_entity_get_bounds3d(_id, out center, out size);

		/// <summary>Plane information for this entity: its general
		/// orientation, and a semantic category like floor or table. An
		/// entity may have either or both, absent values come back as None.
		/// </summary>
		/// <param name="alignment">The plane's general orientation.</param>
		/// <param name="label">The plane's semantic category.</param>
		/// <returns>False if this entity has neither plane component.</returns>
		public bool TryGetPlane(out PlaneAlign alignment, out PlaneLabel label)
			=> NativeAPI.spatial_entity_get_plane(_id, out alignment, out label);

		/// <summary>The entity's 3D mesh! Mesh vertices are relative to the
		/// origin pose, which the system keeps aligned with the physical
		/// world, so draw the mesh at origin each frame. Filling the Mesh is
		/// the expensive path: pass a null mesh to fetch just the current
		/// origin, and refill only when `Changed` flags the Mesh component.
		/// </summary>
		/// <param name="mesh">A valid Mesh to fill with the entity's
		/// geometry, or null to only retrieve the origin pose.</param>
		/// <param name="origin">Pose the mesh's vertices are relative to.</param>
		/// <returns>False if this entity has no mesh component.</returns>
		public bool TryGetMesh(Mesh mesh, out Pose origin)
			=> NativeAPI.spatial_entity_get_mesh(_id, mesh?._inst ?? IntPtr.Zero, out origin);

		/// <summary>The entity's 2D surface mesh, on the XY plane of the
		/// origin pose. Works just like `TryGetMesh`: pass null to fetch
		/// only the origin, and refill when `Changed` flags the Mesh2d
		/// component.</summary>
		/// <param name="mesh">A valid Mesh to fill with the entity's
		/// geometry, or null to only retrieve the origin pose.</param>
		/// <param name="origin">Pose the mesh's vertices are relative to.</param>
		/// <returns>False if this entity has no mesh2d component.</returns>
		public bool TryGetMesh2D(Mesh mesh, out Pose origin)
			=> NativeAPI.spatial_entity_get_mesh2d(_id, mesh?._inst ?? IntPtr.Zero, out origin);

		/// <summary>The boundary polygon outlining the entity's surface, on
		/// the XY plane of the origin pose.</summary>
		/// <param name="origin">Pose the polygon's points are relative to.</param>
		/// <param name="polygon">A copy of the boundary points, in meters
		/// on the origin's XY plane.</param>
		/// <returns>False if this entity has no polygon component.</returns>
		public bool TryGetPolygon(out Pose origin, out Vec2[] polygon)
		{
			polygon = null;
			if (!NativeAPI.spatial_entity_get_polygon(_id, out origin, out IntPtr verts, out int count))
				return false;

			polygon = new Vec2[count];
			if (count > 0)
			{
				float[] data = new float[count * 2];
				Marshal.Copy(verts, data, 0, data.Length);
				for (int i = 0; i < count; i++)
					polygon[i] = new Vec2(data[i * 2], data[i * 2 + 1]);
			}
			return true;
		}

		/// <summary>Marker information, for entities discovered by a marker
		/// tracking capability like QR codes or ArUco markers. The marker's
		/// pose and physical size come from `TryGetBounds2D`.</summary>
		/// <param name="type">What kind of marker this is.</param>
		/// <param name="markerId">The marker's numeric id, for marker
		/// dictionaries like ArUco and AprilTag. 0 for QR codes.</param>
		/// <returns>False if this entity has no marker component.</returns>
		public bool TryGetMarker(out MarkerType type, out uint markerId)
			=> NativeAPI.spatial_entity_get_marker(_id, out type, out markerId);

		/// <summary>The marker's decoded string data, for QR family markers
		/// that contain text. Null if unavailable.</summary>
		public string MarkerText { get {
			IntPtr text = NativeAPI.spatial_entity_get_marker_text(_id);
			return text == IntPtr.Zero ? null : NativeHelper.FromUtf8(text);
		} }

		/// <summary>The marker's raw decoded bytes, for markers with binary
		/// data. Null if unavailable.</summary>
		public byte[] MarkerData { get {
			IntPtr data = NativeAPI.spatial_entity_get_marker_data(_id, out int size);
			if (data == IntPtr.Zero) return null;
			byte[] result = new byte[size];
			Marshal.Copy(data, result, 0, size);
			return result;
		} }

		/// <summary>Removes an app-created entity like an anchor from the
		/// system entirely: it's unpersisted if persisted, the system stops
		/// tracking it, and it leaves the entity list at the end of the
		/// frame. This SpatialEntity stops resolving once that happens.
		/// </summary>
		/// <returns>False if this entity can't be destroyed! Only
		/// app-created entities like anchors can be, entities the system
		/// discovered on its own, like planes, cannot.</returns>
		public bool Destroy() => NativeAPI.spatial_entity_destroy(_id);

		/// <summary>Can entities be written to persistent storage on this
		/// system? When false, `Persist` will do nothing. This can take a
		/// moment to become true while the system starts up, so it works
		/// well for graying out persistence UI.</summary>
		public static bool PersistenceAvailable => NativeAPI.spatial_persistence_available();

		/// <summary>A durable identifier for this entity that stays the same
		/// across sessions and device reboots! This is `Guid.Empty` unless
		/// the entity is persisted: either by the system itself, or by a
		/// call to `Persist`. Store this id to recognize the same physical
		/// entity in a later session.</summary>
		public Guid PersistId { get {
			byte[] uuid = new byte[16];
			return NativeAPI.spatial_entity_get_persist_id(_id, uuid)
				? new Guid(uuid)
				: Guid.Empty;
		} }

		/// <summary>Ask the system to persist this entity, giving it a
		/// durable identity that survives across sessions! This is
		/// asynchronous: on success, `PersistId` becomes valid a few frames
		/// later, and `Changed` flags the Persistence component. Requires a
		/// system with persistence support, and currently only anchor
		/// entities are persistable.</summary>
		public void Persist() => NativeAPI.spatial_entity_persist(_id);

		/// <summary>Remove this entity from persistent storage. Its
		/// `PersistId` becomes invalid once the asynchronous operation
		/// completes.</summary>
		public void Unpersist() => NativeAPI.spatial_entity_unpersist(_id);

		/// <summary>The spatial capabilities the current device supports!
		/// This is None until an XR session with spatial entity support has
		/// initialized.</summary>
		public static SpatialCapability Capabilities => NativeAPI.spatial_capabilities();
		/// <summary>The capabilities that have been requested via `Enable`.
		/// </summary>
		public static SpatialCapability Enabled => NativeAPI.spatial_get_enabled();
		/// <summary>The capabilities that are fully warmed up and actively
		/// providing entities. A subset of `Enabled`: capabilities take a
		/// little time to start after being enabled.</summary>
		public static SpatialCapability Active => NativeAPI.spatial_get_active();

		/// <summary>The components the device can provide on entities
		/// discovered by the given capability.</summary>
		/// <param name="capability">A single capability to look up.</param>
		/// <returns>All components the device supports for that capability.
		/// </returns>
		public static SpatialComponent ComponentsFor(SpatialCapability capability)
			=> NativeAPI.spatial_capability_components(capability);

		/// <summary>Request tracking for these capabilities, additively!
		/// This takes effect asynchronously: entities will appear in the
		/// entity list as the system warms up and discovers them, and the
		/// system may ask the user for permission first. Enabling or
		/// disabling one capability never disturbs entities belonging to
		/// another.</summary>
		/// <param name="capabilities">One or more capabilities to enable.
		/// Unsupported capabilities are ignored.</param>
		public static void Enable(SpatialCapability capabilities)
			=> NativeAPI.spatial_enable(capabilities);

		/// <summary>Enable marker tracking capabilities with additional
		/// configuration: the marker dictionary to detect, the physical
		/// size of your markers, and whether they stay put. Accurate values
		/// improve detection and pose quality, though unsupported options
		/// are quietly ignored, so treat these as hints. Calling this again
		/// with a different config restarts that capability's tracking with
		/// the new settings.</summary>
		/// <param name="capabilities">One or more marker tracking
		/// capabilities to enable with this config. Non-marker capabilities
		/// are ignored.</param>
		/// <param name="config">Configuration these markers should be
		/// tracked with. `default` is a valid baseline config.</param>
		public static void Enable(SpatialCapability capabilities, SpatialMarkerConfig config)
			=> NativeAPI.spatial_enable_marker(capabilities, config);

		/// <summary>Stop tracking these capabilities. Their entities leave
		/// the entity list, and any SpatialEntity identifiers you still
		/// hold stop resolving.</summary>
		/// <param name="capabilities">One or more capabilities to disable.</param>
		public static void Disable(SpatialCapability capabilities)
			=> NativeAPI.spatial_disable(capabilities);

		/// <summary>The number of entities that have data for all the given
		/// components.</summary>
		/// <param name="withComponents">Components to filter by, or None to
		/// count every entity.</param>
		/// <returns>The number of matching entities.</returns>
		public static int Count(SpatialComponent withComponents = SpatialComponent.None)
			=> NativeAPI.spatial_entity_get_count(withComponents);

		/// <summary>An enumeration of every spatial entity StereoKit
		/// currently knows about. This list is maintained for you: entities
		/// appear as the system discovers them, and leave when the system
		/// permanently stops tracking them.</summary>
		public static SpatialEntityCollection All => new SpatialEntityCollection(SpatialComponent.None, false);

		/// <summary>An enumeration of the spatial entities that have data
		/// for all the given components.</summary>
		/// <param name="components">Components each entity must have.</param>
		/// <returns>An enumeration of matching entities.</returns>
		public static SpatialEntityCollection With(SpatialComponent components)
			=> new SpatialEntityCollection(components, false);

		/// <summary>An enumeration of the spatial entities that appeared
		/// for the first time this frame.</summary>
		public static SpatialEntityCollection New => new SpatialEntityCollection(SpatialComponent.None, true);

		/// <summary>Create a spatial anchor entity at the given pose: a
		/// point the system will keep aligned with the physical world as
		/// tracking improves or drifts. Requires
		/// `SpatialCapability.Anchor` to be enabled and active.</summary>
		/// <param name="pose">A world space pose for the new anchor.</param>
		/// <returns>The new anchor entity. This is an invalid entity if
		/// anchoring is not active or creation failed, so check `Valid`.
		/// </returns>
		public static SpatialEntity CreateAnchor(Pose pose)
			=> new SpatialEntity(NativeAPI.spatial_entity_create_anchor(pose));

		/// <summary>Do these identify the same entity?</summary>
		/// <param name="other">The entity to compare with.</param>
		/// <returns>True if the identifiers match.</returns>
		public bool Equals(SpatialEntity other) => _id == other._id;
		/// <summary>Do these identify the same entity?</summary>
		/// <param name="obj">The object to compare with.</param>
		/// <returns>True if obj is a SpatialEntity with the same identifier.
		/// </returns>
		public override bool Equals(object obj) => obj is SpatialEntity e && _id == e._id;
		/// <summary>A hash of the identifier value.</summary>
		/// <returns>A hash of the identifier value.</returns>
		public override int GetHashCode() => (int)_id;
		/// <summary>Do these identify the same entity?</summary>
		/// <param name="a">First entity.</param>
		/// <param name="b">Second entity.</param>
		/// <returns>True if the identifiers match.</returns>
		public static bool operator ==(SpatialEntity a, SpatialEntity b) => a._id == b._id;
		/// <summary>Do these identify different entities?</summary>
		/// <param name="a">First entity.</param>
		/// <param name="b">Second entity.</param>
		/// <returns>True if the identifiers differ.</returns>
		public static bool operator !=(SpatialEntity a, SpatialEntity b) => a._id != b._id;
	}

	/// <summary>An enumerable collection of SpatialEntities, as provided by
	/// `SpatialEntity.All`, `With`, and `New`. Enumeration with foreach is
	/// allocation free.</summary>
	public readonly struct SpatialEntityCollection : IEnumerable<SpatialEntity>
	{
		readonly SpatialComponent _filter;
		readonly bool             _newOnly;
		internal SpatialEntityCollection(SpatialComponent filter, bool newOnly)
		{
			_filter  = filter;
			_newOnly = newOnly;
		}

		/// <summary>The number of entities in this collection.</summary>
		public int Count => _newOnly
			? NativeAPI.spatial_entity_get_new_count()
			: NativeAPI.spatial_entity_get_count(_filter);

		/// <summary>Gets an allocation free enumerator for this collection.
		/// </summary>
		/// <returns>A struct enumerator over the entities.</returns>
		public Enumerator GetEnumerator() => new Enumerator(_filter, _newOnly);
		IEnumerator<SpatialEntity> IEnumerable<SpatialEntity>.GetEnumerator() => GetEnumerator();
		IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

		/// <summary>A struct based enumerator over SpatialEntities, see
		/// SpatialEntityCollection.</summary>
		public struct Enumerator : IEnumerator<SpatialEntity>
		{
			readonly SpatialComponent _filter;
			readonly bool             _newOnly;
			int                       _index;
			SpatialEntity             _current;
			internal Enumerator(SpatialComponent filter, bool newOnly)
			{
				_filter  = filter;
				_newOnly = newOnly;
				_index   = -1;
				_current = default;
			}

			/// <summary>The entity at the enumerator's current position.
			/// </summary>
			public SpatialEntity Current => _current;
			object IEnumerator.Current => _current;

			/// <summary>Advances to the next entity in the collection.
			/// </summary>
			/// <returns>False once the end of the collection is reached.
			/// </returns>
			public bool MoveNext()
			{
				_index++;
				uint id = _newOnly
					? NativeAPI.spatial_entity_get_new_index(_index)
					: NativeAPI.spatial_entity_get_index(_filter, _index);
				if (id == 0) return false;
				_current = new SpatialEntity(id);
				return true;
			}

			/// <summary>Resets the enumerator to the start of the
			/// collection.</summary>
			public void Reset() { _index = -1; _current = default; }
			/// <summary>No unmanaged resources to release.</summary>
			public void Dispose() { }
		}
	}
}
