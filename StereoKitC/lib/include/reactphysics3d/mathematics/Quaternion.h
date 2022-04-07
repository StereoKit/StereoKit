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

#ifndef REACTPHYSICS3D_QUATERNION_H
#define REACTPHYSICS3D_QUATERNION_H

// Libraries
#include <reactphysics3d/decimal.h>
#include <reactphysics3d/mathematics/Vector3.h>

/// ReactPhysics3D namespace
namespace reactphysics3d {

// Declarations
class Matrix3x3;

// Class Quaternion
/**
 * This class represents a quaternion. We use the notation :
 * q = (x*i, y*j, z*k, w) to represent a quaternion.
 */
struct Quaternion {

    public :

        // -------------------- Attributes -------------------- //

        /// Component x
        decimal x;

        /// Component y
        decimal y;

        /// Component z
        decimal z;

        /// Component w
        decimal w;

        // -------------------- Methods -------------------- //

        /// Constructor
        Quaternion();

        /// Constructor with arguments
        Quaternion(decimal newX, decimal newY, decimal newZ, decimal newW);

        /// Constructor with the component w and the vector v=(x y z)
        Quaternion(decimal newW, const Vector3& v);

        /// Constructor with the component w and the vector v=(x y z)
        Quaternion(const Vector3& v, decimal newW);

        /// Create a unit quaternion from a rotation matrix
        Quaternion(const Matrix3x3& matrix);

        /// Set all the values
        void setAllValues(decimal newX, decimal newY, decimal newZ, decimal newW);

        /// Set the quaternion to zero
        void setToZero();

        /// Set to the identity quaternion
        void setToIdentity();

        /// Return the vector v=(x y z) of the quaternion
        Vector3 getVectorV() const;

        /// Return the length of the quaternion
        decimal length() const;

        /// Return the square of the length of the quaternion
        decimal lengthSquare() const;

        /// Normalize the quaternion
        void normalize();

        /// Inverse the quaternion
        void inverse();

        /// Return the unit quaternion
        Quaternion getUnit() const;

        /// Return the conjugate quaternion
        Quaternion getConjugate() const;

        /// Return the inverse of the quaternion
        Quaternion getInverse() const;

        /// Return the orientation matrix corresponding to this quaternion
        Matrix3x3 getMatrix() const;

        /// Return the identity quaternion
        static Quaternion identity();

        /// Return a quaternion constructed from Euler angles (in radians)
        static Quaternion fromEulerAngles(decimal angleX, decimal angleY, decimal angleZ);

        /// Return a quaternion constructed from Euler angles (in radians)
        static Quaternion fromEulerAngles(const Vector3& eulerAngles);

        /// Dot product between two quaternions
        decimal dot(const Quaternion& quaternion) const;

        /// Compute the rotation angle (in radians) and the rotation axis
        void getRotationAngleAxis(decimal& angle, Vector3& axis) const;

        /// Return true if the values are not NAN OR INF
        bool isFinite() const;

        /// Return true if it is a unit quaternion
        bool isUnit() const;

        /// Return true if it is a valid quaternion
        bool isValid() const;

        /// Compute the spherical linear interpolation between two quaternions
        static Quaternion slerp(const Quaternion& quaternion1, const Quaternion& quaternion2, decimal t);

        /// Overloaded operator for the addition
        Quaternion operator+(const Quaternion& quaternion) const;

        /// Overloaded operator for the substraction
        Quaternion operator-(const Quaternion& quaternion) const;

        /// Overloaded operator for addition with assignment
        Quaternion& operator+=(const Quaternion& quaternion);

        /// Overloaded operator for substraction with assignment
        Quaternion& operator-=(const Quaternion& quaternion);

        /// Overloaded operator for the multiplication with a constant
        Quaternion operator*(decimal nb) const;

        /// Overloaded operator for the multiplication
        Quaternion operator*(const Quaternion& quaternion) const;

        /// Overloaded operator for the multiplication with a vector
        Vector3 operator*(const Vector3& point) const;

        /// Overloaded operator for equality condition
        bool operator==(const Quaternion& quaternion) const;

        /// Return the string representation
        std::string to_string() const;

