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

#ifndef REACTPHYSICS3D_HINGE_JOINT_COMPONENTS_H
#define REACTPHYSICS3D_HINGE_JOINT_COMPONENTS_H

// Libraries
#include <reactphysics3d/mathematics/Transform.h>
#include <reactphysics3d/mathematics/Matrix3x3.h>
#include <reactphysics3d/mathematics/Matrix2x2.h>
#include <reactphysics3d/engine/Entity.h>
#include <reactphysics3d/components/Components.h>
#include <reactphysics3d/containers/Map.h>

// ReactPhysics3D namespace
namespace reactphysics3d {

// Class declarations
class MemoryAllocator;
class EntityManager;
class HingeJoint;
enum class JointType;

// Class HingeJointComponents
/**
 * This class represent the component of the ECS with data for the HingeJoint.
 */
class HingeJointComponents : public Components {

    private:

        // -------------------- Attributes -------------------- //

        /// Array of joint entities
        Entity* mJointEntities;

        /// Array of pointers to the joints
        HingeJoint** mJoints;

        /// Anchor point of body 1 (in local-space coordinates of body 1)
        Vector3* mLocalAnchorPointBody1;

        /// Anchor point of body 2 (in local-space coordinates of body 2)
        Vector3* mLocalAnchorPointBody2;

        /// Vector from center of body 2 to anchor point in world-space
        Vector3* mR1World;

        /// Vector from center of body 2 to anchor point in world-space
        Vector3* mR2World;

        /// Inertia tensor of body 1 (in world-space coordinates)
        Matrix3x3* mI1;

        /// Inertia tensor of body 2 (in world-space coordinates)
        Matrix3x3* mI2;

        /// Accumulated impulse for the 3 translation constraints
        Vector3* mImpulseTranslation;

        /// Accumulate impulse for the 3 rotation constraints
        Vector2* mImpulseRotation;

        /// Inverse mass matrix K=JM^-1J^-t of the 3 translation constraints (3x3 matrix)
        Matrix3x3* mInverseMassMatrixTranslation;

        /// Inverse mass matrix K=JM^-1J^-t of the 3 rotation constraints (3x3 matrix)
        Matrix2x2* mInverseMassMatrixRotation;

        /// Bias vector for the 3 translation constraints
        Vector3* mBiasTranslation;

        /// Bias vector for the 3 rotation constraints
        Vector2* mBiasRotation;

        /// Inverse of the initial orientation difference between the two bodies
        Quaternion* mInitOrientationDifferenceInv;

        /// Hinge rotation axis (in local-space coordinates of body 1)
        Vector3* mHingeLocalAxisBody1;

        /// Hinge rotation axis (in local-space coordiantes of body 2)
        Vector3* mHingeLocalAxisBody2;

        /// Hinge rotation axis (in world-space coordinates) computed from body 1
        Vector3* mA1;

        /// Cross product of vector b2 and a1
        Vector3* mB2CrossA1;

        /// Cross product of vector c2 and a1;
        Vector3* mC2CrossA1;

        /// Accumulated impulse for the lower limit constraint
        decimal* mImpulseLowerLimit;

        /// Accumulated impulse for the upper limit constraint
        decimal* mImpulseUpperLimit;

        /// Accumulated impulse for the motor constraint;
        decimal* mImpulseMotor;

        /// Inverse of mass matrix K=JM^-1J^t for the limits and motor constraints (1x1 matrix)
        decimal* mInverseMassMatrixLimitMotor;

        /// Inverse of mass matrix K=JM^-1J^t for the motor
        decimal* mInverseMassMatrixMotor;

        /// Bias of the lower limit constraint
        decimal* mBLowerLimit;

        /// Bias of the upper limit constraint
        decimal* mBUpperLimit;

        /// True if the joint limits are enabled
        bool* mIsLimitEnabled;

        /// True if the motor of the joint in enabled
        bool* mIsMotorEnabled;

        /// Lower limit (minimum allowed rotation angle in radians)
        decimal* mLowerLimit;

        /// Upper limit (maximum allowed rotation angle in radians)
        decimal* mUpperLimit;

        /// True if the lower limit is violated
        bool* mIsLowerLimitViolated;

        /// True if the upper limit is violated
        bool* mIsUpperLimitViolated;

