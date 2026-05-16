import math
import shutil
import sys
from pathlib import Path

import bpy
import bmesh
from mathutils import Vector


ARGS = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else []
EXTRACTED = Path(ARGS[0])
RUNTIME = Path(ARGS[1])
TEXTURES = RUNTIME / "textures"

MODELS = [
    ("office_chair_modern", EXTRACTED / "Office_Chair_v1_L3.123cf4a20c81-89b5-417b-848a-24ea67f9fb6b" / "10239_Office_Chair_v1_L3.obj", 12000, "Z"),
    ("office_chair_classic", EXTRACTED / "Office chair.obj", 12000, "Y"),
    ("office_chair_task", EXTRACTED / "Office Chair.fbx", 12000, None),
    ("filing_cabinet", EXTRACTED / "Filing cabinet OBJ.obj", 900, "Y"),
    ("office_desk", EXTRACTED / "desk.obj", 1000, "Y"),
    ("trashbin", EXTRACTED.parent / "trashbin.fbx", 1600, None),
    ("desklamp", EXTRACTED.parent / "desklamp" / "77763.obj", 1800, "Z"),
    ("audio_caset", EXTRACTED.parent / "audio_caset" / "cassette_audio_4_track_ericdesign.obj", 1600, "Y"),
]

TEXTURE_COPIES = [
    (EXTRACTED / "Office_Chair_v1_L3.123cf4a20c81-89b5-417b-848a-24ea67f9fb6b" / "10239_Office_Chair_v1_Diffuse.jpg", TEXTURES / "office_chair_modern_diffuse.jpg"),
    (EXTRACTED / "tex" / "2209.jpg", TEXTURES / "office_chair_classic_2209.jpg"),
    (EXTRACTED / "tex" / "textiles_1_20090323_1963410504.png", TEXTURES / "office_chair_classic_textiles.png"),
    (EXTRACTED / "Texture" / "Chair.png", TEXTURES / "office_chair_task_diffuse.png"),
]

MATERIAL_IDS = {
    "office_chair_modern": {"default": 16},
    "office_chair_classic": {
        "Mat.3": 17,
        "Материал": 18,
        "Материал.2": 19,
        "Mat": 20,
        "Материал.1": 21,
        "default": 17,
    },
    "office_chair_task": {"default": 22},
    "trashbin": {"default": 10},
    "desklamp": {
        "LAMP_SHADE": 21,
        "NATURAL_OAK": 8,
        "RUBBER": 18,
        "STEEL": 10,
        "WHITE": 21,
        "default": 10,
    },
    "audio_caset": {
        "Mat.1": 23,
        "white_plastic": 21,
        "sticker": 24,
        "crome": 10,
        "glass": 11,
        "black_plastic": 23,
        "default": 23,
    },
}

SKIP_MATERIALS = {
    "desklamp": {"FLOOR"},
}


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()


def import_model(path: Path, obj_up_axis):
    suffix = path.suffix.lower()
    if suffix == ".obj":
        kwargs = {"filepath": str(path)}
        if obj_up_axis == "Z":
            kwargs.update({"forward_axis": "Y", "up_axis": "Z"})
        elif obj_up_axis == "Y":
            kwargs.update({"forward_axis": "NEGATIVE_Z", "up_axis": "Y"})
        bpy.ops.wm.obj_import(**kwargs)
    elif suffix == ".fbx":
        bpy.ops.import_scene.fbx(filepath=str(path))
    else:
        raise ValueError(f"Unsupported source format: {path}")


def mesh_objects():
    return [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]


def join_meshes():
    meshes = mesh_objects()
    if not meshes:
        return None
    bpy.ops.object.select_all(action="DESELECT")
    for obj in meshes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    if len(meshes) > 1:
        bpy.ops.object.join()
    obj = bpy.context.view_layer.objects.active
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    return obj


def triangle_count(obj):
    return sum(max(0, len(poly.vertices) - 2) for poly in obj.data.polygons)


def decimate(obj, target_tris):
    tris = triangle_count(obj)
    if tris <= target_tris:
        return
    ratio = max(0.02, min(1.0, target_tris / float(tris)))
    modifier = obj.modifiers.new(name="RuntimeDecimate", type="DECIMATE")
    modifier.ratio = ratio
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.modifier_apply(modifier=modifier.name)


def triangulate(obj):
    bpy.context.view_layer.objects.active = obj
    modifier = obj.modifiers.new(name="RuntimeTriangulate", type="TRIANGULATE")
    bpy.ops.object.modifier_apply(modifier=modifier.name)


