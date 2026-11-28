"""Replace the Meshy judge's fused hands with modelled hands and 15 finger bones per hand (D-86).

Called by prepare_juge.py after the seam weld, before export. Everything is built from numbers
(box-modelled tubes with edge loops at each joint), so the result is reproducible.

Hand-local frame per side: u along the fingers (0 at the cuff opening), v towards the thumb,
w towards the back of the hand. The judge's T-pose has palms down and thumbs forward (-Y).
"""
import bpy, bmesh, math
import numpy as np
from mathutils import Vector, Matrix, Quaternion

# name, v of the knuckle centre, spread (deg, + towards the thumb), length, radius, knuckle u shift
FINGERS = [
    ('Index',  0.0300,  5.0, 0.104, 0.0090, -0.004),
    ('Middle', 0.0100,  1.0, 0.114, 0.0092,  0.000),
    ('Ring',  -0.0100, -3.5, 0.106, 0.0088, -0.003),
    ('Pinky', -0.0280, -9.0, 0.086, 0.0078, -0.012),
]
PALM_U = [-0.035, -0.015, 0.0, 0.014, 0.030, 0.046, 0.063, 0.080, 0.097, 0.112, 0.126]
KNUCKLE = len(PALM_U) - 1
THUMB_RING = 4                     # thumb patch spans palm rings 4..7 on the thumb side
COLS = 15                          # 4 fingers x 3 columns + 3 web columns
# Decagon angles for the 10-point boundary of a 3x2 grid patch, in boundary order.
THETA = [324, 288, 252, 216, 180, 144, 108, 72, 36, 0]
PATCH = [(0, 0), (1, 0), (2, 0), (3, 0), (3, 1), (3, 2), (2, 2), (1, 2), (0, 2), (0, 1)]
SKIN_MIN_RED, SKIN_MIN_RED_MINUS_BLUE = 0.45, 0.10


def smoothstep(a, b, x):
    t = min(1.0, max(0.0, (x - a) / (b - a)))
    return t * t * (3 - 2 * t)


class Builder:
    def __init__(self):
        self.co, self.wt, self.faces, self.keys = [], [], [], {}

    def add(self, key, p, weights):
        self.keys[key] = len(self.co)
        self.co.append(Vector(p))
        self.wt.append({k: v for k, v in weights.items() if v > 1e-4})
        return self.keys[key]

    def quad(self, *keys):
        self.faces.append([self.keys[k] for k in keys])


def palm_shape(u):
    """Half width (v) and half thickness (w, back / palm side) of the palm at u."""
    w = np.interp(u, [-0.035, 0.0, 0.014, 0.030, 0.063, 0.097, 0.126],
                  [0.0225, 0.0240, 0.0255, 0.0300, 0.0375, 0.0385, 0.0385])
    h = np.interp(u, [-0.035, 0.0, 0.030, 0.063, 0.097, 0.126],
                  [0.0165, 0.0170, 0.0165, 0.0150, 0.0125, 0.0095])
    return float(w), float(h)