        /// Motor speed (in rad/s)
        decimal* mMotorSpeed;

        /// Maximum motor torque (in Newtons) that can be applied to reach to desired motor speed
        decimal* mMaxMotorTorque;

        // -------------------- Methods -------------------- //

        /// Allocate memory for a given number of components
        virtual void allocate(uint32 nbComponentsToAllocate) override;

        /// Destroy a component at a given index
        virtual void destroyComponent(uint32 index) override;

        /// Move a component from a source to a destination index in the components array
        virtual void moveComponentToIndex(uint32 srcIndex, uint32 destIndex) override;

        /// Swap two components in the array
        virtual void swapComponents(uint32 index1, uint32 index2) override;

    public:

        /// Structure for the data of a transform component
        struct HingeJointComponent {

             bool isLimitEnabled;
             bool isMotorEnabled;
             decimal lowerLimit;
             decimal upperLimit;
             decimal motorSpeed;
             decimal maxMotorTorque;

            /// Constructor
            HingeJointComponent(bool isLimitEnabled, bool isMotorEnabled, decimal lowerLimit, decimal upperLimit,
                                decimal motorSpeed, decimal maxMotorTorque)
                : isLimitEnabled(isLimitEnabled), isMotorEnabled(isMotorEnabled), lowerLimit(lowerLimit), upperLimit(upperLimit),
                  motorSpeed(motorSpeed), maxMotorTorque(maxMotorTorque) {

            }
        };

        // -------------------- Methods -------------------- //

        /// Constructor
        HingeJointComponents(MemoryAllocator& allocator);

        /// Destructor
        virtual ~HingeJointComponents() override = default;

        /// Add a component
        void addComponent(Entity jointEntity, bool isSleeping, const HingeJointComponent& component);

        /// Return a pointer to a given joint
        HingeJoint* getJoint(Entity jointEntity) const;

        /// Set the joint pointer to a given joint
        void setJoint(Entity jointEntity, HingeJoint* joint) const;

        /// Return the local anchor point of body 1 for a given joint
        const Vector3& getLocalAnchorPointBody1(Entity jointEntity) const;

        /// Set the local anchor point of body 1 for a given joint
        void setLocalAnchorPointBody1(Entity jointEntity, const Vector3& localAnchoirPointBody1);

        /// Return the local anchor point of body 2 for a given joint
        const Vector3& getLocalAnchorPointBody2(Entity jointEntity) const;

        /// Set the local anchor point of body 2 for a given joint
        void setLocalAnchorPointBody2(Entity jointEntity, const Vector3& localAnchoirPointBody2);

        /// Return the vector from center of body 1 to anchor point in world-space
        const Vector3& getR1World(Entity jointEntity) const;

        /// Set the vector from center of body 1 to anchor point in world-space
        void setR1World(Entity jointEntity, const Vector3& r1World);

        /// Return the vector from center of body 2 to anchor point in world-space
        const Vector3& getR2World(Entity jointEntity) const;

        /// Set the vector from center of body 2 to anchor point in world-space
        void setR2World(Entity jointEntity, const Vector3& r2World);

        /// Return the inertia tensor of body 1 (in world-space coordinates)
        const Matrix3x3& getI1(Entity jointEntity) const;

        /// Set the inertia tensor of body 1 (in world-space coordinates)
        void setI1(Entity jointEntity, const Matrix3x3& i1);

        /// Return the inertia tensor of body 2 (in world-space coordinates)
        const Matrix3x3& getI2(Entity jointEntity) const;

        /// Set the inertia tensor of body 2 (in world-space coordinates)
        void setI2(Entity jointEntity, const Matrix3x3& i2);

        /// Return the translation impulse
        Vector3& getImpulseTranslation(Entity jointEntity);

        /// Set the translation impulse
        void setImpulseTranslation(Entity jointEntity, const Vector3& impulseTranslation);

        /// Return the translation impulse
        Vector2& getImpulseRotation(Entity jointEntity);

        /// Set the translation impulse
        void setImpulseRotation(Entity jointEntity, const Vector2& impulseTranslation);

        /// Return the translation inverse mass matrix of the constraint
        Matrix3x3& getInverseMassMatrixTranslation(Entity jointEntity);

