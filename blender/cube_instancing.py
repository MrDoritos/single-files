import bpy
import mathutils
from mathutils import Vector, Matrix
import bmesh
import random

mesh_obj = bpy.data.objects['Vert']
mesh_data = bpy.data.meshes['Vert']

size = 5
MAX_DEPTH = 3
CELL_SIZE = 4

class Grid:
    def __init__(self, size):
        self.voxels = []
        self.size = size
        
    def get(self, x, y, z):
        return self.voxels[x][y][z]
    
    def set(self, x, y, z, value):
        self.voxels[x][y][z] = value

    def default_generate(self, x, y, z):
        return 1

    def random_generate(self, x, y, z):
        return random.randint(1,6)==1

    def maze_generate(self, x, y, z):
        if x==0 and y==0 and z==0:
            return 1
        return 0

    def create(self, initial_value=0):
        self.voxels.clear()
        for x in range(int(self.size.x)):
            y_grid = []
            for y in range(int(self.size.y)):
                z_grid = []
                for z in range(int(self.size.z)):
                    z_grid.append(initial_value)
                y_grid.append(z_grid)
            self.voxels.append(y_grid);        

    def generate(self, query_func):
        for x in range(int(self.size.x)):
            for y in range(int(self.size.y)):
                for z in range(int(self.size.z)):
                    self.set(x, y, z, query_func(x, y, z))

    def enumerate(self):
        ret = []
        for x in range(int(self.size.x)):
            for y in range(int(self.size.y)):
                for z in range(int(self.size.z)):
                    if self.voxels[x][y][z]:
                        ret.append((x,y,z))
        return ret

    def export(self, obj, mesh):
        bpy.context.view_layer.objects.active = obj
        coords = self.enumerate()
        bm = bmesh.new()
        print(coords)
        for v in coords:
            bm.verts.new(v)
        bm.to_mesh(mesh)
        bm.free()

class Maze(Grid):
    def __init__(self, size):
        Grid.__init__(self, size)
        self.create(0)
        self.generate(self.maze_generate)
        
    def is_voxel_solid(self, x, y, z):
        return self.check_cell_recursive(x, y, z, depth=0)
    
    def check_cell_recursive(self, x, y, z, depth):
        pass
        
maze = Maze(Vector([size,size,size]))
print(maze.voxels)
maze.export(mesh_obj, mesh_data)