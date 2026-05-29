import bpy
import os
import sys


def arg_after(flag, default=None):
    if flag not in sys.argv:
        return default
    index = sys.argv.index(flag)
    return sys.argv[index + 1] if index + 1 < len(sys.argv) else default


source = arg_after("--source")
dest = arg_after("--dest")
ratio = float(arg_after("--ratio", "0.5"))
passes = int(arg_after("--passes", "2"))

if not source or not dest:
    raise SystemExit("Usage: blender --background --python export-decimated-mask.py -- --source input.glb --dest output.obj")

bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete()
bpy.ops.import_scene.gltf(filepath=source)

mesh_objects = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
if not mesh_objects:
    raise SystemExit(f"No mesh objects found in {source}")

for obj in mesh_objects:
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    for pass_index in range(passes):
        modifier = obj.modifiers.new(f"decimate_{pass_index + 1}", "DECIMATE")
        modifier.ratio = ratio
        modifier.use_collapse_triangulate = True
        bpy.ops.object.modifier_apply(modifier=modifier.name)
    obj.select_set(False)

os.makedirs(os.path.dirname(dest), exist_ok=True)
bpy.ops.object.select_all(action="DESELECT")
for obj in mesh_objects:
    obj.select_set(True)
bpy.context.view_layer.objects.active = mesh_objects[0]

if hasattr(bpy.ops.wm, "obj_export"):
    bpy.ops.wm.obj_export(
        filepath=dest,
        export_selected_objects=True,
        export_materials=True,
        export_uv=True,
        export_normals=True,
    )
else:
    bpy.ops.export_scene.obj(
        filepath=dest,
        use_selection=True,
        use_materials=True,
        use_uvs=True,
        use_normals=True,
    )

tri_count = 0
for obj in mesh_objects:
    tri_count += sum(max(0, len(poly.vertices) - 2) for poly in obj.data.polygons)
print(f"Exported {dest} with about {tri_count} triangles after {passes} decimation passes at ratio {ratio}")