        /// Set the translation inverse mass matrix of the constraint
        void setInverseMassMatrixTranslation(Entity jointEntity, const Matrix3x3& inverseMassMatrix);

        /// Return the rotation inverse mass matrix of the constraint
        Matrix2x2& getInverseMassMatrixRotation(Entity jointEntity);

        /// Set the rotation inverse mass matrix of the constraint
        void setInverseMassMatrixRotation(Entity jointEntity, const Matrix2x2& inverseMassMatrix);

        /// Return the translation bias
        Vector3& getBiasTranslation(Entity jointEntity);

        /// Set the translation impulse
        void setBiasTranslation(Entity jointEntity, const Vector3& impulseTranslation);

        /// Return the rotation bias
        Vector2& getBiasRotation(Entity jointEntity);

        /// Set the rotation impulse
        void setBiasRotation(Entity jointEntity, const Vector2& impulseRotation);

        /// Return the initial orientation difference
        Quaternion& getInitOrientationDifferenceInv(Entity jointEntity);

        /// Set the rotation impulse
        void setInitOrientationDifferenceInv(Entity jointEntity, const Quaternion& initOrientationDifferenceInv);

        /// Return the hinge rotation axis (in local-space coordinates of body 1)
        Vector3& getHingeLocalAxisBody1(Entity jointEntity);

        /// Set the hinge rotation axis (in local-space coordinates of body 1)
        void setHingeLocalAxisBody1(Entity jointEntity, const Vector3& hingeLocalAxisBody1);

        /// Return the hinge rotation axis (in local-space coordiantes of body 2)
        Vector3& getHingeLocalAxisBody2(Entity jointEntity);

        /// Set the hinge rotation axis (in local-space coordiantes of body 2)
        void setHingeLocalAxisBody2(Entity jointEntity, const Vector3& hingeLocalAxisBody2);

        /// Return the hinge rotation axis (in world-space coordinates) computed from body 1
        Vector3& getA1(Entity jointEntity);

        /// Set the hinge rotation axis (in world-space coordinates) computed from body 1
        void setA1(Entity jointEntity, const Vector3& a1);

        /// Return the cross product of vector b2 and a1
        Vector3& getB2CrossA1(Entity jointEntity);

        /// Set the cross product of vector b2 and a1
        void setB2CrossA1(Entity jointEntity, const Vector3& b2CrossA1);

        /// Return the cross product of vector c2 and a1;
        Vector3& getC2CrossA1(Entity jointEntity);

        /// Set the cross product of vector c2 and a1;
        void setC2CrossA1(Entity jointEntity, const Vector3& c2CrossA1);

        /// Return the accumulated impulse for the lower limit constraint
        decimal getImpulseLowerLimit(Entity jointEntity) const;

        /// Set the accumulated impulse for the lower limit constraint
        void setImpulseLowerLimit(Entity jointEntity, decimal impulseLowerLimit);

        /// Return the accumulated impulse for the upper limit constraint
        decimal getImpulseUpperLimit(Entity jointEntity) const;

        /// Set the accumulated impulse for the upper limit constraint
        void setImpulseUpperLimit(Entity jointEntity, decimal impulseUpperLimit) const;

        /// Return the accumulated impulse for the motor constraint;
        decimal getImpulseMotor(Entity jointEntity) const;

        /// Set the accumulated impulse for the motor constraint;
        void setImpulseMotor(Entity jointEntity, decimal impulseMotor);

        /// Return the inverse of mass matrix K=JM^-1J^t for the limits and motor constraints (1x1 matrix)
        decimal getInverseMassMatrixLimitMotor(Entity jointEntity) const;

        /// Set the inverse of mass matrix K=JM^-1J^t for the limits and motor constraints (1x1 matrix)
        void setInverseMassMatrixLimitMotor(Entity jointEntity, decimal inverseMassMatrixLimitMotor);

        /// Return the inverse of mass matrix K=JM^-1J^t for the motor
        decimal getInverseMassMatrixMotor(Entity jointEntity);

        /// Set the inverse of mass matrix K=JM^-1J^t for the motor
        void setInverseMassMatrixMotor(Entity jointEntity, decimal inverseMassMatrixMotor);

        /// Return the bias of the lower limit constraint
        decimal getBLowerLimit(Entity jointEntity) const;

