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
	/// Entities are composed of components, which are chunks of data like a
	/// bounding rectangle, a semantic label, or a mesh. Which components an
	/// entity has depends on the capability that discovered it and what the
	/// device supports, so data is accessed through TryGet methods, and
	/// entities can be filtered by the components you need via `With`.
	///
	/// Request the capabilities you're interested in with
	/// `Spatial.Request`, and StereoKit will keep an up-to-date list of
	/// entities that you can poll each frame. Component data reflects the
	/// entity's last known state, so check `Tracked` to know if it's
	/// currently live.
	///
	/// SpatialEntity is a lightweight identifier, not a reference. The
	/// device owns these entities and controls their lifetimes. Identifiers
	/// are never reused within a session, so a stale one simply stops
	/// resolving once its entity is gone. `Valid` becomes false, and
	/// accessors return no data.</summary>
	public readonly struct SpatialEntity : IEquatable<SpatialEntity>
	{
		internal readonly ulong _id;

		internal SpatialEntity(ulong id) => _id = id;

		/// <summary>Does this identifier currently resolve to an entity?
		/// This becomes false once the entity permanently leaves the entity
		/// list, and a `default` SpatialEntity is never valid.</summary>
		public bool Valid => _id != 0 && NativeAPI.spatial_entity_is_valid(_id);

		/// <summary>Is the system tracking this entity right now? While
		/// active, `Pose` and all component data are live. While inactive,
		/// they're last known and may be stale, or empty if the entity hasn't
		/// been tracked yet. For requests still in progress, like a new
		/// anchor, check `Status` instead. Entities that are permanently lost
		/// leave the entity list, except persisted ones, which keep their
		/// identifier with `Status` Pending while storage loads them again.
		/// </summary>
		public BtnState Tracked => NativeAPI.spatial_entity_get_tracked(_id);
		/// <summary>Where this entity is in the world, from its anchor, or
		/// else the center of its 2D or 3D bounds. This is live while
		/// `Tracked` is active, and last known otherwise. It's
		/// `Pose.Identity` until the entity has any data, like a `FindAnchor`
		/// that hasn't loaded yet, or something first seen while untracked.
		/// For a specific meaning, use the matching TryGet.</summary>
		public Pose Pose => NativeAPI.spatial_entity_get_pose(_id);
		/// <summary>The set of components this entity has valid data for.
		/// Each component has a matching TryGet accessor.</summary>
		public SpatialComponent Components => NativeAPI.spatial_entity_get_components(_id);

		/// <summary>Whether the things you've asked of this entity have gone
		/// through, like creating, loading, or persisting it. Ready means
		/// all done, Pending means something's still in progress, and
		/// negative values are failures: Partial if a request like `Persist`
		/// failed, Failed if the entity couldn't be created or found at all.
		/// This is separate from `Tracked`, so a Pending anchor can still be
		/// tracked and usable.</summary>
		public SpatialStatus Status => NativeAPI.spatial_entity_get_status(_id);
		/// <summary>Components whose data meaningfully changed this frame!
		/// Continuously updating poses are only flagged when they first
		/// arrive, while mesh/polygon/marker data is flagged whenever the
		/// system provides new data. Handy for skipping expensive work like
		/// mesh extraction when nothing changed. These flags reset every
		/// frame, so check them each frame or you may miss an update.
		/// </summary>
		public SpatialComponent Changed => NativeAPI.spatial_entity_get_changed(_id);

		/// <summary>Does this entity have data for all of these components?
		/// </summary>
		/// <param name="components">One or more components to check.</param>
		/// <returns>True if every component given is present.</returns>
		public bool Has(SpatialComponent components)
			=> components != SpatialComponent.None && (Components & components) == components;

		/// <summary>Did any of these components change this frame? Like
		/// `Changed`, this resets every frame, so check it each frame.
		/// </summary>
		/// <param name="components">One or more components to check.</param>
		/// <returns>True if any component given changed this frame.</returns>
		public bool HasChanged(SpatialComponent components)
			=> (Changed & components) != SpatialComponent.None;
		/// <summary>The entity this entity is attached to. This is an
		/// invalid entity if there's no parent, so check `Valid`.</summary>
		public SpatialEntity Parent => new SpatialEntity(NativeAPI.spatial_entity_get_parent(_id));

		/// <summary>The 2D rectangular bounds of this entity, such as the
		/// extents of a detected plane, or the shape of a marker. The pose
		/// faces out of the surface, so its Forward is the surface normal,
		/// the same way quads and text face in StereoKit. A floor's pose faces
		/// up, and a wall's pose faces into the room. The center is usually
		/// the same as `Pose`, but can differ on entities with several pose
		/// components, like a table with both a top and a volume.</summary>
		/// <param name="center">The pose at the center of the rectangle.</param>
		/// <param name="size">The rectangle's total size in meters, along
		/// the center pose's X and Y axes.</param>
		/// <returns>False if this entity has no bounds2d component.</returns>
		public bool TryGetBounds2D(out Pose center, out Vec2 size)
			=> NativeAPI.spatial_entity_get_bounds2d(_id, out center, out size);

		/// <summary>The oriented 3D bounding volume of this entity. When
		/// the entity has a front, like a screen or table top, the center
		/// pose's Forward is the direction it faces. The center is usually
		/// the same as `Pose`, but can differ on entities with several pose
		/// components, like a table with both a top and a volume.</summary>
		/// <param name="center">The pose at the center of the volume.</param>
		/// <param name="size">The volume's total size in meters, along the
		/// center pose's axes.</param>
		/// <returns>False if this entity has no bounds3d component.</returns>
		public bool TryGetBounds3D(out Pose center, out Vec3 size)
			=> NativeAPI.spatial_entity_get_bounds3d(_id, out center, out size);

		/// <summary>The general orientation of a detected plane, like
		/// horizontal or vertical. Plane tracking always provides this, so
		/// it's a reliable fallback on devices that don't provide labels.
		/// </summary>
		/// <param name="alignment">The plane's general orientation, None if
		/// unavailable.</param>
		/// <returns>False if this entity has no plane alignment component.
		/// </returns>
		public bool TryGetPlaneAlign(out PlaneAlign alignment)
			=> NativeAPI.spatial_entity_get_plane_align(_id, out alignment);

		/// <summary>A semantic category for this entity, like floor or
		/// table. Not all devices provide labels, so check
		/// `Spatial.ComponentsFor` for the capability you're using. For planes,
		/// `TryGetPlaneAlign` makes a good fallback.</summary>
		/// <param name="label">The entity's semantic category, None if
		/// unavailable.</param>
		/// <returns>False if this entity has no label component.
		/// </returns>
		public bool TryGetLabel(out SpatialLabel label)
			=> NativeAPI.spatial_entity_get_label(_id, out label);

		/// <summary>The entity's 3D mesh! Mesh vertices are relative to the
		/// origin pose, which the system keeps aligned with the physical
		/// world, so draw the mesh at origin each frame. Filling the Mesh is
		/// the expensive path, so pass a null mesh to fetch just the current
		/// origin, and refill only when `Changed` flags the Mesh component.
		/// </summary>
		/// <param name="mesh">A valid Mesh to fill with the entity's
		/// geometry, or null to only retrieve the origin pose.</param>
		/// <param name="origin">Pose the mesh's vertices are relative to.</param>
		/// <returns>False if this entity has no mesh component.</returns>
		public bool TryGetMesh(Mesh mesh, out Pose origin)
			=> NativeAPI.spatial_entity_get_mesh(_id, mesh?._inst ?? IntPtr.Zero, out origin);

		/// <summary>The entity's 2D surface mesh, on the XY plane of the
		/// origin pose. Works just like `TryGetMesh`, so pass null to fetch
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
		/// system entirely. It's unpersisted if persisted, the system stops
		/// tracking it, and it leaves the entity list at the end of the
		/// frame. This SpatialEntity stops resolving once that happens.
		/// This also works on entities that are still Pending, including
		/// ones from `FindAnchor` that haven't loaded yet.</summary>
		/// <returns>False if this entity can't be destroyed! Only
		/// app-created entities like anchors can be, entities the system
		/// discovered on its own, like planes, cannot.</returns>
		public bool Destroy() => NativeAPI.spatial_entity_destroy(_id);

		/// <summary>A durable identifier for this entity that stays the same
		/// across sessions and device reboots! Entities only have one once
		/// they're persisted, either by the system itself, or by a call to
		/// `Persist`. Store it, and pass it to `FindAnchor` to get the same
		/// physical entity back in a later session.</summary>
		/// <param name="guid">The entity's persistent identifier,
		/// `Guid.Empty` if it has none.</param>
		/// <returns>False if this entity has no persistence component.
		/// </returns>
		public bool TryGetGuid(out Guid guid)
		{
			bool result = NativeAPI.spatial_entity_get_uuid(_id, out NativeUuid uuid);
			guid = result ? uuid.ToGuid() : Guid.Empty;
			return result;
		}

		/// <summary>Ask the system to persist this entity, giving it a
		/// durable identity that survives across sessions! This is
		/// asynchronous, and safe to call right away since it waits until
		/// the entity is tracking and persistence has started up. On success,
		/// `TryGetGuid` succeeds and `Changed` flags the Persistence
		/// component. On failure, `Status` becomes Partial and the entity
		/// stays without a Guid. If an `Unpersist` is still in flight, this
		/// persists again once it lands.</summary>
		/// <returns>False if this can't work at all, like an invalid entity,
		/// or one from a capability without persistence support. Check
		/// `Spatial.ComponentsFor` for `SpatialComponent.Persistence` to see
		/// which capabilities support it.</returns>
		public bool Persist() => NativeAPI.spatial_entity_persist(_id);

		/// <summary>Remove this entity from persistent storage. Its name, if
		/// it has one, is released right away, and it loses its Guid once
		/// the asynchronous operation completes. `Status` is Pending until
		/// then, or Partial if it fails. If a `Persist` is still in flight,
		/// this waits for it to land and then undoes it. An entity from
		/// `FindAnchor` that hasn't loaded yet stops loading, and once the
		/// unpersist lands it leaves the entity list, since there's nothing
		/// left to load.</summary>
		/// <returns>False if this can't work at all, like an invalid entity.
		/// </returns>
		public bool Unpersist() => NativeAPI.spatial_entity_unpersist(_id);

		/// <summary>Remove something from persistent storage using just its
		/// Guid, no live entity needed! This is how you clean up something
		/// stored in an earlier session that hasn't been rediscovered, or
		/// never will be. If persistence is still starting up, this waits
		/// for it.</summary>
		/// <param name="guid">A Guid from `TryGetGuid`, saved in an earlier
		/// session.</param>
		/// <returns>False if this can't work at all, like an empty Guid, or
		/// a system without persistence support.</returns>
		public static bool Unpersist(Guid guid)
			=> NativeAPI.spatial_entity_unpersist_uuid(NativeUuid.FromGuid(guid));

		/// <summary>Gets the anchor you created with this name, in this
		/// session or an earlier one! This is the easy way to restore anchors
		/// across app restarts. Call this once and hold onto the result.
		/// Storage loads asynchronously, so the anchor starts out with
		/// `Status` Pending and no data, then fills in when it loads. Nothing
		/// loads while `SpatialCapability.Anchor` isn't `Spatial.Requested`.
		/// </summary>
		/// <param name="name">The name given to `CreateAnchor`.</param>
		/// <returns>The anchor, already loaded or still loading. This is
		/// invalid if no anchor has this name, so check `Valid`.</returns>
		public static SpatialEntity FindAnchor(string name)
			=> new SpatialEntity(NativeAPI.spatial_entity_find_anchor(name));

		/// <summary>Gets the anchor with this Guid, for apps that keep track
		/// of anchors by Guid themselves. Works like `FindAnchor(string)`,
		/// and if storage doesn't have this Guid, `Status` becomes Failed and
		/// the anchor shows up in `Removed`.</summary>
		/// <param name="guid">A Guid from `TryGetGuid`, saved in an earlier
		/// session.</param>
		/// <returns>The anchor, already loaded or still loading. This is
		/// invalid for an empty Guid, or a system without persistence
		/// support.</returns>
		public static SpatialEntity FindAnchor(Guid guid)
			=> new SpatialEntity(NativeAPI.spatial_entity_find_anchor_uuid(NativeUuid.FromGuid(guid)));

		/// <summary>The number of entities that have data for all the given
		/// components.</summary>
		/// <param name="withComponents">Components to filter by, or None to
		/// count every entity.</param>
		/// <returns>The number of matching entities.</returns>
		public static int Count(SpatialComponent withComponents = SpatialComponent.None)
			=> NativeAPI.spatial_entity_get_count(withComponents);

		/// <summary>An enumeration of every spatial entity StereoKit
		/// currently knows about. This list is maintained for you, entities
		/// appear as the system discovers them, and leave when the system
		/// permanently stops tracking them. An entity in `Removed` is still
		/// in this list for its final frame.</summary>
		public static SpatialEntityCollection All => new SpatialEntityCollection(SpatialComponent.None, SpatialEntityCollection.List.All);

		/// <summary>An enumeration of the spatial entities that have data
		/// for all the given components.</summary>
		/// <param name="components">Components each entity must have.</param>
		/// <returns>An enumeration of matching entities.</returns>
		public static SpatialEntityCollection With(SpatialComponent components)
			=> new SpatialEntityCollection(components, SpatialEntityCollection.List.All);

		/// <summary>An enumeration of the spatial entities that appeared
		/// for the first time this frame. Entities you create yourself, like
		/// with `CreateAnchor` or `FindAnchor`, show up here on the
		/// following frame, so every entity is seen here exactly once.
		/// </summary>
		public static SpatialEntityCollection New => new SpatialEntityCollection(SpatialComponent.None, SpatialEntityCollection.List.New);

		/// <summary>An enumeration of the spatial entities that appeared
		/// for the first time this frame, and have data for all the given
		/// components.</summary>
		/// <param name="components">Components each entity must have.</param>
		/// <returns>An enumeration of matching new entities.</returns>
		public static SpatialEntityCollection NewWith(SpatialComponent components)
			=> new SpatialEntityCollection(components, SpatialEntityCollection.List.New);

		/// <summary>An enumeration of the spatial entities leaving the entity
		/// list this frame, because they were lost by the system, destroyed,
		/// or `Status` Failed. Lost persisted entities don't leave, they wait to load
		/// again. These are still `Valid` with readable data for this one
		/// frame, which makes this the place to clean up anything you've
		/// cached per-entity!</summary>
		public static SpatialEntityCollection Removed => new SpatialEntityCollection(SpatialComponent.None, SpatialEntityCollection.List.Removed);

		/// <summary>An enumeration of the spatial entities leaving the entity
		/// list this frame that have data for all the given components.
		/// </summary>
		/// <param name="components">Components each entity must have.</param>
		/// <returns>An enumeration of matching removed entities.</returns>
		public static SpatialEntityCollection RemovedWith(SpatialComponent components)
			=> new SpatialEntityCollection(components, SpatialEntityCollection.List.Removed);

		/// <summary>Create a spatial anchor entity at the given pose, a
		/// point the system will keep aligned with the physical world as
		/// tracking improves or drifts. You can call this any time after
		/// requesting `SpatialCapability.Anchor`, even while it's still
		/// starting up! Until the system takes over, the anchor sits at
		/// this pose with `Tracked` inactive and `Status` Pending. If the
		/// system can't create it, the anchor shows up in `Removed` with
		/// `Status` Failed.</summary>
		/// <param name="pose">A world space pose for the new anchor.</param>
		/// <param name="name">Persists the anchor under this name, so
		/// `FindAnchor` can get it back in a later session. Any anchor that
		/// already has this name is destroyed, so placing "table" again moves
		/// it. Leave this null for an anchor that only lasts this session,
		/// or call `Persist` later to keep it by Guid instead.</param>
		/// <param name="parent">An optional entity to attach the anchor
		/// to, so it follows that entity as it moves. Few runtimes support
		/// this yet, and creation fails on those that don't.</param>
		/// <returns>The new anchor entity. This is an invalid entity if
		/// this system doesn't support anchors, or can't persist them when
		/// given a name, so check `Valid`.</returns>
		public static SpatialEntity CreateAnchor(Pose pose, string name = null, SpatialEntity parent = default)
			=> new SpatialEntity(NativeAPI.spatial_entity_create_anchor(pose, name, parent._id));

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
		public override int GetHashCode() => _id.GetHashCode();
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
	/// `SpatialEntity.All`, `With`, `New`, and `Removed`. Enumeration with
	/// foreach is allocation free.</summary>
	public readonly struct SpatialEntityCollection : IEnumerable<SpatialEntity>
	{
		internal enum List { All, New, Removed }

		readonly SpatialComponent _filter;
		readonly List             _list;
		internal SpatialEntityCollection(SpatialComponent filter, List list)
		{
			_filter = filter;
			_list   = list;
		}

		/// <summary>The number of entities in this collection.</summary>
		public int Count => _list switch {
			List.New     => NativeAPI.spatial_entity_get_new_count    (_filter),
			List.Removed => NativeAPI.spatial_entity_get_removed_count(_filter),
			_            => NativeAPI.spatial_entity_get_count        (_filter),
		};

		/// <summary>Gets an allocation free enumerator for this collection.
		/// </summary>
		/// <returns>A struct enumerator over the entities.</returns>
		public Enumerator GetEnumerator() => new Enumerator(_filter, _list);
		IEnumerator<SpatialEntity> IEnumerable<SpatialEntity>.GetEnumerator() => GetEnumerator();
		IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

		/// <summary>A struct based enumerator over SpatialEntities, see
		/// SpatialEntityCollection.</summary>
		public struct Enumerator : IEnumerator<SpatialEntity>
		{
			readonly SpatialComponent _filter;
			readonly List             _list;
			int                       _index;
			SpatialEntity             _current;
			internal Enumerator(SpatialComponent filter, List list)
			{
				_filter  = filter;
				_list    = list;
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
				ulong id = _list switch {
					List.New     => NativeAPI.spatial_entity_get_new_index    (_filter, _index),
					List.Removed => NativeAPI.spatial_entity_get_removed_index(_filter, _index),
					_            => NativeAPI.spatial_entity_get_index        (_filter, _index),
				};
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
