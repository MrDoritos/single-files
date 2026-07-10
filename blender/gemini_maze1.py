import bpy
import mathutils
from mathutils import Vector
import bmesh
import mathutils.noise # <--- Crucial import for procedural randomness

mesh_obj = bpy.data.objects['Vert']
mesh_data = bpy.data.meshes['Vert']

# Note: For infinite/large worlds, 'size' represents your local chunk bounds
size = 32 
MAX_DEPTH = 3
CELL_SIZE = 8

class Grid:
    def __init__(self, size):
        self.voxels = []
        self.size = size
        
    def get(self, x, y, z):
        return self.voxels[x][y][z]
    
    def set(self, x, y, z, value):
        self.voxels[x][y][z] = value

    def create(self, initial_value=0):
        self.voxels.clear()
        for x in range(int(self.size.x)):
            y_grid = []
            for y in range(int(self.size.y)):
                z_grid = []
                for z in range(int(self.size.z)):
                    z_grid.append(initial_value)
                y_grid.append(z_grid)
            self.voxels.append(y_grid)       

    def generate(self, query_func):
        for x in range(int(self.size.x)):
            for y in range(int(self.size.y)):
                for z in range(int(self.size.z)):
                    self.set(x, y, z, query_func(x, y, z))

    def enumerate(self, invert=False):
        ret = []
        for x in range(int(self.size.x)):
            for y in range(int(self.size.y)):
                for z in range(int(self.size.z)):
                    if (self.voxels[x][y][z]) ^ invert:
                        ret.append((x,y,z))
        return ret

    def export(self, obj, mesh):
        bpy.context.view_layer.objects.active = obj
        coords = self.enumerate(True)
        bm = bmesh.new()
        for v in coords:
            bm.verts.new(v)
        bm.to_mesh(mesh)
        bm.free()
        mesh.update() # Refreshes Geometry Nodes in the viewport
    
class Maze(Grid):
    def __init__(self, size):
        Grid.__init__(self, size)
        self.create(0)
        self.generate(self.maze_generate)
        
    def maze_generate(self, x, y, z):
        return 1 if self.is_voxel_solid(x, y, z) else 0
        
    def is_voxel_solid(self, x, y, z):
            # 1. Establish our grid rhythm (Even = Room, Odd = Wall)
            is_wall_x = (x % 2 == 1)
            is_wall_y = (y % 2 == 1)
            is_wall_z = (z % 2 == 1)
            
            # Pillars where all walls meet remain solid for structural rigidity
            if is_wall_x and is_wall_y and is_wall_z:
                return True
            # Rooms themselves are always open air
            if not is_wall_x and not is_wall_y and not is_wall_z:
                return False
                
            # 2. Get the coordinate of the cell that "owns" this wall
            cell_x = x // 2
            cell_y = y // 2
            cell_z = z // 2
            
            # 3. Use our deterministic hash to make a single choice for this cell
            cell_hash = mathutils.noise.cell_vector(Vector((cell_x, cell_y, cell_z)))
            choice = cell_hash[0] # Returns a float between -1.0 and 1.0
            choice = (choice * 2) - 1    
            # 4. The Magic Logic: Every cell must carve EXACTLY one exit.
            # We divide the hash range into 3 possibilities: Carve X, Carve Y, or Carve Z.
            if choice < -0.33:
                # Cell chooses to carve along X. Open X-walls, close Y and Z.
                if is_wall_x and not is_wall_y and not is_wall_z: return False
                return True
            elif choice < 0.33:
                # Cell chooses to carve along Y. Open Y-walls, close X and Z.
                if is_wall_y and not is_wall_x and not is_wall_z: return False
                return True
            else:
                # Cell chooses to carve along Z (Vertical passage). Open Z-walls, close X and Y.
                if is_wall_z and not is_wall_x and not is_wall_y: return False
                return True

maze = Maze(Vector([size, size, size]))
maze.export(mesh_obj, mesh_data)