def build_hand(side):
    b = Builder()
    hand, fa = side + 'Hand', side + 'ForeArm'
    fingers = []
    for f, (name, vc, spread, length, radius, du) in enumerate(FINGERS):
        a = math.radians(spread)
        fingers.append(dict(name=name, start=Vector((PALM_U[KNUCKLE] + du, vc, 0.0)),
                            dir=Vector((math.cos(a), math.sin(a), 0.0)),
                            e1=Vector((-math.sin(a), math.cos(a), 0.0)), e2=Vector((0, 0, 1)),
                            L=length, r=radius))

    def finger_point(fd, m, s, scale=1.0):
        th = math.radians(THETA[m])
        r = fd['r'] * scale
        return fd['start'] + fd['dir'] * s + fd['e1'] * (r * math.cos(th)) + fd['e2'] * (0.92 * r * math.sin(th))

    # Knuckle ring points = first ring of each finger (on the palm end).
    knuckle_pos = {}
    for f, fd in enumerate(fingers):
        for m in range(10):
            p = finger_point(fd, m, 0.0)
            if (m == 4 and f < 3) or (m == 9 and f > 0):
                p.x -= 0.004    # web dip between fingers
            knuckle_pos[(f, m)] = p

    def grid_to_finger(i, j):
        f, c = min(i // 4, 3), i % 4 if i < 16 else 3
        if j == 0: m = c
        elif j == 2: m = 8 - c
        else: m = 9 if c == 0 else 4
        return f, m

    loop = [(i, 0) for i in range(COLS + 1)] + [(COLS, 1)] + [(i, 2) for i in range(COLS, -1, -1)] + [(0, 1)]
    kx = [p for p in knuckle_pos.values()]
    kw = max(abs(p.y) for p in kx); kh = max(abs(p.z) for p in kx)

    # Thumb base centre: middle of the thumb patch on the palm side (computed after palm rings).
    def finger_weights_near_knuckle(u, v):
        f = min(range(4), key=lambda k: abs(fingers[k]['start'].y - v))
        return f, 0.5 * smoothstep(PALM_U[KNUCKLE] - 0.028, PALM_U[KNUCKLE] - 0.002, u)

    def palm_weights(p):
        u = p.x
        wf = 1 - smoothstep(-0.026, 0.006, u)
        f, wfin = finger_weights_near_knuckle(u, p.y)
        wts = {fa: wf, hand: (1 - wf) * (1 - wfin)}
        wts[side + 'Hand' + fingers[f]['name'] + '1'] = (1 - wf) * wfin
        return wts

    # Palm rings (thumb side points inside the thumb patch are skipped).
    for k, u in enumerate(PALM_U[:KNUCKLE]):
        hw, hh = palm_shape(u)
        blend = smoothstep(0.62, 1.0, (u - 0.03) / (PALM_U[KNUCKLE] - 0.03))
        for (i, j) in loop:
            if i == 0 and j == 1 and THUMB_RING < k < THUMB_RING + 3:
                continue
            kp = knuckle_pos[grid_to_finger(i, j)]
            # Rounded-rectangle section, monotonic across the width (follows the knuckle order).
            x = max(-1.0, min(1.0, kp.y / kw))
            pv = hw * x
            pw = 0.0 if j == 1 else (1 - abs(x) ** 4) ** 0.25 * (hh if j == 2 else -1.12 * hh)
            p = Vector((u, pv, pw)).lerp(Vector((u, kp.y, kp.z)), blend)
            b.add(('P', k, i, j), p, {})
    for (i, j) in loop:
        f, m = grid_to_finger(i, j)
        b.keys[('P', KNUCKLE, i, j)] = None  # placeholder, filled by finger first rings
    # Finger first rings (shared with the palm end grid).
    for f, fd in enumerate(fingers):
        for m in range(10):
            b.add(('F', f, 0, m), knuckle_pos[(f, m)], {})
    for (i, j) in loop:
        f, m = grid_to_finger(i, j)
        b.keys[('P', KNUCKLE, i, j)] = b.keys[('F', f, 0, m)]
    for i in range(1, COLS):
        f, m = grid_to_finger(i, 1)
        if m in (4, 9):
            b.keys[('P', KNUCKLE, i, 1)] = b.keys[('F', f, 0, m)]

    # Palm weights (after positions are known).
    for key, idx in list(b.keys.items()):
        if key[0] == 'P' and key[1] < KNUCKLE:
            b.wt[idx] = {k: v for k, v in palm_weights(b.co[idx]).items() if v > 1e-4}

    # Palm side faces between consecutive rings.
    n = len(loop)
    for k in range(KNUCKLE):
        for a in range(n):
            pa, pb = loop[a], loop[(a + 1) % n]
            if THUMB_RING <= k < THUMB_RING + 3 and pa[0] == 0 and pb[0] == 0:
                continue
            b.quad(('P', k, *pa), ('P', k, *pb), ('P', k + 1, *pb), ('P', k + 1, *pa))
    # Wrist cap (inside the sleeve) as a 15x2 grid.
    hw, hh = palm_shape(PALM_U[0])
    for i in range(1, COLS):
        p = (b.co[b.keys[('P', 0, i, 0)]] + b.co[b.keys[('P', 0, i, 2)]]) / 2
        b.add(('P', 0, i, 1), p - Vector((0.004, 0, 0)), {fa: 1.0})
    for i in range(COLS):
        for j in range(2):
            b.quad(('P', 0, i, j), ('P', 0, i + 1, j), ('P', 0, i + 1, j + 1), ('P', 0, i, j + 1))
    # Webs between fingers at the palm end.
    for g in range(3):
        b.quad(('F', g, 0, 3), ('F', g + 1, 0, 0), ('F', g + 1, 0, 9), ('F', g, 0, 4))
        b.quad(('F', g, 0, 4), ('F', g + 1, 0, 9), ('F', g + 1, 0, 8), ('F', g, 0, 5))

    bones = {}

    def tube(prefix, base_keys, start, d, e1, e2, stations, joints, names, wbase, aspect=0.92):
        """Rings along a tube; base ring given by keys. stations: list of (s, radius)."""
        prev = base_keys
        for si, (s, r) in enumerate(stations):
            keys = []
            for m in range(10):
                th = math.radians(THETA[m])
                p = start + d * s + e1 * (r * math.cos(th)) + e2 * (aspect * r * math.sin(th))
                key = (prefix, si + 1, m)
                b.add(key, p, tube_weights(s, joints, names, wbase))
                keys.append(key)
            for m in range(10):
                m2 = (m + 1) % 10
                b.quad(prev[m], prev[m2], keys[m2], keys[m])
            prev = keys
        # Rounded tip: 3x2 grid cap closing the last ring.
        s_tip, r_last = stations[-1][0] + stations[-1][1] * 0.9, stations[-1][1]
        cap = {}
        for m, (c, j) in enumerate(PATCH):
            cap[(c, j)] = prev[m]
        for c, off in ((1, 0.33), (2, -0.33)):
            key = (prefix, 'tip', c)
            b.add(key, start + d * s_tip + e1 * (off * r_last), tube_weights(s_tip, joints, names, wbase))
            cap[(c, 1)] = key
        for c in range(3):
            for j in range(2):
                b.quad(cap[(c, j)], cap[(c + 1, j)], cap[(c + 1, j + 1)], cap[(c, j + 1)])

    def tube_weights(s, joints, names, wbase):
        j2, j3 = joints
        wh = wbase * (1 - smoothstep(0.0, 0.012, s))
        rest = 1 - wh
        t2 = smoothstep(j2 - 0.007, j2 + 0.007, s)
        t3 = smoothstep(j3 - 0.006, j3 + 0.006, s)
        w = {names[0]: rest * (1 - t2), names[1]: rest * t2 * (1 - t3), names[2]: rest * t3}
        w[wbase_bone[0]] = w.get(wbase_bone[0], 0) + wh
        return w

    wbase_bone = [hand]
    for f, fd in enumerate(fingers):
        L, r = fd['L'], fd['r']
        j2, j3 = 0.45 * L, 0.75 * L
        st = [(0.010, r * 1.02), (0.24 * L, r * 0.97), (j2 - 0.008, r * 0.93), (j2, r * 0.96),
              (j2 + 0.008, r * 0.92), (j3 - 0.007, r * 0.86), (j3, r * 0.88), (j3 + 0.007, r * 0.84),
              (L - 0.016, r * 0.80), (L - 0.008, r * 0.70), (L - 0.0035, r * 0.45)]
        names = [f"{side}Hand{fd['name']}{k}" for k in (1, 2, 3)]
        # Re-weight the shared first ring: half hand, half proximal.
        for m in range(10):
            b.wt[b.keys[('F', f, 0, m)]] = {hand: 0.5, names[0]: 0.5}
        tube(('F', f), [('F', f, 0, m) for m in range(10)], fd['start'], fd['dir'], fd['e1'], fd['e2'],
             st, (j2, j3), names, 0.5)
        mcp = fd['start'] - fd['dir'] * 0.007
        bones[names[0]] = (mcp, fd['start'] + fd['dir'] * j2, fd['e2'], hand)
        bones[names[1]] = (fd['start'] + fd['dir'] * j2, fd['start'] + fd['dir'] * j3, fd['e2'], names[0])
        bones[names[2]] = (fd['start'] + fd['dir'] * j3, fd['start'] + fd['dir'] * (L - 0.004), fd['e2'], names[1])

    # Thumb from the patch on the thumb side of palm rings 4..7.
    tk = THUMB_RING
    base = [('P', tk + c, 0, j) for (c, j) in PATCH]
    bcentre = sum((b.co[b.keys[k]] for k in base), Vector()) / 10
    t = Vector((0.55, 0.78, -0.30)).normalized()
    e2 = (Vector((0, 0, 1)) - t * t.z).normalized()
    e1 = -(Vector((1, 0, 0)) - t * t.x - e2 * e2.x).normalized()
    start = bcentre - t * 0.004
    tn = [f'{side}HandThumb{k}' for k in (1, 2, 3)]
    j2, j3, L = 0.030, 0.058, 0.086
    st = [(0.010, 0.0128), (0.020, 0.0122), (j2 - 0.007, 0.0112), (j2, 0.0114), (j2 + 0.007, 0.0106),
          (j3 - 0.007, 0.0098), (j3, 0.0100), (j3 + 0.007, 0.0094), (L - 0.016, 0.0088), (L - 0.008, 0.0078), (L - 0.0035, 0.0050)]
    for k in base:
        b.wt[b.keys[k]] = {hand: 0.45, tn[0]: 0.55}
    tube('T', base, start, t, e1, e2, st, (j2, j3), tn, 0.45, aspect=0.85)
    cmc = start - t * 0.026 + Vector((0, -0.006, -0.002))
    bones[tn[0]] = (cmc, start + t * j2, e2, hand)
    bones[tn[1]] = (start + t * j2, start + t * j3, e2, tn[0])
    bones[tn[2]] = (start + t * j3, start + t * (L - 0.004), e2, tn[1])
    # Thenar: palm points near the thumb base follow the metacarpal a little.
    for key, idx in b.keys.items():
        if key is None or idx is None or key[0] != 'P' or key[1] >= KNUCKLE:
            continue
        p = b.co[idx]
        if p.y < 0:
            continue
        d = (p - bcentre).length
        th = 0.45 * (1 - smoothstep(0.010, 0.032, d))
        if th > 0:
            wts = {k: v * (1 - th) for k, v in b.wt[idx].items()}
            wts[tn[0]] = wts.get(tn[0], 0) + th
            b.wt[idx] = wts
    return b, bones


def frame(rig, mesh_opening_centre, side):
    """Local hand frame (origin at the cuff opening) -> world matrix."""
    fa = rig.data.bones[side + 'ForeArm']
    d = (fa.tail_local - fa.head_local).normalized()
    w = (Vector((0, 0, 1)) - d * d.z).normalized()
    v = w.cross(d) if side == 'Right' else d.cross(w)
    if v.y > 0: v = -v
    R = Matrix((d, v, w)).transposed()
    return mesh_opening_centre, R


def replace_hands(rig, mesh, image, report):
    """Delete the Meshy hand skin, add modelled hands + finger bones. Returns new-face normals info."""
    me = mesh.data
    W, H = image.size
    px = np.empty(W * H * 4, dtype=np.float32)
    image.pixels.foreach_get(px)
    px = px.reshape(H, W, 4)[:, :, :3]

    bm = bmesh.new(); bm.from_mesh(me)
    uvl = bm.loops.layers.uv.active
    dw = bm.verts.layers.deform.active

    def fuv(f):
        return sum((l[uvl].uv for l in f.loops), Vector((0, 0))) / len(f.loops)

    def colour(u):
        return px[int(u.y % 1 * H) % H, int(u.x % 1 * W) % W]

    def is_skin(c):
        return c[0] > SKIN_MIN_RED and c[0] - c[2] > SKIN_MIN_RED_MINUS_BLUE

    # A uniform skin texel (from the back of the old hands) and a dark sleeve texel for the cuff caps.
    def window_stats(u, r):
        x, y = int(u.x % 1 * W), int(u.y % 1 * H)
        win = px[max(0, y - r):y + r + 1, max(0, x - r):x + r + 1].reshape(-1, 3)
        return win.mean(0), win.std(0).sum()

    skin_uv = dark_uv = None
    best_skin = best_dark = 1e9
    hand_faces = {'Left': [], 'Right': []}
    dorsal = []
    for f in bm.faces:
        c = f.calc_center_median()
        if abs(c.x) <= 0.6:
            continue
        sidename = 'Left' if c.x > 0 else 'Right'
        u = fuv(f)
        if is_skin(colour(u)):
            hand_faces[sidename].append(f)
            if f.normal.z > 0.7:
                dorsal.append(u)
    skin_ref = np.median(np.array([colour(u) for u in dorsal]), 0)
    for u in dorsal:
        mean, sd = window_stats(u, 10)
        score = sd + 4 * np.abs(mean - skin_ref).sum()
        if score < best_skin and is_skin(mean):
            best_skin, skin_uv = score, u.copy()
    for f in bm.faces:
        c = f.calc_center_median()
        if 0.45 < abs(c.x) < 0.6:
            u = fuv(f)
            mean, sd = window_stats(u, 10)
            if mean.sum() < 0.15 and sd < best_dark:
                best_dark, dark_uv = sd, u.copy()
    report['hand_skin_uv'] = list(skin_uv); report['hand_skin_rgb'] = [round(float(x), 3) for x in colour(skin_uv)]
    report['cuff_cap_uv'] = list(dark_uv)

    # Delete the old hands, cap each cuff opening with a dark fan (inside the sleeve).
    openings = {}
    for side, faces in hand_faces.items():
        fs = set(faces)
        ring = [e for f in faces for e in f.edges if any(g not in fs for g in e.link_faces)]
        ring = list(dict.fromkeys(ring))
        verts = list({v for e in ring for v in e.verts})
        centre = sum((v.co for v in verts), Vector()) / len(verts)
        openings[side] = (centre, ring)
        report.setdefault('removed_hand_faces', {})[side] = len(faces)
    for side, faces in hand_faces.items():
        bmesh.ops.delete(bm, geom=faces, context='FACES')
    flag = bm.faces.layers.int.new('dub_flat')
    uvl = bm.loops.layers.uv.active
    dw = bm.verts.layers.deform.active
    for side, (centre, _) in openings.items():
        bnd = [e for e in bm.edges if e.is_boundary and (e.verts[0].co - centre).length < 0.05
               and (e.verts[1].co - centre).length < 0.05]
        # order the boundary loop
        vs = {v for e in bnd for v in e.verts}
        start = next(iter(vs)); order = [start]; prev = None
        while True:
            cur = order[-1]
            nxt = [e.other_vert(cur) for e in cur.link_edges if e in bnd and e.other_vert(cur) is not prev]
            nxt = [v for v in nxt if v is not cur]
            if not nxt or nxt[0] is start: break
            prev = cur; order.append(nxt[0])
        assert len(order) == len(vs), (side, len(order), len(vs))
        # Shallow cone receding into the sleeve, so it never lies in the plane of the folded cuff lip.
        inward = Vector((-1 if side == 'Left' else 1, 0, 0))
        cv = bm.verts.new(centre + inward * 0.02)
        acc = {}
        for v in order:
            for gi, wv in v[dw].items():
                acc[gi] = acc.get(gi, 0) + wv / len(order)
        for gi, wv in acc.items():
            cv[dw][gi] = wv
        for a in range(len(order)):
            va, vb = order[a], order[(a + 1) % len(order)]
            e = bm.edges.get((va, vb))
            # Wind opposite to the cuff face already using this edge (consistent normals).
            loop = e.link_loops[0]
            tri = [vb, va, cv] if loop.vert is va else [va, vb, cv]
            f = bm.faces.new(tri)
            f.smooth = False
            f[flag] = 1
            for l in f.loops:
                if l.vert is cv:
                    l[uvl].uv = dark_uv
                else:
                    ang = 2 * math.pi * order.index(l.vert) / len(order)
                    l[uvl].uv = dark_uv + Vector((math.cos(ang), math.sin(ang))) * (2 / W)
        report.setdefault('cuff_opening', {})[side] = {'centre': list(centre), 'vertices': len(order)}
    bm.to_mesh(me); bm.free(); me.update()

    # Build each hand as its own object, then join it into the body.
    hand_objs = []
    all_bones = {}
    for side in ('Left', 'Right'):
        centre, _ = openings[side]
        origin, R = frame(rig, centre, side)
        b, bones = build_hand(side)
        world = [origin + R @ p for p in b.co]
        hm = bpy.data.meshes.new('HandTmp_' + side)
        hm.from_pydata([tuple(p) for p in world], [], b.faces)
        hm.update()
        hb = bmesh.new(); hb.from_mesh(hm)
        bmesh.ops.recalc_face_normals(hb, faces=hb.faces)
        hb.to_mesh(hm); hb.free()
        for p in hm.polygons: p.use_smooth = True
        uv = hm.uv_layers.new(name=me.uv_layers.active.name)
        for l in hm.loops:
            lp = b.co[l.vertex_index]
            uv.data[l.index].uv = (skin_uv.x + lp.x * 0.012, skin_uv.y + lp.y * 0.012)
        ob = bpy.data.objects.new('HandTmp_' + side, hm)
        bpy.context.scene.collection.objects.link(ob)
        ob.data.materials.append(me.materials[0])
        groups = {}
        for vi, wts in enumerate(b.wt):
            tot = sum(wts.values())
            for name, wv in wts.items():
                g = groups.get(name) or ob.vertex_groups.new(name=name)
                groups[name] = g
                g.add([vi], wv / tot, 'REPLACE')
        hand_objs.append(ob)
        for name, (head, tail, up, parent) in bones.items():
            all_bones[name] = (origin + R @ head, origin + R @ tail, (R @ up).normalized(), parent)
        hm.calc_loop_triangles()
        report.setdefault('hand_triangles', {})[side] = len(hm.loop_triangles)

    # Finger bones under each Hand bone (Mixamo-style names, mapped to Humanoid fingers in Unity).
    bpy.ops.object.select_all(action='DESELECT')
    bpy.context.view_layer.objects.active = rig
    rig.select_set(True)
    bpy.ops.object.mode_set(mode='EDIT')
    eb = rig.data.edit_bones
    for name, (head, tail, up, parent) in all_bones.items():
        e = eb.new(name)
        e.head, e.tail = head, tail
        e.align_roll(up)
        e.parent = eb[parent]
        e.use_connect = parent in all_bones
        e.use_deform = True
    bpy.ops.object.mode_set(mode='OBJECT')
    report['finger_bones'] = len(all_bones)

    # Vertex normals of the new hands, keyed by position, for the custom-normal restore.
    hand_normals = {}
    for ob in hand_objs:
        for v in ob.data.vertices:
            hand_normals[tuple(round(x, 6) for x in v.co)] = tuple(v.normal)
    bpy.ops.object.select_all(action='DESELECT')
    for ob in hand_objs: ob.select_set(True)
    mesh.select_set(True)
    bpy.context.view_layer.objects.active = mesh
    bpy.ops.object.join()
    return hand_normals


CURL = {'Index': (80, 95, 60), 'Middle': (85, 95, 60), 'Ring': (88, 95, 60), 'Pinky': (90, 95, 60),
        'Thumb': (20, 40, 50)}


def curl(rig, amount):
    """Review pose: bend every finger towards the palm (bone roll: local Z = back of the hand)."""
    for side in ('Left', 'Right'):
        for finger, angles in CURL.items():
            for k, deg in enumerate(angles, 1):
                pb = rig.pose.bones[f'{side}Hand{finger}{k}']
                pb.rotation_mode = 'QUATERNION'
                pb.rotation_quaternion = Quaternion((1, 0, 0), math.radians(-deg * amount))
    bpy.context.view_layer.update()