        /// Set the bias of the lower limit constraint
        void setBLowerLimit(Entity jointEntity, decimal bLowerLimit) const;

        /// Return the bias of the upper limit constraint
        decimal getBUpperLimit(Entity jointEntity) const;

        /// Set the bias of the upper limit constraint
        void setBUpperLimit(Entity jointEntity, decimal bUpperLimit);

        /// Return true if the joint limits are enabled
        bool getIsLimitEnabled(Entity jointEntity) const;

        /// Set to true if the joint limits are enabled
        void setIsLimitEnabled(Entity jointEntity, bool isLimitEnabled);

        /// Return true if the motor of the joint in enabled
        bool getIsMotorEnabled(Entity jointEntity) const;

        /// Set to true if the motor of the joint in enabled
        void setIsMotorEnabled(Entity jointEntity, bool isMotorEnabled) const;

        /// Return the Lower limit (minimum allowed rotation angle in radian)
        decimal getLowerLimit(Entity jointEntity) const;

        /// Set the Lower limit (minimum allowed rotation angle in radian)
        void setLowerLimit(Entity jointEntity, decimal lowerLimit) const;

        /// Return the upper limit (maximum translation distance)
        decimal getUpperLimit(Entity jointEntity) const;

        /// Set the upper limit (maximum translation distance)
        void setUpperLimit(Entity jointEntity, decimal upperLimit);

        /// Return true if the lower limit is violated
        bool getIsLowerLimitViolated(Entity jointEntity) const;

        /// Set to true if the lower limit is violated
        void setIsLowerLimitViolated(Entity jointEntity, bool isLowerLimitViolated);

        /// Return true if the upper limit is violated
        bool getIsUpperLimitViolated(Entity jointEntity) const;

        /// Set to true if the upper limit is violated
        void setIsUpperLimitViolated(Entity jointEntity, bool isUpperLimitViolated) const;

        /// Return the motor speed (in rad/s)
        decimal getMotorSpeed(Entity jointEntity) const;

        /// Set the motor speed (in rad/s)
        void setMotorSpeed(Entity jointEntity, decimal motorSpeed);

        /// Return the maximum motor torque (in Newtons) that can be applied to reach to desired motor speed
        decimal getMaxMotorTorque(Entity jointEntity) const;

        /// Set the maximum motor torque (in Newtons) that can be applied to reach to desired motor speed
        void setMaxMotorTorque(Entity jointEntity, decimal maxMotorTorque);

        // -------------------- Friendship -------------------- //

        friend class BroadPhaseSystem;
        friend class SolveHingeJointSystem;
        friend class HingeJoint;
};

// Return a pointer to a given joint
RP3D_FORCE_INLINE HingeJoint* HingeJointComponents::getJoint(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mJoints[mMapEntityToComponentIndex[jointEntity]];
}

// Set the joint pointer to a given joint
RP3D_FORCE_INLINE void HingeJointComponents::setJoint(Entity jointEntity, HingeJoint* joint) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mJoints[mMapEntityToComponentIndex[jointEntity]] = joint;
}

// Return the local anchor point of body 1 for a given joint
RP3D_FORCE_INLINE const Vector3& HingeJointComponents::getLocalAnchorPointBody1(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mLocalAnchorPointBody1[mMapEntityToComponentIndex[jointEntity]];
}

// Set the local anchor point of body 1 for a given joint
RP3D_FORCE_INLINE void HingeJointComponents::setLocalAnchorPointBody1(Entity jointEntity, const Vector3& localAnchoirPointBody1) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mLocalAnchorPointBody1[mMapEntityToComponentIndex[jointEntity]] = localAnchoirPointBody1;
}

// Return the local anchor point of body 2 for a given joint
RP3D_FORCE_INLINE const Vector3& HingeJointComponents::getLocalAnchorPointBody2(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mLocalAnchorPointBody2[mMapEntityToComponentIndex[jointEntity]];
}

// Set the local anchor point of body 2 for a given joint
RP3D_FORCE_INLINE void HingeJointComponents::setLocalAnchorPointBody2(Entity jointEntity, const Vector3& localAnchoirPointBody2) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mLocalAnchorPointBody2[mMapEntityToComponentIndex[jointEntity]] = localAnchoirPointBody2;
}

