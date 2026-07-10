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
"""
class Maze(Grid):
    def __init__(self, size):
        Grid.__init__(self, size)
        self.create(0)
        self.generate(self.maze_generate)
        
    def maze_generate(self, x, y, z):
        # We want to place a vertex if the algorithm decides it is SOLID
        return 1 if self.is_voxel_solid(x, y, z) else 0
        
    def is_voxel_solid(self, x, y, z):
        return self.check_cell_recursive(x, y, z, depth=0)
    
    def check_cell_recursive(self, x, y, z, depth):
        # Base Case: Reached maximum structural depth
        if depth == MAX_DEPTH:
            # Final check: Deterministic pseudo-random wall placement
            # cell_vector returns a Vector; we extract a component and threshold it
            noise_val = mathutils.noise.cell_vector(Vector((x, y, z)))[0]
            return noise_val > 0.3 # Adjust this threshold to change maze density

        # 1. Scale coordinates down to find the parent macro-cell
        scale = CELL_SIZE ** (MAX_DEPTH - depth)
        cell_x = x // scale
        cell_y = y // scale
        cell_z = z // scale

        # 2. Check if the entire macro-cell should be solid wall using a deterministic seed
        # Adding depth ensures each recursive layer looks completely different
        macro_vec = Vector((cell_x, cell_y, cell_z + depth * 100))
        macro_noise = mathutils.noise.cell_vector(macro_vec)[1] # Use Y component for variety
        
        # If the macro noise dictates a massive block/wall, stop early and fill it
        if macro_noise > 0.7: 
            return True

        # 3. Handle local sub-maze boundaries
        local_scale = scale // CELL_SIZE
        if local_scale > 0:
            local_x = (x // local_scale) % CELL_SIZE
            local_y = (y // local_scale) % CELL_SIZE
            local_z = (z // local_scale) % CELL_SIZE
            
            # If we are on the grid boundary of this sub-cell, decide if it's a closed doorway
            if local_x == 0 or local_y == 0 or local_z == 0:
                border_noise = mathutils.noise.cell_vector(Vector((cell_x + local_x, cell_y + local_y, cell_z + local_z)))[2]
                if border_noise > 0.4:
                    return True

        # 4. Dig deeper into the next recursive depth layer
        return self.check_cell_recursive(x, y, z, depth + 1)
"""
    
class Maze(Grid):
    def __init__(self, size):
        Grid.__init__(self, size)
        self.create(0)
        self.generate(self.maze_generate)
        
    def maze_generate(self, x, y, z):
        return 1 if self.is_voxel_solid(x, y, z) else 0
        
    def is_voxel_solid(self, x, y, z):
        # We enforce a strict 2-voxel wide rhythm: 
        # Corridors are at even coordinates, Walls are at odd coordinates.
        is_wall_x = (x % 2 == 1)
        is_wall_y = (y % 2 == 1)
        is_wall_z = (z % 2 == 1)
        
        # 1. The pillars where all wall axes intersect are ALWAYS solid to maintain structure
        if is_wall_x and is_wall_y and is_wall_z:
            return True
            
        # 2. If it's a corridor cell (even X, Y, Z), it's always open (Air)
        if not is_wall_x and not is_wall_y and not is_wall_z:
            return False
            
        # 3. If we are on a wall plane, we use the deterministic noise to "carve" a doorway.
        # By mapping the noise to the specific cell coordinates, we create straight axis-aligned cuts.
        cell_x = x // 2
        cell_y = y // 2
        cell_z = z // 2
        
        # Use cell_vector to get a stable 3D hash for this entire cell block
        cell_hash = mathutils.noise.cell_vector(Vector((cell_x, cell_y, cell_z)))
        
        # Selectively knock out walls along specific axes based on the hash
        if is_wall_x and not is_wall_y and not is_wall_z:
            # This is a wall separating cells along the X axis.
            # If the X-component of our hash is high, keep the wall. Otherwise, carve a doorway.
            return cell_hash[0] > 0.4
            
        if is_wall_y and not is_wall_x and not is_wall_z:
            # Wall separating cells along the Y axis.
            return cell_hash[1] > 0.1
            
        if is_wall_z and not is_wall_x and not is_wall_y:
            # Wall separating cells along the Z axis (Vertical floors/ceilings).
            # Make vertical connections rarer to keep it mostly horizontal but navigable
            return cell_hash[2] > 0.1
            
        # Default fallback for complex intersecting wall edges
        return True

maze = Maze(Vector([size, size, size]))
maze.export(mesh_obj, mesh_data)