import json
import math
import shutil
import struct
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "assets" / "models" / "emergency_exit_sign" / "source" / "theappxr_Uscita_Di_Emergenza_NODRACO_standard.glb"
SOURCE_TEXTURE = ROOT / "assets" / "models" / "emergency_exit_sign" / "textures" / "ExitSign_DIFFUSE_0.jpeg"
RUNTIME = ROOT / "assets" / "models" / "runtime"
RUNTIME_TEXTURES = RUNTIME / "textures"
OUTPUT_OBJ = RUNTIME / "emergency_exit_sign.obj"
OUTPUT_TEXTURE = RUNTIME_TEXTURES / "emergency_exit_sign_diffuse.jpeg"


COMPONENT_STRUCT = {
    5120: ("b", 1),
    5121: ("B", 1),
    5122: ("h", 2),
    5123: ("H", 2),
    5125: ("I", 4),
    5126: ("f", 4),
}

TYPE_COUNT = {
    "SCALAR": 1,
    "VEC2": 2,
    "VEC3": 3,
    "VEC4": 4,
    "MAT4": 16,
}


def mat4_identity():
    return [
        [1.0, 0.0, 0.0, 0.0],
        [0.0, 1.0, 0.0, 0.0],
        [0.0, 0.0, 1.0, 0.0],
        [0.0, 0.0, 0.0, 1.0],
    ]


def mat4_mul(a, b):
    return [[sum(a[r][k] * b[k][c] for k in range(4)) for c in range(4)] for r in range(4)]


def quat_to_mat4(q):
    x, y, z, w = q
    xx, yy, zz = x * x, y * y, z * z
    xy, xz, yz = x * y, x * z, y * z
    wx, wy, wz = w * x, w * y, w * z
    return [
        [1.0 - 2.0 * (yy + zz), 2.0 * (xy - wz), 2.0 * (xz + wy), 0.0],
        [2.0 * (xy + wz), 1.0 - 2.0 * (xx + zz), 2.0 * (yz - wx), 0.0],
        [2.0 * (xz - wy), 2.0 * (yz + wx), 1.0 - 2.0 * (xx + yy), 0.0],
        [0.0, 0.0, 0.0, 1.0],
    ]


def node_matrix(node):
    if "matrix" in node:
        raw = node["matrix"]
        return [[float(raw[c * 4 + r]) for c in range(4)] for r in range(4)]
    t = node.get("translation", [0.0, 0.0, 0.0])
    r = node.get("rotation", [0.0, 0.0, 0.0, 1.0])
    s = node.get("scale", [1.0, 1.0, 1.0])
    m = quat_to_mat4([float(v) for v in r])
    for col in range(3):
        scale = float(s[col])
        for row in range(3):
            m[row][col] *= scale
    m[0][3], m[1][3], m[2][3] = float(t[0]), float(t[1]), float(t[2])
    return m


def transform_point(m, p):
    x, y, z = p
    return (
        m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3],
        m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3],
        m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3],
    )


def read_glb(path):
    data = path.read_bytes()
    magic, version, length = struct.unpack_from("<4sII", data, 0)
    if magic != b"glTF" or version != 2 or length != len(data):
        raise ValueError(f"Unsupported GLB header: {path}")
    offset = 12
    json_chunk = None
    bin_chunk = None
    while offset < len(data):
        chunk_len, chunk_type = struct.unpack_from("<II", data, offset)
        offset += 8
        chunk = data[offset : offset + chunk_len]
        offset += chunk_len
        if chunk_type == 0x4E4F534A:
            json_chunk = chunk
        elif chunk_type == 0x004E4942:
            bin_chunk = chunk
    if json_chunk is None or bin_chunk is None:
        raise ValueError("GLB is missing JSON or BIN chunks")
    return json.loads(json_chunk.decode("utf-8")), bin_chunk