// Return the vector from center of body 1 to anchor point in world-space
RP3D_FORCE_INLINE const Vector3& HingeJointComponents::getR1World(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mR1World[mMapEntityToComponentIndex[jointEntity]];
}

// Set the vector from center of body 1 to anchor point in world-space
RP3D_FORCE_INLINE void HingeJointComponents::setR1World(Entity jointEntity, const Vector3& r1World) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mR1World[mMapEntityToComponentIndex[jointEntity]] = r1World;
}

// Return the vector from center of body 2 to anchor point in world-space
RP3D_FORCE_INLINE const Vector3& HingeJointComponents::getR2World(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mR2World[mMapEntityToComponentIndex[jointEntity]];
}

// Set the vector from center of body 2 to anchor point in world-space
RP3D_FORCE_INLINE void HingeJointComponents::setR2World(Entity jointEntity, const Vector3& r2World) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mR2World[mMapEntityToComponentIndex[jointEntity]] = r2World;
}

// Return the inertia tensor of body 1 (in world-space coordinates)
RP3D_FORCE_INLINE const Matrix3x3& HingeJointComponents::getI1(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mI1[mMapEntityToComponentIndex[jointEntity]];
}

// Set the inertia tensor of body 1 (in world-space coordinates)
RP3D_FORCE_INLINE void HingeJointComponents::setI1(Entity jointEntity, const Matrix3x3& i1) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mI1[mMapEntityToComponentIndex[jointEntity]] = i1;
}

// Return the inertia tensor of body 2 (in world-space coordinates)
RP3D_FORCE_INLINE const Matrix3x3& HingeJointComponents::getI2(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mI2[mMapEntityToComponentIndex[jointEntity]];
}

// Set the inertia tensor of body 2 (in world-space coordinates)
RP3D_FORCE_INLINE void HingeJointComponents::setI2(Entity jointEntity, const Matrix3x3& i2) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mI2[mMapEntityToComponentIndex[jointEntity]] = i2;
}

// Return the translation impulse
RP3D_FORCE_INLINE Vector3& HingeJointComponents::getImpulseTranslation(Entity jointEntity) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mImpulseTranslation[mMapEntityToComponentIndex[jointEntity]];
}

// Set the translation impulse
RP3D_FORCE_INLINE void HingeJointComponents::setImpulseTranslation(Entity jointEntity, const Vector3& impulseTranslation) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mImpulseTranslation[mMapEntityToComponentIndex[jointEntity]] = impulseTranslation;
}

// Return the translation impulse
RP3D_FORCE_INLINE Vector2& HingeJointComponents::getImpulseRotation(Entity jointEntity) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mImpulseRotation[mMapEntityToComponentIndex[jointEntity]];
}

// Set the translation impulse
RP3D_FORCE_INLINE void HingeJointComponents::setImpulseRotation(Entity jointEntity, const Vector2& impulseTranslation) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mImpulseRotation[mMapEntityToComponentIndex[jointEntity]] = impulseTranslation;
}

// Return the translation inverse mass matrix of the constraint
RP3D_FORCE_INLINE Matrix3x3& HingeJointComponents::getInverseMassMatrixTranslation(Entity jointEntity) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mInverseMassMatrixTranslation[mMapEntityToComponentIndex[jointEntity]];
}


// Set the translation inverse mass matrix of the constraint
RP3D_FORCE_INLINE void HingeJointComponents::setInverseMassMatrixTranslation(Entity jointEntity, const Matrix3x3& inverseMassMatrix) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mInverseMassMatrixTranslation[mMapEntityToComponentIndex[jointEntity]] = inverseMassMatrix;
}

// Return the rotation inverse mass matrix of the constraint
RP3D_FORCE_INLINE Matrix2x2& HingeJointComponents::getInverseMassMatrixRotation(Entity jointEntity) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mInverseMassMatrixRotation[mMapEntityToComponentIndex[jointEntity]];
}

// Set the rotation inverse mass matrix of the constraint
RP3D_FORCE_INLINE void HingeJointComponents::setInverseMassMatrixRotation(Entity jointEntity, const Matrix2x2& inverseMassMatrix) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mInverseMassMatrixRotation[mMapEntityToComponentIndex[jointEntity]] = inverseMassMatrix;
}

