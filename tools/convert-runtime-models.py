import argparse
import math
import struct
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
RUNTIME = ROOT / "assets" / "models" / "runtime"
MATERIAL_COUNT = 26
PACKED_VERTEX_STRIDE = 24
HEADER = struct.Struct("<8sII10f")
PACKED_VERTEX = struct.Struct("<3H3h3h3H")

DEFAULT_MATERIALS = {
    "office_chair_modern": 16,
    "office_chair_classic": 17,
    "office_chair_task": 22,
    "filing_cabinet": 10,
    "office_desk": 8,
    "trashbin": 10,
    "desklamp": 21,
    "audio_caset": 23,
    "air_vent": 10,
    "emergency_exit_sign": 7,
    "ceiling_lamp_01": 21,
    "ceiling_lamp_02": 21,
    "ceiling_lamp_03": 21,
    "ceiling_lamp_04": 21,
}

MATERIAL_REMAPS = {
    # Material 18 is reserved for glowing menu text in the runtime shader.
    # The classic chair source used it for dark plastic armrest pieces.
    "office_chair_classic": {18: 17},
}


def parse_index(raw: str, count: int) -> int | None:
    if not raw:
        return None
    value = int(raw)
    if value > 0:
        index = value - 1
    elif value < 0:
        index = count + value
    else:
        return None
    return index if 0 <= index < count else None


def parse_face_token(token: str, position_count: int, texcoord_count: int) -> tuple[int, int | None] | None:
    parts = token.split("/")
    vertex = parse_index(parts[0], position_count)
    if vertex is None:
        return None
    texcoord = None
    if len(parts) > 1 and parts[1]:
        texcoord = parse_index(parts[1], texcoord_count)
    return vertex, texcoord


def sub(a, b):
    return a[0] - b[0], a[1] - b[1], a[2] - b[2]


def cross(a, b):
    return (
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    )


def dot(a, b):
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]


def normalize(v, fallback):
    length = math.sqrt(max(dot(v, v), 0.0))
    if length <= 1.0e-6:
        return fallback
    return v[0] / length, v[1] / length, v[2] / length


def uv_for(position, normal, texcoord_index, texcoords):
    if texcoord_index is not None and 0 <= texcoord_index < len(texcoords):
        return texcoords[texcoord_index]
    if abs(normal[1]) > 0.62:
        return position[0] * 1.7 + 0.5, position[2] * 1.7 + 0.5
    if abs(normal[0]) > abs(normal[2]):
        return position[2] * 1.7 + 0.5, position[1] * 1.7
    return position[0] * 1.7 + 0.5, position[1] * 1.7


def material_from_usemtl(line: str, fallback: int) -> int:
    name = line[7:].strip()
    if len(name) >= 2 and name[0].lower() == "p":
        digits = ""
        for ch in name[1:]:
            if not ch.isdigit():
                break
            digits += ch
        if digits:
            value = int(digits)
            if 0 <= value < MATERIAL_COUNT:
                return value
    return fallback


