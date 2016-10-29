import bpy, time, struct
from bpy.props import *

bl_info = {
    "name": "MeshV2 (.mh2)",
    "author": "Void-7",
    "blender": (2, 7, 5),
    "location": "File > Export > MeshV2 (.mh2)",
    "description": "DO NOT USE",
    "warning": "Maybe buggy",
    "category": "Import-Export"
    }

def mesh_triangulate(me):
    import bmesh
    bm = bmesh.new()
    bm.from_mesh(me)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bm.to_mesh(me)
    bm.free()

def writeObj(fi, obj, scene):
    mesh = obj.to_mesh(scene, True, "PREVIEW")
    mesh_triangulate(mesh)
    matrix = obj.matrix_world.copy()

    triangles = []

    for poly in mesh.polygons:
        vts = poly.vertices
        txs = poly.loop_indices
        mat = obj.material_slots[poly.material_index].name
        triangles.append([vts[0], vts[1], vts[2], txs[0], txs[1], txs[2], mat, poly.use_smooth, poly.normal])

    layer = mesh.uv_layers[0].data
    try:
        colors = [v.color for v in mesh.vertex_colors[0].data]
    except:
        colors = [[1, 1, 1]] * len(layer)
    for t in triangles:
        norm = None
        if t[7]:
            norm = [mesh.vertices[t[0]].normal, mesh.vertices[t[1]].normal, mesh.vertices[t[2]].normal]
        else:
            norm = [t[8], t[8], t[8]]
        verts = [
            mesh.vertices[t[0]].co,
            norm[0],
            layer[t[3]].uv,
            colors[t[3]],
            mesh.vertices[t[1]].co,
            norm[1],
            layer[t[4]].uv,
            colors[t[4]],
            mesh.vertices[t[2]].co,
            norm[2],
            layer[t[5]].uv,
            colors[t[5]]]

        for i in [0, 1, 4, 5, 8, 9]:
            if i % 4 == 0:
                verts[i] = matrix * verts[i]
            else:
                verts[i] = verts[i] * matrix

        first = True
        i = 0
        for v in verts:
            if i > 3:
                i = 0
                first = True
                fi.write(" %s\n" % t[6])
            i += 1
            for co in v:
                if not first: fi.write(" ")
                first = False
                fi.write("%.8f" % co)
        fi.write(" %s\n" % t[6])
    bpy.data.meshes.remove(mesh)

class ExportMH2(bpy.types.Operator):
    bl_idname = "export.mh2"
    bl_label = "Export MH2"

    filename_ext = ".mh2"

    filepath = StringProperty(subtype = "FILE_PATH", name = "File Path", description = "Filepath for exporting", maxlen = 1024, default = "")

    def execute(self, context):
        fi = open(self.properties.filepath, "w")
        for obj in context.selected_objects:
            writeObj(fi, obj, context.scene)
        fi.close()
      
        return {"FINISHED"}

    def invoke(self, context, event):
        wm = context.window_manager
        wm.fileselect_add(self)
        return {'RUNNING_MODAL'}

    @classmethod
    def poll(self, context):
        return True

def menu_func(self, context):
    self.layout.operator(ExportMSH.bl_idname, text = "MeshV2 (.mh2)", icon = "BLENDER")

def register():
    bpy.utils.register_class(ExportMH2)
    bpy.types.INFO_MT_file_export.append(menu_func)

def unregister():
    bpy.utils.unregister_class(ExportMH2)
    bpy.types.INFO_MT_file_export.remove(menu_func)

if __name__ ==  "__main__":
    register()