// Return the translation bias
RP3D_FORCE_INLINE Vector3& HingeJointComponents::getBiasTranslation(Entity jointEntity) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mBiasTranslation[mMapEntityToComponentIndex[jointEntity]];
}

// Set the translation impulse
RP3D_FORCE_INLINE void HingeJointComponents::setBiasTranslation(Entity jointEntity, const Vector3 &impulseTranslation) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mBiasTranslation[mMapEntityToComponentIndex[jointEntity]] = impulseTranslation;
}

// Return the rotation bias
RP3D_FORCE_INLINE Vector2 &HingeJointComponents::getBiasRotation(Entity jointEntity) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mBiasRotation[mMapEntityToComponentIndex[jointEntity]];
}

// Set the rotation impulse
RP3D_FORCE_INLINE void HingeJointComponents::setBiasRotation(Entity jointEntity, const Vector2& impulseRotation) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mBiasRotation[mMapEntityToComponentIndex[jointEntity]] = impulseRotation;
}

// Return the initial orientation difference
RP3D_FORCE_INLINE Quaternion& HingeJointComponents::getInitOrientationDifferenceInv(Entity jointEntity) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mInitOrientationDifferenceInv[mMapEntityToComponentIndex[jointEntity]];
}

// Set the rotation impulse
RP3D_FORCE_INLINE void HingeJointComponents::setInitOrientationDifferenceInv(Entity jointEntity, const Quaternion& initOrientationDifferenceInv) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mInitOrientationDifferenceInv[mMapEntityToComponentIndex[jointEntity]] = initOrientationDifferenceInv;
}

// Return the hinge rotation axis (in local-space coordinates of body 1)
RP3D_FORCE_INLINE Vector3& HingeJointComponents::getHingeLocalAxisBody1(Entity jointEntity) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mHingeLocalAxisBody1[mMapEntityToComponentIndex[jointEntity]];
}

// Set the hinge rotation axis (in local-space coordinates of body 1)
RP3D_FORCE_INLINE void HingeJointComponents::setHingeLocalAxisBody1(Entity jointEntity, const Vector3& hingeLocalAxisBody1) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mHingeLocalAxisBody1[mMapEntityToComponentIndex[jointEntity]] = hingeLocalAxisBody1;
}

// Return the hinge rotation axis (in local-space coordiantes of body 2)
RP3D_FORCE_INLINE Vector3& HingeJointComponents::getHingeLocalAxisBody2(Entity jointEntity) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mHingeLocalAxisBody2[mMapEntityToComponentIndex[jointEntity]];
}

// Set the hinge rotation axis (in local-space coordiantes of body 2)
RP3D_FORCE_INLINE void HingeJointComponents::setHingeLocalAxisBody2(Entity jointEntity, const Vector3& hingeLocalAxisBody2) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mHingeLocalAxisBody2[mMapEntityToComponentIndex[jointEntity]] = hingeLocalAxisBody2;
}


// Return the hinge rotation axis (in world-space coordinates) computed from body 1
RP3D_FORCE_INLINE Vector3& HingeJointComponents::getA1(Entity jointEntity) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mA1[mMapEntityToComponentIndex[jointEntity]];
}

// Set the hinge rotation axis (in world-space coordinates) computed from body 1
RP3D_FORCE_INLINE void HingeJointComponents::setA1(Entity jointEntity, const Vector3& a1) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mA1[mMapEntityToComponentIndex[jointEntity]] = a1;
}

// Return the cross product of vector b2 and a1
RP3D_FORCE_INLINE Vector3& HingeJointComponents::getB2CrossA1(Entity jointEntity) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mB2CrossA1[mMapEntityToComponentIndex[jointEntity]];
}

// Set the cross product of vector b2 and a1
RP3D_FORCE_INLINE void HingeJointComponents::setB2CrossA1(Entity jointEntity, const Vector3& b2CrossA1) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mB2CrossA1[mMapEntityToComponentIndex[jointEntity]] = b2CrossA1;
}

// Return the cross product of vector c2 and a1;
RP3D_FORCE_INLINE Vector3& HingeJointComponents::getC2CrossA1(Entity jointEntity) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mC2CrossA1[mMapEntityToComponentIndex[jointEntity]];
}