    private:

        /// Initialize the quaternion using Euler angles
        void initWithEulerAngles(decimal angleX, decimal angleY, decimal angleZ);
};

// Constructor of the class
RP3D_FORCE_INLINE Quaternion::Quaternion() : x(0.0), y(0.0), z(0.0), w(0.0) {

}

// Constructor with arguments
RP3D_FORCE_INLINE Quaternion::Quaternion(decimal newX, decimal newY, decimal newZ, decimal newW)
           :x(newX), y(newY), z(newZ), w(newW) {

}

// Constructor with the component w and the vector v=(x y z)
RP3D_FORCE_INLINE Quaternion::Quaternion(decimal newW, const Vector3& v) : x(v.x), y(v.y), z(v.z), w(newW) {

}

// Constructor with the component w and the vector v=(x y z)
RP3D_FORCE_INLINE Quaternion::Quaternion(const Vector3& v, decimal newW) : x(v.x), y(v.y), z(v.z), w(newW) {

}

// Set all the values
RP3D_FORCE_INLINE void Quaternion::setAllValues(decimal newX, decimal newY, decimal newZ, decimal newW) {
    x = newX;
    y = newY;
    z = newZ;
    w = newW;
}

// Set the quaternion to zero
RP3D_FORCE_INLINE void Quaternion::setToZero() {
    x = 0;
    y = 0;
    z = 0;
    w = 0;
}

// Set to the identity quaternion
RP3D_FORCE_INLINE void Quaternion::setToIdentity() {
    x = 0;
    y = 0;
    z = 0;
    w = 1;
}

// Return the vector v=(x y z) of the quaternion
RP3D_FORCE_INLINE Vector3 Quaternion::getVectorV() const {

    // Return the vector v
    return Vector3(x, y, z);
}

// Return the length of the quaternion (RP3D_FORCE_INLINE)
RP3D_FORCE_INLINE decimal Quaternion::length() const {
    return std::sqrt(x*x + y*y + z*z + w*w);
}

// Return the square of the length of the quaternion
RP3D_FORCE_INLINE decimal Quaternion::lengthSquare() const {
    return x*x + y*y + z*z + w*w;
}

// Normalize the quaternion
RP3D_FORCE_INLINE void Quaternion::normalize() {

    decimal l = length();

    // Check if the length is not equal to zero
    assert (l > MACHINE_EPSILON);

    x /= l;
    y /= l;
    z /= l;
    w /= l;
}

// Inverse the quaternion
RP3D_FORCE_INLINE void Quaternion::inverse() {

    // Use the conjugate of the current quaternion
    x = -x;
    y = -y;
    z = -z;
}

// Return the unit quaternion
RP3D_FORCE_INLINE Quaternion Quaternion::getUnit() const {
    decimal lengthQuaternion = length();

    // Check if the length is not equal to zero
    assert (lengthQuaternion > MACHINE_EPSILON);

    // Compute and return the unit quaternion
    return Quaternion(x / lengthQuaternion, y / lengthQuaternion,
                      z / lengthQuaternion, w / lengthQuaternion);
}

// Return the identity quaternion
RP3D_FORCE_INLINE Quaternion Quaternion::identity() {
    return Quaternion(0.0, 0.0, 0.0, 1.0);
}

// Return the conjugate of the quaternion (RP3D_FORCE_INLINE)
RP3D_FORCE_INLINE Quaternion Quaternion::getConjugate() const {
    return Quaternion(-x, -y, -z, w);
}

// Return the inverse of the quaternion (RP3D_FORCE_INLINE)
RP3D_FORCE_INLINE Quaternion Quaternion::getInverse() const {

    // Return the conjugate quaternion
    return Quaternion(-x, -y, -z, w);
}

// Scalar product between two quaternions
RP3D_FORCE_INLINE decimal Quaternion::dot(const Quaternion& quaternion) const {
    return (x*quaternion.x + y*quaternion.y + z*quaternion.z + w*quaternion.w);
}

// Return true if the values are not NAN OR INF
RP3D_FORCE_INLINE bool Quaternion::isFinite() const {
    return std::isfinite(x) && std::isfinite(y) && std::isfinite(z) && std::isfinite(w);
}

// Return true if it is a unit quaternion
RP3D_FORCE_INLINE bool Quaternion::isUnit() const {
    const decimal length = std::sqrt(x*x + y*y + z*z + w*w);
    const decimal tolerance = 1e-5f;
    return std::abs(length - decimal(1.0)) < tolerance;
}

// Return true if it is a valid quaternion
RP3D_FORCE_INLINE bool Quaternion::isValid() const {
   return isFinite() && isUnit();
}

// Overloaded operator for the addition of two quaternions
RP3D_FORCE_INLINE Quaternion Quaternion::operator+(const Quaternion& quaternion) const {

    // Return the result quaternion
    return Quaternion(x + quaternion.x, y + quaternion.y, z + quaternion.z, w + quaternion.w);
}

// Overloaded operator for the substraction of two quaternions
RP3D_FORCE_INLINE Quaternion Quaternion::operator-(const Quaternion& quaternion) const {

    // Return the result of the substraction
    return Quaternion(x - quaternion.x, y - quaternion.y, z - quaternion.z, w - quaternion.w);
}

// Overloaded operator for addition with assignment
RP3D_FORCE_INLINE Quaternion& Quaternion::operator+=(const Quaternion& quaternion) {
    x += quaternion.x;
    y += quaternion.y;
    z += quaternion.z;
    w += quaternion.w;
    return *this;
}

// Overloaded operator for substraction with assignment
RP3D_FORCE_INLINE Quaternion& Quaternion::operator-=(const Quaternion& quaternion) {
    x -= quaternion.x;
    y -= quaternion.y;
    z -= quaternion.z;
    w -= quaternion.w;
    return *this;
}

// Overloaded operator for the multiplication with a constant
RP3D_FORCE_INLINE Quaternion Quaternion::operator*(decimal nb) const {
    return Quaternion(nb * x, nb * y, nb * z, nb * w);
}

// Overloaded operator for the multiplication of two quaternions
RP3D_FORCE_INLINE Quaternion Quaternion::operator*(const Quaternion& quaternion) const {

    /* The followin code is equivalent to this
    return Quaternion(w * quaternion.w - getVectorV().dot(quaternion.getVectorV()),
                          w * quaternion.getVectorV() + quaternion.w * getVectorV() +
                          getVectorV().cross(quaternion.getVectorV()));
    */

    return Quaternion(w * quaternion.x + quaternion.w * x + y * quaternion.z - z * quaternion.y,
                      w * quaternion.y + quaternion.w * y + z * quaternion.x - x * quaternion.z,
                      w * quaternion.z + quaternion.w * z + x * quaternion.y - y * quaternion.x,
                      w * quaternion.w - x * quaternion.x - y * quaternion.y - z * quaternion.z);
}

// Overloaded operator for the multiplication with a vector.
/// This methods rotates a point given the rotation of a quaternion.
RP3D_FORCE_INLINE Vector3 Quaternion::operator*(const Vector3& point) const {

    /* The following code is equivalent to this
     * Quaternion p(point.x, point.y, point.z, 0.0);
     * return (((*this) * p) * getConjugate()).getVectorV();
    */

    const decimal prodX = w * point.x + y * point.z - z * point.y;
    const decimal prodY = w * point.y + z * point.x - x * point.z;
    const decimal prodZ = w * point.z + x * point.y - y * point.x;
    const decimal prodW = -x * point.x - y * point.y - z * point.z;
    return Vector3(w * prodX - prodY * z + prodZ * y - prodW * x,
                   w * prodY - prodZ * x + prodX * z - prodW * y,
                   w * prodZ - prodX * y + prodY * x - prodW * z);
}

// Overloaded operator for equality condition
RP3D_FORCE_INLINE bool Quaternion::operator==(const Quaternion& quaternion) const {
    return (x == quaternion.x && y == quaternion.y &&
            z == quaternion.z && w == quaternion.w);
}

// Get the string representation
RP3D_FORCE_INLINE std::string Quaternion::to_string() const {
    return "Quaternion(" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z) + "," +
            std::to_string(w) + ")";
}

}

#endif
