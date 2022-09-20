// TODO(david): should normalize all data based on their access patterns
// #include "data_normalised_types.hpp"
#include "data_tags.hpp"

void MotionTransform(position *Positions, velocity *Velocities, acceleration *Accelerations, drag *Drags, u32 Size, r32 dt)
{
    for (u32 Index = 0;
         Index < Size;
         ++Index)
    {
        velocity PreviousVelocity = Velocities[Index];
        v2_r32 Acceleration = -Drags[Index].Amount * Velocities[Index].d + Accelerations[Index].dd;
        Velocities[Index].d += Acceleration * dt;
        Positions[Index].p += dt * (Velocities[Index].d + PreviousVelocity.d) / 2.0f;
    }
}
