import ctypes

lib = ctypes.cdll.LoadLibrary(
    r'C:\Users\Dan\source\repos\psd_writer\x64\Release\psd_writer.dll')

class PSDRect:
    def __init__(self, x, y, w, h):
        self.rect = ctypesPSDRect(x=x, y=y, w=w, h=h)

class ctypesPSDRect(ctypes.Structure):
    _fields_ = [("x", ctypes.c_int),
                ("y", ctypes.c_int),
                ("w", ctypes.c_int),
                ("h", ctypes.c_int)]
    
def from_ndarray(param):
    return param.ctypes.data_as(ctypes.POINTER(ctypes.c_char))

class PSDocument(object):
    def __init__(self, width, height):
        lib.psd_new.argtypes = [ctypes.c_int, ctypes.c_int]
        lib.psd_new.restype = ctypes.c_void_p

        lib.psd_delete.argtypes = [ctypes.c_void_p]
        lib.psd_delete.restype = None

        lib.set_resolution.argtypes = [ctypes.c_void_p, ctypes.c_double]
        lib.set_resolution.restype = ctypes.c_bool

        lib.set_profile.argtypes = [ctypes.c_void_p, ctypes.c_wchar_p]
        lib.set_profile.restype = ctypes.c_bool

        lib.add_layer.argtypes = [ctypes.c_void_p,
                                  ctypes.POINTER(ctypes.c_char),
                                  ctypes.c_bool,
                                  ctypesPSDRect,
                                  ctypes.c_wchar_p]
        lib.add_layer.restype = ctypes.c_bool

        lib.save.argtypes = [ctypes.c_void_p, ctypes.c_wchar_p, ctypes.c_bool]
        lib.save.restype = ctypes.c_bool

        self.obj = lib.psd_new(width, height)

    def __del__(self):
        lib.psd_delete(self.obj)

    def set_resolution(self, res):
        return lib.set_resolution(self.obj, res)
    
    def set_profile(self, profile_path):
        return lib.set_profile(self.obj, profile_path)
    
    def add_layer(self, img, rgba, rect, layer_name):
        return lib.add_layer(self.obj, from_ndarray(img), rgba, rect.rect,
                             layer_name)
    
    def save(self, save_path, overwrite):
        return lib.save(self.obj, save_path, overwrite)