// Set the cross product of vector c2 and a1;
RP3D_FORCE_INLINE void HingeJointComponents::setC2CrossA1(Entity jointEntity, const Vector3& c2CrossA1) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mC2CrossA1[mMapEntityToComponentIndex[jointEntity]] = c2CrossA1;
}

// Return the accumulated impulse for the lower limit constraint
RP3D_FORCE_INLINE decimal HingeJointComponents::getImpulseLowerLimit(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mImpulseLowerLimit[mMapEntityToComponentIndex[jointEntity]];
}

// Set the accumulated impulse for the lower limit constraint
RP3D_FORCE_INLINE void HingeJointComponents::setImpulseLowerLimit(Entity jointEntity, decimal impulseLowerLimit) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mImpulseLowerLimit[mMapEntityToComponentIndex[jointEntity]] = impulseLowerLimit;
}


// Return the accumulated impulse for the upper limit constraint
RP3D_FORCE_INLINE decimal HingeJointComponents::getImpulseUpperLimit(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mImpulseUpperLimit[mMapEntityToComponentIndex[jointEntity]];
}

// Set the accumulated impulse for the upper limit constraint
RP3D_FORCE_INLINE void HingeJointComponents::setImpulseUpperLimit(Entity jointEntity, decimal impulseUpperLimit) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mImpulseUpperLimit[mMapEntityToComponentIndex[jointEntity]] = impulseUpperLimit;
}


// Return the accumulated impulse for the motor constraint;
RP3D_FORCE_INLINE decimal HingeJointComponents::getImpulseMotor(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mImpulseMotor[mMapEntityToComponentIndex[jointEntity]];
}

// Set the accumulated impulse for the motor constraint;
RP3D_FORCE_INLINE void HingeJointComponents::setImpulseMotor(Entity jointEntity, decimal impulseMotor) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mImpulseMotor[mMapEntityToComponentIndex[jointEntity]] = impulseMotor;
}

// Return the inverse of mass matrix K=JM^-1J^t for the limits and motor constraints (1x1 matrix)
RP3D_FORCE_INLINE decimal HingeJointComponents::getInverseMassMatrixLimitMotor(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mInverseMassMatrixLimitMotor[mMapEntityToComponentIndex[jointEntity]];
}

// Set the inverse of mass matrix K=JM^-1J^t for the limits and motor constraints (1x1 matrix)
RP3D_FORCE_INLINE void HingeJointComponents::setInverseMassMatrixLimitMotor(Entity jointEntity, decimal inverseMassMatrixLimitMotor) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mInverseMassMatrixLimitMotor[mMapEntityToComponentIndex[jointEntity]] = inverseMassMatrixLimitMotor;
}

// Return the inverse of mass matrix K=JM^-1J^t for the motor
RP3D_FORCE_INLINE decimal HingeJointComponents::getInverseMassMatrixMotor(Entity jointEntity) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mInverseMassMatrixMotor[mMapEntityToComponentIndex[jointEntity]];
}

// Return the inverse of mass matrix K=JM^-1J^t for the motor
RP3D_FORCE_INLINE void HingeJointComponents::setInverseMassMatrixMotor(Entity jointEntity, decimal inverseMassMatrixMotor) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mInverseMassMatrixMotor[mMapEntityToComponentIndex[jointEntity]] = inverseMassMatrixMotor;
}

// Return the bias of the lower limit constraint
RP3D_FORCE_INLINE decimal HingeJointComponents::getBLowerLimit(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mBLowerLimit[mMapEntityToComponentIndex[jointEntity]];
}

// Set the bias of the lower limit constraint
RP3D_FORCE_INLINE void HingeJointComponents::setBLowerLimit(Entity jointEntity, decimal bLowerLimit) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mBLowerLimit[mMapEntityToComponentIndex[jointEntity]] = bLowerLimit;
}

// Return the bias of the upper limit constraint
RP3D_FORCE_INLINE decimal HingeJointComponents::getBUpperLimit(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mBUpperLimit[mMapEntityToComponentIndex[jointEntity]];
}

// Set the bias of the upper limit constraint
RP3D_FORCE_INLINE void HingeJointComponents::setBUpperLimit(Entity jointEntity, decimal bUpperLimit) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mBUpperLimit[mMapEntityToComponentIndex[jointEntity]] = bUpperLimit;
}

