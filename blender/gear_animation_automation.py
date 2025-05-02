import bpy
import math

scene = bpy.context.scene
g1 = bpy.data.objects['12t']
g2 = bpy.data.objects['24t']
g1t = 12
g2t = 24
handler = bpy.app.handlers.frame_change_pre
frame_start = scene.frame_start
frame_end = scene.frame_end

def get_frame():
    return (scene.frame_current - frame_start) / (frame_end - frame_start)

def frame_change_pre_handler(dummy):
    g1r = g1.rotation_euler[2] = 120 * (10 * get_frame() * get_frame())
    g2r = g2.rotation_euler[2] = g1r / (g2t / -g1t) + (math.pi / g1t / 1.65)

def register(handle, function):
    for x in range(len(handle)):
        if handle[x].__name__ == function.__name__:
            handle.pop(x)
            x -= 1
    handle.append(function)

frame_change_pre_handler(0)

register(handler, frame_change_pre_handler)
