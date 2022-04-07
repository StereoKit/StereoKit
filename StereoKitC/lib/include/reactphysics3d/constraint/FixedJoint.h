/********************************************************************************
* ReactPhysics3D physics library, http://www.reactphysics3d.com                 *
* Copyright (c) 2010-2022 Daniel Chappuis                                       *
*********************************************************************************
*                                                                               *
* This software is provided 'as-is', without any express or implied warranty.   *
* In no event will the authors be held liable for any damages arising from the  *
* use of this software.                                                         *
*                                                                               *
* Permission is granted to anyone to use this software for any purpose,         *
* including commercial applications, and to alter it and redistribute it        *
* freely, subject to the following restrictions:                                *
*                                                                               *
* 1. The origin of this software must not be misrepresented; you must not claim *
*    that you wrote the original software. If you use this software in a        *
*    product, an acknowledgment in the product documentation would be           *
*    appreciated but is not required.                                           *
*                                                                               *
* 2. Altered source versions must be plainly marked as such, and must not be    *
*    misrepresented as being the original software.                             *
*                                                                               *
* 3. This notice may not be removed or altered from any source distribution.    *
*                                                                               *
********************************************************************************/

#ifndef REACTPHYSICS3D_FIXED_JOINT_H
#define REACTPHYSICS3D_FIXED_JOINT_H

// Libraries
#include <reactphysics3d/constraint/Joint.h>
#include <reactphysics3d/mathematics/mathematics.h>

namespace reactphysics3d {

// Structure FixedJointInfo
/**
 * This structure is used to gather the information needed to create a fixed
 * joint. This structure will be used to create the actual fixed joint.
 */
struct FixedJointInfo : public JointInfo {

    public :

        // -------------------- Attributes -------------------- //

        /// True if this object has been constructed using local-space anchors
        bool isUsingLocalSpaceAnchors;

        /// Anchor point (in world-space coordinates)
        Vector3 anchorPointWorldSpace;

        /// Anchor point on body 1 (in local-space coordinates)
        Vector3 anchorPointBody1LocalSpace;

        /// Anchor point on body 2 (in local-space coordinates)
        Vector3 anchorPointBody2LocalSpace;

        /// Constructor
        /**
         * @param rigidBody1 The first body of the joint
         * @param rigidBody2 The second body of the joint
         * @param initAnchorPointWorldSpace The initial anchor point of the joint in
         *                                  world-space coordinates
         */
        FixedJointInfo(RigidBody* rigidBody1, RigidBody* rigidBody2,
                       const Vector3& initAnchorPointWorldSpace)
                       : JointInfo(rigidBody1, rigidBody2, JointType::FIXEDJOINT),
                         isUsingLocalSpaceAnchors(false),
                         anchorPointWorldSpace(initAnchorPointWorldSpace){}

        /// Constructor
        /**
         * @param rigidBody1 Pointer to the first body of the joint
         * @param rigidBody2 Pointer to the second body of the joint
         * @param anchorPointBody1LocalSpace The anchor point on body 1 in local-space coordinates
         * @param anchorPointBody2LocalSpace The anchor point on body 2 in local-space coordinates
         */
        FixedJointInfo(RigidBody* rigidBody1, RigidBody* rigidBody2,
                               const Vector3& anchorPointBody1LocalSpace,
                               const Vector3& anchorPointBody2LocalSpace)
                              : JointInfo(rigidBody1, rigidBody2, JointType::FIXEDJOINT),
                                isUsingLocalSpaceAnchors(true),
                                anchorPointBody1LocalSpace(anchorPointBody1LocalSpace),
                                anchorPointBody2LocalSpace(anchorPointBody2LocalSpace) {}
};

// Class FixedJoint
/**
 * This class represents a fixed joint that is used to forbid any translation or rotation
 * between two bodies.
 */
class FixedJoint : public Joint {

    private :

        // -------------------- Methods -------------------- //

        /// Return the number of bytes used by the joint
        virtual size_t getSizeInBytes() const override;

    public :

        // -------------------- Methods -------------------- //

        /// Constructor
        FixedJoint(Entity entity, PhysicsWorld& world, const FixedJointInfo& jointInfo);

        /// Destructor
        virtual ~FixedJoint() override = default;

        /// Deleted copy-constructor
        FixedJoint(const FixedJoint& constraint) = delete;

        /// Return the force (in Newtons) on body 2 required to satisfy the joint constraint in world-space
        virtual Vector3 getReactionForce(decimal timeStep) const override;

        /// Return the torque (in Newtons * meters) on body 2 required to satisfy the joint constraint in world-space
        virtual Vector3 getReactionTorque(decimal timeStep) const override;

        /// Return a string representation
        virtual std::string to_string() const override;

        /// Deleted assignment operator
        FixedJoint& operator=(const FixedJoint& constraint) = delete;
};

// Return the number of bytes used by the joint
RP3D_FORCE_INLINE size_t FixedJoint::getSizeInBytes() const {
    return sizeof(FixedJoint);
}

}

#endif