def accessor_values(doc, bin_chunk, accessor_index):
    accessor = doc["accessors"][accessor_index]
    if accessor.get("sparse"):
        raise ValueError("Sparse accessors are not supported")
    view = doc["bufferViews"][accessor["bufferView"]]
    fmt, size = COMPONENT_STRUCT[accessor["componentType"]]
    count = int(accessor["count"])
    elems = TYPE_COUNT[accessor["type"]]
    offset = int(view.get("byteOffset", 0)) + int(accessor.get("byteOffset", 0))
    stride = int(view.get("byteStride", size * elems))
    unpack = struct.Struct("<" + fmt * elems).unpack_from
    out = []
    for i in range(count):
        value = unpack(bin_chunk, offset + i * stride)
        if accessor["type"] == "SCALAR":
            out.append(value[0])
        else:
            out.append(tuple(float(v) for v in value))
    return out


def collect_mesh_instances(doc):
    roots = []
    scenes = doc.get("scenes", [])
    scene_index = int(doc.get("scene", 0))
    if scenes:
        roots = scenes[scene_index].get("nodes", [])
    elif doc.get("nodes"):
        roots = list(range(len(doc["nodes"])))

    instances = []

    def walk(node_index, parent):
        node = doc["nodes"][node_index]
        local = node_matrix(node)
        world = mat4_mul(parent, local)
        if "mesh" in node:
            instances.append((int(node["mesh"]), world))
        for child in node.get("children", []):
            walk(int(child), world)

    for root in roots:
        walk(int(root), mat4_identity())
    return instances


def main():
    doc, bin_chunk = read_glb(SOURCE)
    triangles = []
    positions_for_bounds = []
    for mesh_index, matrix in collect_mesh_instances(doc):
        mesh = doc["meshes"][mesh_index]
        for primitive in mesh.get("primitives", []):
            if primitive.get("mode", 4) != 4:
                continue
            attributes = primitive["attributes"]
            positions = [transform_point(matrix, p) for p in accessor_values(doc, bin_chunk, attributes["POSITION"])]
            uvs = accessor_values(doc, bin_chunk, attributes["TEXCOORD_0"]) if "TEXCOORD_0" in attributes else [(0.5, 0.5)] * len(positions)
            indices = accessor_values(doc, bin_chunk, primitive["indices"]) if "indices" in primitive else list(range(len(positions)))
            positions_for_bounds.extend(positions)
            for i in range(0, len(indices) - 2, 3):
                tri = []
                for raw_index in (indices[i], indices[i + 1], indices[i + 2]):
                    idx = int(raw_index)
                    tri.append((positions[idx], uvs[idx]))
                triangles.append(tri)

    if not triangles:
        raise ValueError(f"No triangle geometry found in {SOURCE}")

    min_x = min(p[0] for p in positions_for_bounds)
    max_x = max(p[0] for p in positions_for_bounds)
    min_y = min(p[1] for p in positions_for_bounds)
    max_y = max(p[1] for p in positions_for_bounds)
    min_z = min(p[2] for p in positions_for_bounds)
    max_z = max(p[2] for p in positions_for_bounds)
    center_x = (min_x + max_x) * 0.5
    center_z = (min_z + max_z) * 0.5
    height = max(max_y - min_y, 0.001)
    scale = 1.0 / height

    RUNTIME.mkdir(parents=True, exist_ok=True)
    RUNTIME_TEXTURES.mkdir(parents=True, exist_ok=True)
    if SOURCE_TEXTURE.exists():
        shutil.copyfile(SOURCE_TEXTURE, OUTPUT_TEXTURE)

    vertex_index = 1
    tex_index = 1
    with OUTPUT_OBJ.open("w", encoding="utf-8", newline="\n") as handle:
        handle.write(f"# Runtime prop generated from {SOURCE.relative_to(ROOT)}\n")
        handle.write("usemtl p7\n")
        for tri in triangles:
            face = []
            for position, uv in tri:
                x = (position[0] - center_x) * scale
                y = (position[1] - min_y) * scale
                z = (position[2] - center_z) * scale
                handle.write(f"v {x:.8f} {y:.8f} {z:.8f}\n")
                handle.write(f"vt {float(uv[0]):.8f} {float(uv[1]):.8f}\n")
                face.append((vertex_index, tex_index))
                vertex_index += 1
                tex_index += 1
            handle.write("f " + " ".join(f"{vi}/{ti}" for vi, ti in face) + "\n")

    print(f"{OUTPUT_OBJ.relative_to(ROOT)}: {len(triangles)} tris")
    if OUTPUT_TEXTURE.exists():
        print(f"{OUTPUT_TEXTURE.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
