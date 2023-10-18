## Create primitive: Group
<b>VDU 23, 30, 110, id; pid; flags; x; y;</b> : Create primitive: Group

This commmand creates a primitive that groups its child primitives,
for the purposes of motion and clipping. If a group node has
children, and the group node is moved, and the children are positioned
relative to their parent (i.e., do not use the PRIM_FLAG_ABSOLUTE flag),
then the children are moved with the parent. Note that a group node
has no visible representation (i.e., is not drawn).

Changing the flags of a group node can show or hide its children.

[Home](otf_mode.md)
