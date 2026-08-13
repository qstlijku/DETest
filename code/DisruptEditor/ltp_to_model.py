# ltp_to_model.py
# Converts local-to-parent bone transforms to model-space transforms (FK).
#
# Input: populate the `bones` list with tuples parsed from an anim_pose*.h capture:
#   (name, parent, x, y, z, w, px, py, pz)
# Output: prints wdlAnimPose[] entries ready to paste into a header file.
#
# Quaternion format: x, y, z, w
# Position format:   px, py, pz (local-to-parent translation)

def qmul(q1, q2):
    x1,y1,z1,w1 = q1; x2,y2,z2,w2 = q2
    return (w1*x2+x1*w2+y1*z2-z1*y2,
            w1*y2-x1*z2+y1*w2+z1*x2,
            w1*z2+x1*y2-y1*x2+z1*w2,
            w1*w2-x1*x2-y1*y2-z1*z2)

def qrot(q, v):
    x,y,z,w = q; vx,vy,vz = v
    cx = y*vz - z*vy
    cy = z*vx - x*vz
    cz = x*vy - y*vx
    return (vx + 2*w*cx + 2*(y*cz - z*cy),
            vy + 2*w*cy + 2*(z*cx - x*cz),
            vz + 2*w*cz + 2*(x*cy - y*cx))

# Populate this list from your anim_pose*.h file.
# Format: (name, parent, x, y, z, w, px, py, pz)
bones = [
    # ("Empty_Hack", -1, 0.0, 0.0, 0.0, 1.0,  0.0, 0.0, 0.0),
    # ("Pelvis",      0,  ...,                  ...),
    # ...
]

worldQ = [None] * len(bones)
worldP = [None] * len(bones)

for i, b in enumerate(bones):
    name, parent, lx,ly,lz,lw, px,py,pz = b
    lq = (lx, ly, lz, lw)
    lp = (px, py, pz)
    if parent == -1:
        worldQ[i] = lq
        worldP[i] = lp
    else:
        worldQ[i] = qmul(worldQ[parent], lq)
        rotated = qrot(worldQ[parent], lp)
        worldP[i] = (worldP[parent][0] + rotated[0],
                     worldP[parent][1] + rotated[1],
                     worldP[parent][2] + rotated[2])

print("static SkelBone wdlAnimPose[] = {")
for i, b in enumerate(bones):
    name, parent = b[0], b[1]
    x, y, z, w = worldQ[i]
    px, py, pz = worldP[i]
    print(f'    {{ "{name}", {parent}, {x:f}f,{y:f}f,{z:f}f,{w:f}f, {px:f}f,{py:f}f,{pz:f}f }},')
print("};")