def load_obj(path: Path, fallback_material: int):
    positions: list[tuple[float, float, float]] = []
    texcoords: list[tuple[float, float]] = []
    faces: list[tuple[int, list[tuple[int, int | None]]]] = []
    current_material = fallback_material
    material_remap = MATERIAL_REMAPS.get(path.stem, {})

    with path.open("r", encoding="utf-8", errors="ignore") as handle:
        for raw in handle:
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            if line.startswith("v "):
                parts = line.split()
                if len(parts) >= 4:
                    positions.append((float(parts[1]), float(parts[2]), float(parts[3])))
            elif line.startswith("vt "):
                parts = line.split()
                if len(parts) >= 3:
                    texcoords.append((float(parts[1]), float(parts[2])))
            elif line.startswith("usemtl "):
                parsed_material = material_from_usemtl(line, fallback_material)
                current_material = material_remap.get(parsed_material, parsed_material)
            elif line.startswith("f "):
                poly = []
                for token in line.split()[1:]:
                    parsed = parse_face_token(token, len(positions), len(texcoords))
                    if parsed is not None:
                        poly.append(parsed)
                if len(poly) >= 3:
                    faces.append((current_material, poly))

    if not positions or not faces:
        raise ValueError(f"{path} has no usable geometry")

    vertices = []
    for material, poly in faces:
        for i in range(1, len(poly) - 1):
            tri = [poly[0], poly[i], poly[i + 1]]
            a = positions[tri[0][0]]
            b = positions[tri[1][0]]
            c = positions[tri[2][0]]
            normal = normalize(cross(sub(b, a), sub(c, a)), (0.0, 1.0, 0.0))
            tangent = normalize(sub(b, a), (1.0, 0.0, 0.0))
            if abs(dot(tangent, normal)) > 0.92:
                tangent = normalize(cross((0.0, 1.0, 0.0), normal), (1.0, 0.0, 0.0))
            for position_index, texcoord_index in tri:
                position = positions[position_index]
                uv = uv_for(position, normal, texcoord_index, texcoords)
                vertices.append((position, normal, tangent, uv, float(material)))

    min_pos = tuple(min(p[axis] for p in positions) for axis in range(3))
    max_pos = tuple(max(p[axis] for p in positions) for axis in range(3))
    return vertices, min_pos, max_pos


def quantize_unit(value: float, minimum: float, maximum: float) -> int:
    extent = maximum - minimum
    if abs(extent) <= 1.0e-9:
        return 0
    return max(0, min(65535, round((value - minimum) / extent * 65535.0)))


def quantize_snorm(value: float) -> int:
    return max(-32767, min(32767, round(max(-1.0, min(1.0, value)) * 32767.0)))


def write_brmesh(path: Path, vertices, min_pos, max_pos) -> None:
    uvs = [vertex[3] for vertex in vertices]
    min_uv = (min(uv[0] for uv in uvs), min(uv[1] for uv in uvs))
    max_uv = (max(uv[0] for uv in uvs), max(uv[1] for uv in uvs))
    with path.open("wb") as handle:
        handle.write(HEADER.pack(b"BRMPRP3\0", len(vertices), PACKED_VERTEX_STRIDE, *min_pos, *max_pos, *min_uv, *max_uv))
        for position, normal, tangent, uv, material in vertices:
            handle.write(
                PACKED_VERTEX.pack(
                    quantize_unit(position[0], min_pos[0], max_pos[0]),
                    quantize_unit(position[1], min_pos[1], max_pos[1]),
                    quantize_unit(position[2], min_pos[2], max_pos[2]),
                    quantize_snorm(normal[0]),
                    quantize_snorm(normal[1]),
                    quantize_snorm(normal[2]),
                    quantize_snorm(tangent[0]),
                    quantize_snorm(tangent[1]),
                    quantize_snorm(tangent[2]),
                    quantize_unit(uv[0], min_uv[0], max_uv[0]),
                    quantize_unit(uv[1], min_uv[1], max_uv[1]),
                    max(0, min(65535, round(material))),
                )
            )


def convert_obj(path: Path) -> tuple[Path, int]:
    fallback_material = DEFAULT_MATERIALS.get(path.stem, 10)
    vertices, min_pos, max_pos = load_obj(path, fallback_material)
    output = path.with_suffix(".brmesh")
    write_brmesh(output, vertices, min_pos, max_pos)
    return output, len(vertices) // 3


def main() -> None:
    parser = argparse.ArgumentParser(description="Convert runtime OBJ props into BackroomsMaze .brmesh files.")
    parser.add_argument("paths", nargs="*", type=Path, help="Specific OBJ files to convert. Defaults to assets/models/runtime/*.obj.")
    args = parser.parse_args()

    paths = args.paths or sorted(RUNTIME.glob("*.obj"))
    if not paths:
        raise SystemExit("No runtime OBJ files found.")

    for path in paths:
        path = path if path.is_absolute() else ROOT / path
        output, tris = convert_obj(path)
        print(f"{output.relative_to(ROOT)}: {tris} tris, {output.stat().st_size} bytes")


if __name__ == "__main__":
    main()
