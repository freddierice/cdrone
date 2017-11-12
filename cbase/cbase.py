'''cbase is a program for controlling the cdrone.'''

import socket
import ssl
import os

class CBaseError(Exception):
    '''Exception for CBase.'''

    def __init__(self, err):
        self.err = err

    def __str__(self):
        return self.err

class CBase:
    '''CBase holds most of the logic for sending commands.'''

    def __init__(self, host, port):
        '''Initialize the CBase class.'''
        self.host = host
        self.port = port
        self.conn = None
        pass

    def __enter__(self):
        self.connect()
        self.start_heartbeats()

    def __exit__(self, ctx_type, ctx_value, ctx_traceback):
        self.stop_heartbeats()
        self.close()

    def start_heartbeats(self):
        '''start the heartbeats.'''
        pass

    def stop_heartbeats(self):
        '''stop the heartbeats.'''
        pass
    
    def connect(self):
        '''connect will connect the base to the drone and initialize
        heartbeats.'''
        self.close()
        try:
            # If CBase cannot connect to the drone in 3 seconds, the heartbeats
            # will most likely miss.
            self.conn = socket.create_connection((self.host, self.port), 3)
        except socket.timeout:
            raise CBaseError("could not connect: host not available")

        key_directory = os.path.dirname(__import__(self.__module__)) + '/../keys'
        keyfile = key_directory + '/base-key.pem'
        certfile = key_directory + '/base.pem'
        ca_certs = key_directory + '/ca.pem'
        self.conn = ssl.wrap_socket(self.conn, keyfile=keyfile, 
                certfile=certfile, cert_reqs=ssl.CERT_REQUIRED, 
                ca_certs=ca_certs)

    def close(self):
        '''close a connection.'''
        if self.conn is not None:
            self.conn.close()
            self.conn = None

