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
mtl_name = os.path.splitext(os.path.basename(dest))[0] + ".mtl"
with open(os.path.join(os.path.dirname(dest), mtl_name), "w", encoding="utf-8", newline="\n") as mtl:
    mtl.write("newmtl horror_mask\n")
    mtl.write("Kd 0.800000 0.800000 0.800000\n")
    mtl.write("Ks 0.200000 0.200000 0.200000\n")
    mtl.write("Ns 120.000000\n")
    mtl.write("d 1.000000\n")
    mtl.write("illum 2\n")

with open(dest, "w", encoding="utf-8", newline="\n") as obj_file:
    obj_file.write("# Decimated from horror_mask.glb by export-decimated-mask.py\n")
    obj_file.write(f"mtllib {mtl_name}\n")
    vertex_offset = 1
    uv_offset = 1
    normal_offset = 1
    for obj in mesh_objects:
        mesh = obj.data
        obj_file.write(f"o {obj.name}\n")
        for vertex in mesh.vertices:
            world_pos = obj.matrix_world @ vertex.co
            obj_file.write(f"v {world_pos.x:.7f} {world_pos.y:.7f} {world_pos.z:.7f}\n")
        uv_layer = mesh.uv_layers.active.data if mesh.uv_layers.active else None
        for poly in mesh.polygons:
            for loop_index in poly.loop_indices:
                if uv_layer:
                    uv = uv_layer[loop_index].uv
                    obj_file.write(f"vt {uv.x:.7f} {uv.y:.7f}\n")
                else:
                    loop = mesh.loops[loop_index]
                    co = mesh.vertices[loop.vertex_index].co
                    obj_file.write(f"vt {co.x + 0.5:.7f} {co.y + 0.5:.7f}\n")
        normal_matrix = obj.matrix_world.to_3x3().inverted().transposed()
        for poly in mesh.polygons:
            for loop_index in poly.loop_indices:
                normal = (normal_matrix @ mesh.loops[loop_index].normal).normalized()
                obj_file.write(f"vn {normal.x:.7f} {normal.y:.7f} {normal.z:.7f}\n")
        obj_file.write("usemtl horror_mask\n")
        for poly in mesh.polygons:
            if len(poly.loop_indices) != 3:
                continue
            refs = []
            for loop_index in poly.loop_indices:
                loop = mesh.loops[loop_index]
                vertex_id = vertex_offset + loop.vertex_index
                loop_local = loop_index - mesh.polygons[0].loop_start
                # Blender loop indices are contiguous for the exported mesh; use the absolute loop offset.
                loop_id = uv_offset + loop_index
                refs.append(f"{vertex_id}/{loop_id}/{loop_id}")
            obj_file.write("f " + " ".join(refs) + "\n")
        vertex_offset += len(mesh.vertices)
        loop_count = len(mesh.loops)
        uv_offset += loop_count
        normal_offset += loop_count

tri_count = 0
for obj in mesh_objects:
    tri_count += sum(max(0, len(poly.vertices) - 2) for poly in obj.data.polygons)
print(f"Exported {dest} with about {tri_count} triangles after {passes} decimation passes at ratio {ratio}")
