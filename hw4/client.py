import socket
import sys

server_addr = ('localhost', 5000)

def test7():
	tcp1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	tcp2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	tcp1.connect(server_addr)
	tcp2.connect(server_addr)

	try:
		tcp1.sendall('LOGIN Rick\n')
		print(tcp1.recv(1000))

		tcp1.sendall('SEND Morty 21\nAre you there, Morty?\n')
		print(tcp1.recv(1000))

		tcp2.sendall('LOGIN Morty\n')
		print(tcp2.recv(1000))

		tcp2.sendall('WHO\n')
		print(tcp2.recv(1000))

		tcp1.sendall('SEND Morty 24\nAha, there u are, Morty!\n')
		print(tcp1.recv(1000))

		tcp2.sendall('SEND Rick 27\nYes, idiot, I\'m right here!\n')
		print(tcp2.recv(1000))

		tcp2.sendall('BROADCAST 21\nAaaaaaaaaaaaaaaaaagh!\n')
		print(tcp2.recv(1000))

		tcp1.sendall('LOGOUT\n')
		print(tcp1.recv(1000))

		tcp2.sendall('WHO\n')
		print(tcp2.recv(1000))

		tcp2.sendall('LOGOUT\n')
		print(tcp2.recv(1000))

	finally:
		tcp1.close()
		tcp2.close()

def test8():
	udp1 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	udp1.settimeout(1)
	udp1.sendto('BROADCAST 21\nAre you there, Morty?\n', server_addr)

	try:
		data, server = udp1.recvfrom(1024)
		print data
	except timeout:
		print 'TIMEOUT'



test7()
test8()