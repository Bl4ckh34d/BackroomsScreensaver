import argparse
import sys

import bpy


def parse_args():
    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--triangles", type=int, required=True)
    return parser.parse_args(argv)


def import_obj(path):
    if hasattr(bpy.ops.wm, "obj_import"):
        bpy.ops.wm.obj_import(filepath=path)
    else:
        bpy.ops.import_scene.obj(filepath=path)


def export_obj(path):
    bpy.ops.object.select_all(action="DESELECT")
    bpy.context.view_layer.objects.active.select_set(True)
    if hasattr(bpy.ops.wm, "obj_export"):
        bpy.ops.wm.obj_export(
            filepath=path,
            export_selected_objects=True,
            export_materials=True,
            export_uv=True,
            export_normals=True,
        )
    else:
        bpy.ops.export_scene.obj(
            filepath=path,
            use_selection=True,
            use_materials=True,
            use_uvs=True,
            use_normals=True,
        )


def main():
    args = parse_args()
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()

    import_obj(args.input)
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError("No mesh objects imported")

    bpy.ops.object.select_all(action="DESELECT")
    for obj in meshes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    if len(meshes) > 1:
        bpy.ops.object.join()
    obj = bpy.context.view_layer.objects.active

    current = max(1, len(obj.data.polygons))
    ratio = max(0.01, min(1.0, args.triangles / current))
    dec = obj.modifiers.new("target_triangle_decimate", "DECIMATE")
    dec.decimate_type = "COLLAPSE"
    dec.ratio = ratio
    dec.use_collapse_triangulate = True
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.modifier_apply(modifier=dec.name)

    tri = obj.modifiers.new("triangulate_export_mesh", "TRIANGULATE")
    bpy.ops.object.modifier_apply(modifier=tri.name)
    obj.data.update()

    print(f"Reduced {current} faces to {len(obj.data.polygons)} faces")
    export_obj(args.output)


if __name__ == "__main__":
    main()
