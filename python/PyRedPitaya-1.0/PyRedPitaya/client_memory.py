import numpy as np

class ClientMemory(object):
    def __init__(self, remote_connection):
        self.remote_connection = remote_connection
        self.remote_interface = remote_connection.root.mem()

    def read(self, addr):
        return self.remote_interface.read(addr)

    def reads(self, addr, length, return_buffer=False):
        out = self.remote_interface.reads(addr, length, return_buffer=True)
        if return_buffer:
            return out
        else:
            return np.frombuffer(out, dtype='uint32')

    def write(self, addr, value):
        self.remote_interface.write(addr, value)

    def writes(self, addr, values):
        if not isinstance(values, str):
            values = np.array(values, dtype='uint32')
            values = str(values.data)
        self.remote_interface.writes(addr, values)

