#ifndef DATA_TRANSFORMS_HPP
# define DATA_TRANSFORMS_HPP

// TODO(david): maybe better name is transient_data.hpp

// TODO(david): after implementation of the tree, think about the access patterns and redesign these as this is just an initial
// presumption.. another way to structure it is to put everything in the node_mesh.. another is to put number of meshes in node_mesh_container
// as its more closely related to it, but if they are also accessed whenever a node_mesh is accessed, then it doesn't make sense
// to separate them

struct node_mesh // 4B
{
    i16 Index;     // if leaf -> index to node_mesh_container, if internal -> index to first node_mesh child
    i16 NOfMeshes; // if 0 >= -> leaf node,                    if -1       -> internal node
    // TODO(david): it might be worth it to store node's AABB, considering how small a node is, instead of always recomputing it on the go
    // that would add an extra 16B + padding(?)
};
static_assert(sizeof(node_mesh) % 4 == 0);

struct node_mesh_container
{
    u16 MeshForeignKey[8];
};
static_assert(sizeof(node_mesh_container) % 16 == 0);

#endif