// Return true if the joint limits are enabled
RP3D_FORCE_INLINE bool HingeJointComponents::getIsLimitEnabled(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mIsLimitEnabled[mMapEntityToComponentIndex[jointEntity]];
}

// Set to true if the joint limits are enabled
RP3D_FORCE_INLINE void HingeJointComponents::setIsLimitEnabled(Entity jointEntity, bool isLimitEnabled) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mIsLimitEnabled[mMapEntityToComponentIndex[jointEntity]] = isLimitEnabled;
}

// Return true if the motor of the joint in enabled
RP3D_FORCE_INLINE bool HingeJointComponents::getIsMotorEnabled(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mIsMotorEnabled[mMapEntityToComponentIndex[jointEntity]];
}

// Set to true if the motor of the joint in enabled
RP3D_FORCE_INLINE void HingeJointComponents::setIsMotorEnabled(Entity jointEntity, bool isMotorEnabled) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mIsMotorEnabled[mMapEntityToComponentIndex[jointEntity]] = isMotorEnabled;
}

// Return the Lower limit (minimum allowed rotation angle in radian)
RP3D_FORCE_INLINE decimal HingeJointComponents::getLowerLimit(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mLowerLimit[mMapEntityToComponentIndex[jointEntity]];
}

// Set the Lower limit (minimum allowed rotation angle in radian)
RP3D_FORCE_INLINE void HingeJointComponents::setLowerLimit(Entity jointEntity, decimal lowerLimit) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mLowerLimit[mMapEntityToComponentIndex[jointEntity]] = lowerLimit;
}

// Return the upper limit (maximum translation distance)
RP3D_FORCE_INLINE decimal HingeJointComponents::getUpperLimit(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mUpperLimit[mMapEntityToComponentIndex[jointEntity]];
}

// Set the upper limit (maximum translation distance)
RP3D_FORCE_INLINE void HingeJointComponents::setUpperLimit(Entity jointEntity, decimal upperLimit) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mUpperLimit[mMapEntityToComponentIndex[jointEntity]] = upperLimit;
}

// Return true if the lower limit is violated
RP3D_FORCE_INLINE bool HingeJointComponents::getIsLowerLimitViolated(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mIsLowerLimitViolated[mMapEntityToComponentIndex[jointEntity]];
}

// Set to true if the lower limit is violated
RP3D_FORCE_INLINE void HingeJointComponents::setIsLowerLimitViolated(Entity jointEntity, bool isLowerLimitViolated) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mIsLowerLimitViolated[mMapEntityToComponentIndex[jointEntity]] = isLowerLimitViolated;
}

// Return true if the upper limit is violated
RP3D_FORCE_INLINE bool HingeJointComponents::getIsUpperLimitViolated(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mIsUpperLimitViolated[mMapEntityToComponentIndex[jointEntity]];
}

// Set to true if the upper limit is violated
RP3D_FORCE_INLINE void HingeJointComponents::setIsUpperLimitViolated(Entity jointEntity, bool isUpperLimitViolated) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mIsUpperLimitViolated[mMapEntityToComponentIndex[jointEntity]] = isUpperLimitViolated;
}

// Return the motor speed (in rad/s)
RP3D_FORCE_INLINE decimal HingeJointComponents::getMotorSpeed(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mMotorSpeed[mMapEntityToComponentIndex[jointEntity]];
}

// Set the motor speed (in rad/s)
RP3D_FORCE_INLINE void HingeJointComponents::setMotorSpeed(Entity jointEntity, decimal motorSpeed) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mMotorSpeed[mMapEntityToComponentIndex[jointEntity]] = motorSpeed;
}

// Return the maximum motor torque (in Newtons) that can be applied to reach to desired motor speed
RP3D_FORCE_INLINE decimal HingeJointComponents::getMaxMotorTorque(Entity jointEntity) const {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    return mMaxMotorTorque[mMapEntityToComponentIndex[jointEntity]];
}

// Set the maximum motor torque (in Newtons) that can be applied to reach to desired motor speed
RP3D_FORCE_INLINE void HingeJointComponents::setMaxMotorTorque(Entity jointEntity, decimal maxMotorTorque) {

    assert(mMapEntityToComponentIndex.containsKey(jointEntity));
    mMaxMotorTorque[mMapEntityToComponentIndex[jointEntity]] = maxMotorTorque;
}

}

#endif
