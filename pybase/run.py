#!/usr/bin/env python3
'''cbase is a module for controlling the cdrone.'''

import sys
import time
from cbase import CBase, CBaseError

def usage():
    '''print usage.'''
    print('usage: python -m cbase <host> [<port>]')

def main():
    '''main starts up the client.'''
    if len(sys.argv) != 2 and len(sys.argv) != 3:
        usage()
        sys.exit(1)
    host = sys.argv[1]
    port = 6006
    if len(sys.argv) == 3:
        try:
            port = int(sys.argv[2])
        except ValueError:
            print('invalid port number')
            sys.exit(1)
    
    try:
        with CBase(host,port) as cbase:
            time.sleep(3)
    except CBaseError as err:
        print(err)

if __name__ == '__main__':
    main()