def vertex_bounds_world(obj):
    points = [obj.matrix_world @ vertex.co for vertex in obj.data.vertices]
    min_v = Vector((min(p.x for p in points), min(p.y for p in points), min(p.z for p in points)))
    max_v = Vector((max(p.x for p in points), max(p.y for p in points), max(p.z for p in points)))
    return min_v, max_v


def clean_material_name(name: str) -> str:
    if len(name) > 4 and name[-4] == "." and name[-3:].isdigit():
        return name[:-4]
    return name


def poly_material_name(obj, poly) -> str | None:
    if 0 <= poly.material_index < len(obj.material_slots):
        material = obj.material_slots[poly.material_index].material
        if material:
            return clean_material_name(material.name)
    return None


def face_material_id(model_name: str, obj, poly) -> int | None:
    material_map = MATERIAL_IDS.get(model_name)
    if not material_map:
        return None
    name = poly_material_name(obj, poly)
    if name and name in material_map:
        return material_map[name]
    return material_map.get("default")


def write_engine_obj(obj, path: Path, model_name: str):
    mesh = obj.data
    min_v, max_v = vertex_bounds_world(obj)
    center_x = (min_v.x + max_v.x) * 0.5
    center_y = (min_v.y + max_v.y) * 0.5
    height = max(max_v.z - min_v.z, 0.001)
    scale = 1.0 / height

    vertices = []
    for vertex in mesh.vertices:
        world = obj.matrix_world @ vertex.co
        engine_x = (world.x - center_x) * scale
        engine_y = (world.z - min_v.z) * scale
        engine_z = (world.y - center_y) * scale
        vertices.append((engine_x, engine_y, engine_z))

    uv_layer = mesh.uv_layers.active.data if mesh.uv_layers.active else None
    vt_entries = []
    faces = []
    skip_materials = SKIP_MATERIALS.get(model_name, set())
    for poly in mesh.polygons:
        material_name = poly_material_name(obj, poly)
        if material_name in skip_materials:
            continue
        order = [0, 2, 1] if len(poly.vertices) == 3 else list(range(len(poly.vertices) - 1, -1, -1))
        face = []
        for offset in order:
            vertex_index = poly.vertices[offset] + 1
            loop_index = poly.loop_indices[offset]
            if uv_layer:
                uv = uv_layer[loop_index].uv
                vt_entries.append((uv.x, 1.0 - uv.y))
            else:
                p = vertices[vertex_index - 1]
                vt_entries.append((p[0] + 0.5, p[2] + 0.5))
            face.append((vertex_index, len(vt_entries)))
        faces.append((face_material_id(model_name, obj, poly), face))

    with path.open("w", encoding="utf-8", newline="\n") as handle:
        handle.write(f"# Runtime prop generated from {obj.name}\n")
        for x, y, z in vertices:
            handle.write(f"v {x:.6f} {y:.6f} {z:.6f}\n")
        for u, v in vt_entries:
            handle.write(f"vt {u:.6f} {v:.6f}\n")
        current_material = None
        for material_id, face in faces:
            if material_id is not None and material_id != current_material:
                handle.write(f"usemtl p{material_id}\n")
                current_material = material_id
            if len(face) == 3:
                handle.write("f " + " ".join(f"{vi}/{ti}" for vi, ti in face) + "\n")
            elif len(face) > 3:
                for i in range(1, len(face) - 1):
                    tri = [face[0], face[i], face[i + 1]]
                    handle.write("f " + " ".join(f"{vi}/{ti}" for vi, ti in tri) + "\n")


def process(name, source, target_tris, obj_up_axis):
    if not source.exists():
        print(f"Skipping missing source: {source}")
        return
    clear_scene()
    import_model(source, obj_up_axis)
    obj = join_meshes()
    if obj is None:
        print(f"Skipping empty source: {source}")
        return
    decimate(obj, target_tris)
    triangulate(obj)
    output = RUNTIME / f"{name}.obj"
    output.parent.mkdir(parents=True, exist_ok=True)
    write_engine_obj(obj, output, name)
    print(f"{name}: {triangle_count(obj)} tris -> {output}")
    if name in MATERIAL_IDS:
        mats = [clean_material_name(slot.material.name) for slot in obj.material_slots if slot.material]
        print(f"{name} materials: {', '.join(mats) if mats else '(none)'}")


def main():
    RUNTIME.mkdir(parents=True, exist_ok=True)
    TEXTURES.mkdir(parents=True, exist_ok=True)
    for item in RUNTIME.glob("*.obj"):
        item.unlink()
    for source, target in TEXTURE_COPIES:
        if source.exists():
            shutil.copyfile(source, target)
            print(f"copied texture: {target}")
        else:
            print(f"missing texture: {source}")
    for name, source, target_tris, obj_up_axis in MODELS:
        process(name, source, target_tris, obj_up_axis)


main()
