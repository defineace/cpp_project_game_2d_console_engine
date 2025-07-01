import socket
import time

s = socket.socket()
s.connect(('127.0.0.1',8080))

x = 110.0

while x < 130.0:
    data = "player_1," + str(x) +",10.0"
    print(data)
    
    s.send(data.encode())
    
    x = x + 1
    time.sleep(1)

data = "quit," + str(x) +",10.0"
print(data)
s.send(data.encode())
s.close()