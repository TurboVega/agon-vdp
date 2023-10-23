## Create primitive: Render 3D Scene
<b>VDU 23, 30, 200, id; pid; flags; x; y; w; h;</b> :  Create primitive: Render 3D Scene

This command creates a primitive that renders a 3D scene to a bitmap. The bitmap can be
manipulated or altered using the bitmap related commands, after the scene image
has been rendered. A scene may contain multiple objects.

## Create Mesh
<b>VDU 23, 30, 201, id; mid; n; x1; y1; z1; ... xn; yn; zn;</b> :  Create Mesh

## Set Mesh Vertex Indices
<b>VDU 23, 30, 202, id; mid; n; i1; ... in;</b> :  Set Mesh Vertex Indices

## Create Texture
<b>VDU 23, 30, 203, id; tid; n; u1; v1; ... un; vn;</b> :  Create Texture

## Set Texture Coordinate Indices
<b>VDU 23, 30, 204, id; mid; tid; n; i1; ... in;</b> :  Set Texture Coordinate Indices

## Create Object
<b>VDU 23, 30, 205, id; oid; mid; tid; bmid;</b> :  Create Object

## Set Object X Scale Factor
<b>VDU 23, 30, 206, id; oid; scalex;</b> :  Set Object X Scale Factor

## Set Object Y Scale Factor
<b>VDU 23, 30, 207, id; oid; scaley;</b> :  Set Object Y Scale Factor

## Set Object Z Scale Factor
<b>VDU 23, 30, 208, id; oid; scalez;</b> :  Set Object Z Scale Factor

## Set Object XYZ Scale Factors
<b>VDU 23, 30, 209, id; oid; scalex; scaley; scalez</b> :  Set Object XYZ Scale Factors

## Set Object X Rotation Angle
<b>VDU 23, 30, 210, id; oid; anglex;</b> :  Set Object X Rotation Angle

## Set Object Y Rotation Angle
<b>VDU 23, 30, 211, id; oid; angley;</b> :  Set Object Y Rotation Angle

## Set Object Z Rotation Angle
<b>VDU 23, 30, 212, id; oid; anglez;</b> :  Set Object Z Rotation Angle

## Set Object XYZ Rotation Angles
<b>VDU 23, 30, 213, id; oid; anglex; angley; anglez;</b> :  Set Object XYZ Rotation Angles

## Set Object X Translation Distance
<b>VDU 23, 30, 214, id; oid; distx;</b> :  Set Object X Translation Distance

## Set Object Y Translation Distance
<b>VDU 23, 30, 215, id; oid; disty;</b> :  Set Object Y Translation Distance

## Set Object Z Translation Distance
<b>VDU 23, 30, 216, id; oid; distz;</b> :  Set Object Z Translation Distance

## Set Object XYZ Translation Distances
<b>VDU 23, 30, 217, id; oid; distx; disty; distz</b> :  Set Object XYZ Translation Distances

## Render To Bitmap
<b>VDU 23, 30, 218, id;</b> :  Render To Bitmap

The following image illustrates the concept, but the actual appearance will differ on the Agon, because this image was created on a PC.

![Render](render.png)


[Bitmap](otf_bitmap.md)
[Home](otf_mode.md)
