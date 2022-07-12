#include "physics.h"
#include "../stereokit.h"
#include "../_stereokit.h"
#include "../libraries/array.h"

#if !defined(SK_PHYSICS_PASSTHROUGH)
#pragma warning(push)
#pragma warning( disable: 4244 4267 4100 4099 4189 )
#include <reactphysics3d/reactphysics3d.h>
using namespace reactphysics3d;
#pragma warning(pop)
#endif

namespace sk {

///////////////////////////////////////////

struct solid_move_t {
	solid_t solid;
	vec3    dest;
	quat    dest_rot;
	vec3    old_velocity;
	vec3    old_rot_velocity;
};
array_t<solid_move_t> solid_moves = {};

double physics_sim_time = 0;
double physics_step = 1 / 90.0;

#if !defined(SK_PHYSICS_PASSTHROUGH)
PhysicsCommon physics_common;
PhysicsWorld *physics_world;

inline vec3       vec3_rp_to_sk(Vector3    v) { return {v.x, v.y, v.z     }; }
inline Vector3    vec3_sk_to_rp(vec3       v) { return {v.x, v.y, v.z     }; }
inline quat       quat_rp_to_sk(Quaternion q) { return {q.x, q.y, q.z, q.w}; }
inline Quaternion quat_sk_to_rp(quat       q) { return {q.x, q.y, q.z, q.w}; }
#endif

///////////////////////////////////////////

bool physics_init() {
#if !defined(SK_PHYSICS_PASSTHROUGH)
	physics_world = physics_common.createPhysicsWorld();
#endif
	return true;
}

///////////////////////////////////////////

void physics_shutdown() {
	solid_moves.free();

#if !defined(SK_PHYSICS_PASSTHROUGH)
	physics_common.destroyPhysicsWorld(physics_world);
#endif
}

///////////////////////////////////////////

void physics_update() {
	// How many physics frames are we going to be calculating this time?
	int32_t frames = (int32_t)ceil((sk_timev - physics_sim_time) / physics_step);
	if (frames <= 0)
		return;
	if (frames > (0.5f/physics_step))
		frames = (int32_t)(0.5f/physics_step);

#if !defined(SK_PHYSICS_PASSTHROUGH)
	// Calculate move velocities for objects that need to be at their destination by the end of this function!
	for (int32_t i = 0; i < solid_moves.count; i++) {
		solid_move_t &move = solid_moves[i];
		RigidBody    *body = (RigidBody*)move.solid->data;

		// Position
		move.old_velocity = vec3_rp_to_sk( body->getLinearVelocity() );
		vec3 pos      = vec3_rp_to_sk(body->getTransform().getPosition());
		vec3 velocity = (move.dest - pos) / (float)(physics_step * frames);
		body->setLinearVelocity(vec3_sk_to_rp(velocity));
		// Rotation
		move.old_rot_velocity = vec3_rp_to_sk(body->getAngularVelocity());
		quat rot = quat_rp_to_sk(body->getTransform().getOrientation());
		if (rot.x != move.dest_rot.x || rot.y != move.dest_rot.y || rot.z != move.dest_rot.z || rot.w != move.dest_rot.w) {
			Quaternion delta = quat_sk_to_rp(quat_difference(rot, move.dest_rot));
			float      angle;
			Vector3    axis;
			delta.getRotationAngleAxis(angle, axis);
			if (!isnan(angle)) {
				body->setAngularVelocity((angle / (reactphysics3d::decimal)(physics_step * frames)) * axis.getUnit());
			}
		}
	}

	// Sim physics!
	while (physics_sim_time < sk_timev) {
		physics_world->update((reactphysics3d::decimal)physics_step);
		physics_sim_time += physics_step;
	}

	// Reset moved objects back to their original values, and clear out our list
	for (int32_t i = 0; i < solid_moves.count; i++) {
		RigidBody *body = (RigidBody*)solid_moves[i].solid->data;
		body->setLinearVelocity (vec3_sk_to_rp(solid_moves[i].old_velocity));
		body->setAngularVelocity(vec3_sk_to_rp(solid_moves[i].old_rot_velocity));
	}
#else
	for (int32_t i = 0; i < solid_moves.count; i++) {
		solid_moves[i].solid->pos = solid_moves[i].dest;
		solid_moves[i].solid->rot = solid_moves[i].dest_rot;
	}
#endif
	solid_moves.clear();
}

///////////////////////////////////////////

solid_t solid_create(const vec3 &position, const quat &rotation, solid_type_ type) {
	solid_t result = (_solid_t*)assets_allocate(asset_type_solid);
#if !defined(SK_PHYSICS_PASSTHROUGH)
	result->data = physics_world->createRigidBody(Transform((Vector3 &)position, (Quaternion &)rotation));
#endif
	solid_set_type(result, type);
	return result;
}

///////////////////////////////////////////

void solid_release(solid_t solid) {
	if (solid == nullptr)
		return;

	assets_releaseref(&solid->header);
}

///////////////////////////////////////////

void solid_set_id(const solid_t solid, const char* id) {
	assets_set_id(&solid->header, id);
}

///////////////////////////////////////////

const char* solid_get_id(const solid_t solid) {
	return solid->header.id_text;
}

///////////////////////////////////////////

void solid_destroy(solid_t solid) {
	/*RigidBody *body  = (RigidBody*)solid;
	for (int i = body->getNbColliders()-1; i >= 0; i--) {
	const Collider *c = body->getCollider(i);
	switch (c->getCollisionShape()->getName()) {
	case CollisionShapeName::BOX:     physics_common.destroyBoxShape    ((BoxShape     *)c); break;
	case CollisionShapeName::SPHERE:  physics_common.destroySphereShape ((SphereShape  *)c); break;
	case CollisionShapeName::CAPSULE: physics_common.destroyCapsuleShape((CapsuleShape *)c); break;
	default:log_warn("Haven't added support for all physics shapes yet!");
	}
	}*/
#if !defined(SK_PHYSICS_PASSTHROUGH)
	physics_world->destroyRigidBody((RigidBody *)solid->data);
#endif
	*solid = {};
}

///////////////////////////////////////////

void solid_add_sphere(solid_t solid, float diameter, float kilograms, const vec3 *offset) {
#if !defined(SK_PHYSICS_PASSTHROUGH)
	RigidBody   *body   = (RigidBody*)solid->data;
	SphereShape *sphere = physics_common.createSphereShape(diameter/2);
	body->addCollider(sphere, Transform(offset == nullptr ? Vector3(0,0,0) : (Vector3 &)*offset, { 0,0,0,1 }));
	(void)kilograms;
#endif
}

///////////////////////////////////////////

void solid_add_box(solid_t solid, const vec3 &dimensions, float kilograms, const vec3 *offset) {
#if !defined(SK_PHYSICS_PASSTHROUGH)
	RigidBody *body = (RigidBody*)solid->data;
	BoxShape  *box  = physics_common.createBoxShape(Vector3{ dimensions.x / 2, dimensions.y / 2,dimensions.z / 2 });
	body->addCollider(box, Transform(offset == nullptr ? Vector3(0,0,0) : (Vector3 &)*offset, { 0,0,0,1 }));
	(void)kilograms;
#endif
}

///////////////////////////////////////////

void solid_add_capsule(solid_t solid, float diameter, float height, float kilograms, const vec3 *offset) {
#if !defined(SK_PHYSICS_PASSTHROUGH)
	RigidBody    *body    = (RigidBody*)solid->data;
	CapsuleShape *capsule = physics_common.createCapsuleShape(diameter/2, height);
	body->addCollider(capsule, Transform(offset == nullptr ? Vector3(0,0,0) : (Vector3 &)*offset, { 0,0,0,1 }));
	(void)kilograms;
#endif
}

///////////////////////////////////////////

void solid_set_type(solid_t solid, solid_type_ type) {
#if !defined(SK_PHYSICS_PASSTHROUGH)
	RigidBody *body = (RigidBody *)solid->data;

	switch (type) {
	case solid_type_normal:     body->setType(BodyType::DYNAMIC);   break;
	case solid_type_immovable:  body->setType(BodyType::STATIC);    break;
	case solid_type_unaffected: body->setType(BodyType::KINEMATIC); break;
	}
#endif
}

///////////////////////////////////////////

void solid_set_enabled(solid_t solid, bool32_t enabled) {
#if !defined(SK_PHYSICS_PASSTHROUGH)
	RigidBody *body = (RigidBody *)solid->data;
	body->setIsActive(enabled);
#endif
}

///////////////////////////////////////////

void solid_teleport(solid_t solid, const vec3 &position, const quat &rotation) {
#if !defined(SK_PHYSICS_PASSTHROUGH)
	RigidBody *body = (RigidBody *)solid->data;
	Transform t = Transform((Vector3 &)position, (Quaternion &)rotation);
	body->setTransform(t);
	//body->setIsSleeping(false);
#else
	solid->pos = position;
	solid->rot = rotation;
#endif
}

///////////////////////////////////////////

void solid_move(solid_t solid, const vec3 &position, const quat &rotation) {
	solid_moves.add(solid_move_t{solid, position, rotation});
}

///////////////////////////////////////////

void solid_set_velocity(solid_t solid, const vec3 &meters_per_second) {
#if !defined(SK_PHYSICS_PASSTHROUGH)
	RigidBody *body = (RigidBody *)solid->data;
	body->setLinearVelocity((Vector3&)meters_per_second);
#endif
}

///////////////////////////////////////////

void solid_set_velocity_ang(solid_t solid, const vec3 &radians_per_second) {
#if !defined(SK_PHYSICS_PASSTHROUGH)
	RigidBody *body = (RigidBody *)solid->data;
	body->setAngularVelocity((Vector3&)radians_per_second);
#endif
}

///////////////////////////////////////////

void solid_get_pose(const solid_t solid, pose_t &out_pose) {
#if !defined(SK_PHYSICS_PASSTHROUGH)
	const Transform &solid_tr = ((RigidBody *)solid->data)->getTransform();
	memcpy(&out_pose.position,    &solid_tr.getPosition   ().x, sizeof(vec3));
	memcpy(&out_pose.orientation, &solid_tr.getOrientation().x, sizeof(quat));
#else
	out_pose.position    = solid->pos;
	out_pose.orientation = solid->rot;
#endif
}

} // namespace sk