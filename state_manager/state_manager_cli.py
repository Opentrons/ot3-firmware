# Util to access the state manager.
# To send data to the state manager, enter 4 whitespace-separated numbers and hit enter.

import sys
import socket 
import argparse 
import select

def parse_args():
    parser = argparse.ArgumentParser(description="Talk to the state manager")
    parser.add_argument('-i', '--ip', type=str, required=False, 
                        default='0.0.0.0', help='IP Address')
    parser.add_argument('-p', '--port', type=int, required=False, 
                        default=9999, help='Port to connect to')
    return parser.parse_args()


def main():
    args = parse_args()

    print(f'Connecting to {args.ip}:{args.port}')

    UDP_ADDR = (args.ip, args.port)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    while True:
        rlist = [sys.stdin, sock]
        readable, _, _ = select.select(rlist, [], [])
        if sys.stdin in readable:
            next = sys.stdin.readline()
            vals = next.split()
            if(len(vals) != 4):
                print('Please enter exactly 4 numbers') 
                continue
            message_ints = []
            for val in vals:
                int_val = int(val)
                if int_val > 0xFF or int_val < 0:
                    print(f'Bad value {int_val}', sys.stderr)
                    return -1
                message_ints.append(int_val)
            message = bytearray(message_ints)
            print(f'Sending: {message}')
            sock.sendto(message, UDP_ADDR)
        if sock in readable:
            data, _ = sock.recvfrom(100)
            print(f'Received {len(data)} byte message: {data}')
    
    return 0

if __name__ == '__main__':
    exit(main())