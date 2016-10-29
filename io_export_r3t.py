import bpy, time, struct, mathutils
from bpy.props import *

bl_info = {
    "name": "Rac3r track (.trk)",
    "author": "LiterallyVoid",
    "blender": (2, 7, 5),
    "location": "File > Export > Rac3r track (.trk)",
    "description": "DO NOT USE",
    "warning": "Maybe buggy",
    "category": "Import-Export"
    }

def writeObj(fi, obj, scene):
    d = obj.to_mesh(scene, True, "PREVIEW")
    edges = {} # All edges (and the faces they have)
    actualEdges = {}
    loop = {} # loop[face] = [face 1, ...]
    faces = []

    v = mathutils.Vector

    def edgestr(e):
        return str(list(sorted(e)))

    def facestr(f):
        return str(list(sorted(f.vertices)))

    def isEdge(e):
        return len(edges[e]) > 1

    for e in d.edges:
        actualEdges[edgestr(e.key)] = e

    for p in d.polygons:
        faces.append(p)
        faceEdges = p.edge_keys
        for e in faceEdges:
            e = edgestr(e)
            edges[e] = edges.get(e, [])
            edges[e].append(p)

    for e in edges:
        if isEdge(e):
            others = []
            for f in edges[e]:
                others.append([f, e])
            for o in others:
                if facestr(o[0]) in loop:
                    loop[facestr(o[0])].extend(others)
                else:
                    loop[facestr(o[0])] = others

    empty = scene.objects["start"]

    pads = {}

    for s in scene.objects:
        closest = None
        closestDist = 99999999
        for f in faces:
            dist = (v(f.center) - s.location).length
            if dist < closestDist:
                closestDist = dist
                closest = f
        if s.name.startswith("padL"):
            pads[facestr(closest)] = 1
        elif s.name.startswith("padR"):
            pads[facestr(closest)] = 2
        elif s.name.startswith("padM"):
            pads[facestr(closest)] = 3

    pos = empty.location
    closest = None
    closestDist = 99999999
    for f in faces:
        dist = (v(f.center) - pos).length
        if dist < closestDist:
            closestDist = dist
            closest = f
    direction = mathutils.Vector([empty.matrix_world[0][2], empty.matrix_world[1][2], empty.matrix_world[2][2]])
    f = closest
    lastdir = direction
    traveled = []

    print(pads)

    while True:
        if f in traveled:
            break
        traveled.append(f)

        possible = []
        for o in loop[facestr(f)]:
            if o[0] in traveled:
                continue
            possible.append([o, v(o[0].center) - v(f.center)])
        possible.sort(key = lambda a: (a[1] - lastdir).length)
        if len(possible) == 0:
            break
        lastdir = possible[0][1]
        e = actualEdges[possible[0][0][1]]
        v1 = d.vertices[e.vertices[0]].co
        v2 = d.vertices[e.vertices[1]].co
        length = (v1 - v2).length

        fi.write("%f %f %f %f %f %f %f %d\n" % (v(f.center).x, v(f.center).y, v(f.center).z, f.normal.x, f.normal.y, f.normal.z, length, pads.get(facestr(f), 0)))
        
        f = possible[0][0][0]

    bpy.data.meshes.remove(d)

class ExportR3Track(bpy.types.Operator):
    bl_idname = "export.trk"
    bl_label = "Export R3Track"

    filename_ext = ".trk"

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
    self.layout.operator(ExportMSH.bl_idname, text = "Rac3r track (.trk)", icon = "BLENDER")

def register():
    bpy.utils.register_class(ExportR3Track)
    bpy.types.INFO_MT_file_export.append(menu_func)

def unregister():
    bpy.utils.unregister_class(ExportR3Track)
    bpy.types.INFO_MT_file_export.remove(menu_func)

if __name__ ==  "__main__":
    register()
