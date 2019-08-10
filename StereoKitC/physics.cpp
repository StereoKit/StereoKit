#pragma comment(lib, "reactphysics3d.lib")

#include "physics.h"
#include "stereokit.h"

#include <vector>
using namespace std;
#include <reactphysics3d.h>
using namespace reactphysics3d;

struct physics_shape_asset_t {
	int64_t id;
	void   *shape;
	int     refs;
};

double physics_sim_time = 0;
double physics_step = 1 / 90.0;

DynamicsWorld *physics_world;

vector<physics_shape_asset_t> physics_shapes;

void physics_init() {

	WorldSettings settings;
	physics_world = new DynamicsWorld(Vector3(0,-9.81f,0), settings);
}

void physics_shutdown() {
	delete physics_world;
}

void physics_update() {
	while (physics_sim_time < sk_timev) {
		physics_world->update(1.f / 90.f);
		physics_sim_time += physics_step;
	}
}

solid_t solid_create(const vec3 &position, const quat &rotation, solid_type_ type) {
	RigidBody *body = physics_world->createRigidBody(Transform((Vector3 &)position, (Quaternion &)rotation));
	solid_set_type(body, type);
	return (solid_t)body;
}
void solid_release(solid_t solid) {
	RigidBody        *body  = (RigidBody*)solid;
	const ProxyShape *shape = body->getProxyShapesList();
	while (shape != nullptr) {
		const CollisionShape *asset = shape->getCollisionShape();

		CollisionShapeName name = asset->getName();
		if (name == CollisionShapeName::BOX || name == CollisionShapeName::SPHERE || name == CollisionShapeName::CAPSULE)
			delete asset;
		else
			log_write(log_warning, "Haven't added support for all physics shapes yet!");

		shape = shape->getNext();
	}

	physics_world->destroyRigidBody((RigidBody *)solid);
}

void solid_add_sphere(solid_t solid, float diameter, float kilograms, const vec3 *offset) {
	RigidBody   *body   = (RigidBody*)solid;
	SphereShape *sphere = new SphereShape(diameter/2);

	body->addCollisionShape(sphere, Transform(offset == nullptr ? Vector3(0,0,0) : (Vector3 &)*offset, { 0,0,0,1 }), kilograms);
}

void solid_add_box(solid_t solid, const vec3 &dimensions, float kilograms, const vec3 *offset) {
	RigidBody *body = (RigidBody*)solid;
	BoxShape  *box  = new BoxShape((Vector3&)dimensions);

	body->addCollisionShape(box, Transform(offset == nullptr ? Vector3(0,0,0) : (Vector3 &)*offset, { 0,0,0,1 }), kilograms);
}

void solid_set_type(solid_t solid, solid_type_ type) {
	RigidBody *body = (RigidBody *)solid;

	switch (type) {
	case solid_type_normal:     body->setType(BodyType::DYNAMIC);   break;
	case solid_type_immovable:  body->setType(BodyType::STATIC);    break;
	case solid_type_unaffected: body->setType(BodyType::KINEMATIC); break;
	}
}

void solid_set_enabled(solid_t solid, bool32_t enabled) {
	RigidBody *body = (RigidBody *)solid;
	body->setIsActive(enabled);
}

void solid_set_pose(solid_t solid, const vec3 &position, const quat &rotation) {
	// Doesn't seem to like an empty non-identity quaternion
	//assert(!(rotation.a == 0 && rotation.i == 0 && rotation.j == 0 && rotation.k == 0));

	RigidBody *body = (RigidBody *)solid;
	//body->setLinearVelocity((Vector3&)((vec3&)body->getTransform().getPosition() - position));
	Transform t = Transform((Vector3 &)position, (Quaternion &)rotation);
	body->setTransform(t);
}

void solid_transform(const solid_t solid, transform_t &out_transform) {
	const Transform &solid_tr = ((RigidBody *)solid)->getTransform();
	memcpy(&out_transform._position, &solid_tr.getPosition   ().x, sizeof(vec3));
	memcpy(&out_transform._rotation, &solid_tr.getOrientation().x, sizeof(quat));
	out_transform._dirty = true;
}