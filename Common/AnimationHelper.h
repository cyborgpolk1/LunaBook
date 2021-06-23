#pragma once

#include "d3dUtil.h"

///<summary>
/// A Keyframe defines the bone transformation at an instant in time.
///</summary>
struct Keyframe
{
    Keyframe();
    ~Keyframe();

    float TimePos;
    XMFLOAT3 Translation;
    XMFLOAT3 Scale;
    XMFLOAT4 RotationQuat;
};

///<summary>
/// A BoneAnimation is defined by a list of keyframes. For time
/// values inbetween two keyframes, we interpolate between the
/// two nearest keyframes that bound the tile.
///
/// WE assume an animation always has two keyframes.
///</summary>
struct BoneAnimation
{
    float GetStartTime() const;
    float GetEndTime() const;

    void Interpolate(float t, XMFLOAT4X4& M) const;

    std::vector<Keyframe> Keyframes;
};