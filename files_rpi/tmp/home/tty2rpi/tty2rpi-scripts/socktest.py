# first of all import the socket library
import socket

# next create a socket object
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print ("Socket successfully created")

# reserve a port on your computer in our
# case it is 12345 but it can be anything
port = 6666

# Next bind to the port
# we have not typed any ip in the ip field
# instead we have inputted an empty string
# this makes the server listen to requests
# coming from other computers on the network
s.bind(('', port))
print ("socket bound to %s" %(port))

# put the socket into listening mode
s.listen(5)
print ("socket is listening")

# a forever loop until we interrupt it or
# an error occurs
while True:

        # Establish connection with client.
        print ("waiting")
        c, addr = s.accept()
        print ('Got connection from', addr )

        data = c.recv(4096).decode()
        if data:
            print(data)
