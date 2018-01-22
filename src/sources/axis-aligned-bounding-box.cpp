/**
 *	@file		axis-aligned-bounding-box.cpp
 *	@author		Jonathan Hernandez (jmher019)
 *	@date		1/21/2018
 *	@version	1.0
 *
 *	@brief AxisAlignedBoundingBox derived class definition
 *
 *	@section DESCRIPTION
 *
 *	This file is intended to be an API for axis aligned bounding boxes.
 *
 *	The intended use case of this class is for simple rigid body collision
 */

#include <axis-aligned-bounding-box.hpp>

using namespace collision;

/**
 * @brief Default constructor
 */
AxisAlignedBoundingBox::AxisAlignedBoundingBox(const vec3& initialHalfExtents):
	initialHalfExtents(initialHalfExtents), halfExtents(initialHalfExtents) {}

/**
 * @brief Copy constructor
 */
AxisAlignedBoundingBox::AxisAlignedBoundingBox(const AxisAlignedBoundingBox& aabb): 
	initialHalfExtents(aabb.initialHalfExtents), halfExtents(aabb.halfExtents) {}

/**
 * @bried Move constructor
 */
AxisAlignedBoundingBox::AxisAlignedBoundingBox(AxisAlignedBoundingBox&& aabb) {
	initialHalfExtents = move(aabb.initialHalfExtents);
	halfExtents = move(aabb.halfExtents);
}

/**
 * @brief gets the current half extents
 *
 * @return the current half extents
 */
const vec3& AxisAlignedBoundingBox::getHalfExtents(void) const {
	return halfExtents;
}

/**
 * @brief overwrites the initial and current half extents
 *
 * @param halfExtents a vector that contains half the length in each dimension
 * @return void
 */
void AxisAlignedBoundingBox::setHalfExtents(const vec3& halfExtents) {
	initialHalfExtents = halfExtents;
	this->halfExtents = halfExtents;
}

/**
 * @brief computes the squared distance between a point and the box
 *
 * @param p the input point
 * @return the squared distance between the box and the point
 */
float AxisAlignedBoundingBox::getSquaredDistancePtPointAABB(const vec3& p) const {
	float sqDist = 0.f;

	const vec3 min = vec3(getCenter()) - halfExtents;
	const vec3 max = vec3(getCenter()) + halfExtents;

	for (int i = 0; i < 3; i++) {
		// For each axis count any excess distance outside box extents
		const float& v = p[i];
		if (v < min[i]) sqDist += (min[i] - v) * (min[i] - v);
		if (v > max[i]) sqDist += (v - max[i]) * (v - max[i]);
	}
	return sqDist;
}

/**
 * @brief handles updating transform matrix for this geometry
 *
 * @param transform will be used to update the geometry
 * @return void
 */
void AxisAlignedBoundingBox::update(const mat4& transform) {
	const mat3& upperLeft = mat3(transform);
	const vec4& center = transform[3];

	for (int i = 0; i < 3; i++) {
		halfExtents[i] = 0.f;
		for (int j = 0; j < 3; j++) {
			halfExtents[i] += fabs(upperLeft[j][i] * initialHalfExtents[j]);
		}
	}

	this->transform = mat4(upperLeft);
	this->transform[3] = center;
}

/**
 * @brief handles checking for intersection
 *
 * @param bv the bounding volume that will be checked against for intersection
 * @return bool that determines if the volumes are intersecting
 */
bool AxisAlignedBoundingBox::isIntersecting(BoundingVolume*& bv) const {
	// handle bounding sphere here
	if (const BoundingSphere* bSphere = dynamic_cast<BoundingSphere*>(bv)) {
		const vec3 sphereCenter = vec3(bSphere->getCenter());
		const float& radius = bSphere->getRadius();
		const float sqDist = getSquaredDistancePtPointAABB(sphereCenter);
		return sqDist <= radius * radius;
	}
	// handle axis aligned bounding box here
	else if (const AxisAlignedBoundingBox* bAABB = dynamic_cast<AxisAlignedBoundingBox*>(bv)) {
		const vec4& center = getCenter();
		const vec4& bCenter = bAABB->getCenter();
		const vec3& bHalfExents = bAABB->getHalfExtents();

		for (int i = 0; i < 3; i++) {
			if (fabs(center[i] - bCenter[i]) > (halfExtents[i] + bHalfExents[i])) {
				return false;
			}
		}

		return true;
	}

	// handle all others here
	BoundingVolume* self = (BoundingVolume*)this;
	return bv->isIntersecting(self);
}