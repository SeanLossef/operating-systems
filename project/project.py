# Sean Lossef - losses
# Joe Om - omj
# Solomon Chau - chaus2

import sys
import math
import heapq
from schedulers import *
from cpu import *


# Random Object
class Rand48:
	def __init__(self, seed, lamb, u_bound):
		self.lamb = lamb
		self.n = (seed << 16) + 0x330e
		self.u_bound = u_bound

	def seed(self, seed):
		self.n = seed

	def next(self):
		self.n = (25214903917 * self.n + 11) & (2**48 - 1)
		return self.n

	def drand(self):
		return self.next() / 2**48

	def exp_rand(self):
		r = self.u_bound + 1
		while r > self.u_bound:
			r = -math.log( self.drand() ) / self.lamb
		return r


# Returns list of psuedo-random processes
def generate_random_processes(seed, lamb, u_bound, n, alpha=None):
	r = Rand48(seed, lamb, u_bound)

	processes = []

	# Generate Pseudo-Random Processes
	for i in range(n):
		pid = chr(i + 65)
		arrival = math.floor(r.exp_rand())
		num_bursts = math.ceil(r.drand() * 100)

		p = Process(pid, arrival, alpha)
		last_burst = None

		for j in range(num_bursts):
			cpu_t = math.ceil(r.exp_rand())
			io_t  = math.ceil(r.exp_rand()) if j != num_bursts-1 else 0
			if alpha is not None:
				if j == 0:
					tau = 1 / lamb
				else:
					tau = (alpha * last_burst.cpu) + ((1 - alpha) * last_burst.tau)
				b = Burst(cpu_t, io_t, math.ceil(tau))
				last_burst = b
			else:
				b = Burst(cpu_t, io_t, 0)
			p.add_burst(b)

		processes.append(p)

	return processes


# Simulation Main
def main():
	if len(sys.argv) != 8 and len(sys.argv) != 9:
		print("Usage: python3 project1.py seed lambda u_bound n t_cs alpha t_slice [rr_add]")
		return

	# Get command line arguments
	seed    = int(sys.argv[1])
	lamb    = float(sys.argv[2])
	u_bound = int(sys.argv[3])
	n       = int(sys.argv[4])
	t_cs    = int(int(sys.argv[5]) / 2)
	alpha   = float(sys.argv[6])
	t_slice = int(sys.argv[7])
	rr_add  = sys.argv[8] if len(sys.argv) == 9 else "END"



	# Simulate FCFS
	processes = generate_random_processes(seed, lamb, u_bound, n)
	cpu = CPU(t_cs)
	fcfs = FCFS(cpu, processes)
	p = processes[0]
	while not fcfs.clock():
		continue

	print()

	# Simulate SJF
	processes = generate_random_processes(seed, lamb, u_bound, n, alpha)
	cpu = CPU(t_cs)
	sjf = SJF(cpu, processes)
	while not sjf.clock():
		continue

	print()

	# Simulate SRT
	processes = generate_random_processes(seed, lamb, u_bound, n, alpha)
	cpu = CPU(t_cs)
	srt = SRT(cpu, processes)
	while not srt.clock():
		continue

	print()

	# Simulate RR
	processes = generate_random_processes(seed, lamb, u_bound, n)
	cpu = CPU(t_cs)
	rr  = RR(cpu, processes, t_slice, rr_add)
	while not rr.clock():
		continue

if __name__== "__main__":
	main()